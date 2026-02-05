/*
 * Unit tests for FrankenPHP worker mode support
 *
 * Tests:
 * - Trigger detection in cookies and query strings
 * - SAPI hook installation/restoration
 * - Debugger state reset on request start
 * - Connection cleanup on request end
 *
 * Compile: cc -o frankenphp_test frankenphp_test.c
 * Run: ./frankenphp_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Test framework
 */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
	printf("  %-50s ", #name); \
	tests_run++; \
	test_##name(); \
	tests_passed++; \
	printf("OK\n"); \
} while(0)

#define ASSERT(cond) do { \
	if (!(cond)) { \
		printf("FAILED\n    Assertion failed: %s\n    at %s:%d\n", #cond, __FILE__, __LINE__); \
		exit(1); \
	} \
} while(0)

#define ASSERT_EQ(a, b) do { \
	if ((a) != (b)) { \
		printf("FAILED\n    Expected %d, got %d\n    at %s:%d\n", (int)(b), (int)(a), __FILE__, __LINE__); \
		exit(1); \
	} \
} while(0)

/*
 * Mock structures mimicking PHP/Xdebug internals
 */

/* Mock request_info structure */
typedef struct {
	char *cookie_data;
	char *query_string;
} mock_request_info_t;

/* Mock SAPI globals */
static struct {
	mock_request_info_t request_info;
} mock_sapi_globals;

#define SG(member) mock_sapi_globals.member

/* Mock xdebug debugger context */
typedef struct {
	int do_break;
	int do_step;
	int do_next;
	int do_finish;
	int do_connect_to_client;
} mock_xdebug_context_t;

/* Mock xdebug debugger globals */
static struct {
	int detached;
	int no_exec;
	int breakpoints_allowed;
	mock_xdebug_context_t context;
} mock_xdebug_globals;

/* Mock debug connection state */
static int mock_debug_connection_active = 0;
static int mock_remote_deinit_called = 0;

/* Mock SAPI module */
typedef struct {
	const char *name;
	int (*activate)(void);
	int (*deactivate)(void);
} mock_sapi_module_t;

static mock_sapi_module_t mock_sapi_module;

/* Mock xdebug mode */
static int mock_xdebug_mode_step_debug = 1;

/*
 * Mock function implementations
 */
#define SUCCESS 0
#define XDEBUG_MODE_IS(mode) (mock_xdebug_mode_step_debug)
#define XG_DBG(member) mock_xdebug_globals.member

static int xdebug_is_debug_connection_active(void) {
	return mock_debug_connection_active;
}

static void xdebug_mark_debug_connection_not_active(void) {
	mock_debug_connection_active = 0;
}

static void mock_remote_deinit(void *ctx) {
	(void)ctx;
	mock_remote_deinit_called = 1;
}

/* Mock handler for context */
typedef struct {
	void (*remote_deinit)(void *);
} mock_handler_t;

static mock_handler_t mock_handler = { mock_remote_deinit };

/*
 * Copy of functions under test (from frankenphp.c)
 */

static int has_trigger_in_string(const char *str, char delim)
{
	const char *triggers[] = {"XDEBUG_SESSION", "XDEBUG_TRIGGER", NULL};

	if (!str) return 0;

	for (const char **t = triggers; *t; t++) {
		size_t len = strlen(*t);
		const char *p = str;
		while ((p = strstr(p, *t))) {
			if ((p == str || p[-1] == delim || p[-1] == ' ') && p[len] == '=') {
				return 1;
			}
			p += len;
		}
	}
	return 0;
}

static int has_debug_trigger(void)
{
	return has_trigger_in_string(SG(request_info).cookie_data, ';') ||
	       has_trigger_in_string(SG(request_info).query_string, '&');
}

/* SAPI hooks state */
static int (*original_sapi_activate)(void) = NULL;
static int (*original_sapi_deactivate)(void) = NULL;
static int is_frankenphp = 0;

static int frankenphp_sapi_activate(void)
{
	int result = original_sapi_activate ? original_sapi_activate() : SUCCESS;

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		XG_DBG(detached) = 0;
		XG_DBG(no_exec) = 0;
		XG_DBG(breakpoints_allowed) = 1;
		XG_DBG(context).do_break = 0;
		XG_DBG(context).do_step = 0;
		XG_DBG(context).do_next = 0;
		XG_DBG(context).do_finish = 0;
		XG_DBG(context).do_connect_to_client = 0;

		if (has_debug_trigger()) {
			XG_DBG(context).do_connect_to_client = 1;
		}
	}
	return result;
}

static int frankenphp_sapi_deactivate(void)
{
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG) && xdebug_is_debug_connection_active()) {
		mock_remote_deinit(NULL);
		xdebug_mark_debug_connection_not_active();
	}
	return original_sapi_deactivate ? original_sapi_deactivate() : SUCCESS;
}

static void xdebug_frankenphp_minit(void)
{
	if (strcmp(mock_sapi_module.name, "frankenphp") == 0) {
		is_frankenphp = 1;
		original_sapi_activate = mock_sapi_module.activate;
		mock_sapi_module.activate = frankenphp_sapi_activate;
		original_sapi_deactivate = mock_sapi_module.deactivate;
		mock_sapi_module.deactivate = frankenphp_sapi_deactivate;
	}
}

static void xdebug_frankenphp_mshutdown(void)
{
	if (is_frankenphp) {
		mock_sapi_module.activate = original_sapi_activate;
		mock_sapi_module.deactivate = original_sapi_deactivate;
		original_sapi_activate = NULL;
		original_sapi_deactivate = NULL;
		is_frankenphp = 0;
	}
}

/*
 * Test helpers
 */
static void reset_mocks(void)
{
	memset(&mock_sapi_globals, 0, sizeof(mock_sapi_globals));
	memset(&mock_xdebug_globals, 0, sizeof(mock_xdebug_globals));
	mock_debug_connection_active = 0;
	mock_remote_deinit_called = 0;
	mock_xdebug_mode_step_debug = 1;
	mock_sapi_module.name = "cli";
	mock_sapi_module.activate = NULL;
	mock_sapi_module.deactivate = NULL;
	original_sapi_activate = NULL;
	original_sapi_deactivate = NULL;
	is_frankenphp = 0;
}

/*
 * ==========================================================================
 * Trigger detection tests
 * ==========================================================================
 */

TEST(trigger_cookie_xdebug_session)
{
	ASSERT(has_trigger_in_string("XDEBUG_SESSION=abc", ';') == 1);
}

TEST(trigger_cookie_xdebug_session_middle)
{
	ASSERT(has_trigger_in_string("foo=bar; XDEBUG_SESSION=abc", ';') == 1);
}

TEST(trigger_cookie_xdebug_trigger)
{
	ASSERT(has_trigger_in_string("XDEBUG_TRIGGER=1", ';') == 1);
}

TEST(trigger_cookie_no_match)
{
	ASSERT(has_trigger_in_string("foo=bar; session=abc", ';') == 0);
}

TEST(trigger_cookie_in_value_no_match)
{
	ASSERT(has_trigger_in_string("foo=XDEBUG_SESSION", ';') == 0);
}

TEST(trigger_cookie_prefix_no_match)
{
	ASSERT(has_trigger_in_string("MY_XDEBUG_SESSION=abc", ';') == 0);
}

TEST(trigger_cookie_null)
{
	ASSERT(has_trigger_in_string(NULL, ';') == 0);
}

TEST(trigger_query_xdebug_session)
{
	ASSERT(has_trigger_in_string("XDEBUG_SESSION=abc", '&') == 1);
}

TEST(trigger_query_xdebug_trigger)
{
	ASSERT(has_trigger_in_string("foo=bar&XDEBUG_TRIGGER=1", '&') == 1);
}

TEST(trigger_query_empty_value)
{
	ASSERT(has_trigger_in_string("XDEBUG_TRIGGER=", '&') == 1);
}

TEST(trigger_query_no_equals_no_match)
{
	ASSERT(has_trigger_in_string("XDEBUG_SESSION", '&') == 0);
}

TEST(trigger_has_debug_trigger_cookie)
{
	reset_mocks();
	SG(request_info).cookie_data = "XDEBUG_SESSION=ide";
	SG(request_info).query_string = NULL;
	ASSERT(has_debug_trigger() == 1);
}

TEST(trigger_has_debug_trigger_query)
{
	reset_mocks();
	SG(request_info).cookie_data = NULL;
	SG(request_info).query_string = "XDEBUG_TRIGGER=1";
	ASSERT(has_debug_trigger() == 1);
}

TEST(trigger_has_debug_trigger_both)
{
	reset_mocks();
	SG(request_info).cookie_data = "foo=bar";
	SG(request_info).query_string = "XDEBUG_SESSION=abc";
	ASSERT(has_debug_trigger() == 1);
}

TEST(trigger_has_debug_trigger_none)
{
	reset_mocks();
	SG(request_info).cookie_data = "foo=bar";
	SG(request_info).query_string = "baz=qux";
	ASSERT(has_debug_trigger() == 0);
}

/*
 * ==========================================================================
 * SAPI hook installation tests
 * ==========================================================================
 */

static int mock_original_activate_called = 0;
static int mock_original_activate(void) {
	mock_original_activate_called = 1;
	return SUCCESS;
}

static int mock_original_deactivate_called = 0;
static int mock_original_deactivate(void) {
	mock_original_deactivate_called = 1;
	return SUCCESS;
}

TEST(minit_detects_frankenphp)
{
	reset_mocks();
	mock_sapi_module.name = "frankenphp";
	xdebug_frankenphp_minit();
	ASSERT(is_frankenphp == 1);
}

TEST(minit_ignores_other_sapi)
{
	reset_mocks();
	mock_sapi_module.name = "cli";
	xdebug_frankenphp_minit();
	ASSERT(is_frankenphp == 0);
}

TEST(minit_installs_activate_hook)
{
	reset_mocks();
	mock_sapi_module.name = "frankenphp";
	mock_sapi_module.activate = mock_original_activate;
	xdebug_frankenphp_minit();
	ASSERT(mock_sapi_module.activate == frankenphp_sapi_activate);
	ASSERT(original_sapi_activate == mock_original_activate);
}

TEST(minit_installs_deactivate_hook)
{
	reset_mocks();
	mock_sapi_module.name = "frankenphp";
	mock_sapi_module.deactivate = mock_original_deactivate;
	xdebug_frankenphp_minit();
	ASSERT(mock_sapi_module.deactivate == frankenphp_sapi_deactivate);
	ASSERT(original_sapi_deactivate == mock_original_deactivate);
}

TEST(mshutdown_restores_hooks)
{
	reset_mocks();
	mock_sapi_module.name = "frankenphp";
	mock_sapi_module.activate = mock_original_activate;
	mock_sapi_module.deactivate = mock_original_deactivate;
	xdebug_frankenphp_minit();
	xdebug_frankenphp_mshutdown();
	ASSERT(mock_sapi_module.activate == mock_original_activate);
	ASSERT(mock_sapi_module.deactivate == mock_original_deactivate);
	ASSERT(is_frankenphp == 0);
}

/*
 * ==========================================================================
 * SAPI activate tests (request start)
 * ==========================================================================
 */

TEST(activate_resets_detached_flag)
{
	reset_mocks();
	mock_xdebug_globals.detached = 1;
	frankenphp_sapi_activate();
	ASSERT_EQ(mock_xdebug_globals.detached, 0);
}

TEST(activate_resets_no_exec_flag)
{
	reset_mocks();
	mock_xdebug_globals.no_exec = 1;
	frankenphp_sapi_activate();
	ASSERT_EQ(mock_xdebug_globals.no_exec, 0);
}

TEST(activate_enables_breakpoints)
{
	reset_mocks();
	mock_xdebug_globals.breakpoints_allowed = 0;
	frankenphp_sapi_activate();
	ASSERT_EQ(mock_xdebug_globals.breakpoints_allowed, 1);
}

TEST(activate_resets_stepping_flags)
{
	reset_mocks();
	mock_xdebug_globals.context.do_break = 1;
	mock_xdebug_globals.context.do_step = 1;
	mock_xdebug_globals.context.do_next = 1;
	mock_xdebug_globals.context.do_finish = 1;
	frankenphp_sapi_activate();
	ASSERT_EQ(mock_xdebug_globals.context.do_break, 0);
	ASSERT_EQ(mock_xdebug_globals.context.do_step, 0);
	ASSERT_EQ(mock_xdebug_globals.context.do_next, 0);
	ASSERT_EQ(mock_xdebug_globals.context.do_finish, 0);
}

TEST(activate_sets_connect_on_trigger)
{
	reset_mocks();
	SG(request_info).cookie_data = "XDEBUG_SESSION=ide";
	mock_xdebug_globals.context.do_connect_to_client = 0;
	frankenphp_sapi_activate();
	ASSERT_EQ(mock_xdebug_globals.context.do_connect_to_client, 1);
}

TEST(activate_no_connect_without_trigger)
{
	reset_mocks();
	SG(request_info).cookie_data = "foo=bar";
	mock_xdebug_globals.context.do_connect_to_client = 0;
	frankenphp_sapi_activate();
	ASSERT_EQ(mock_xdebug_globals.context.do_connect_to_client, 0);
}

TEST(activate_calls_original_hook)
{
	reset_mocks();
	mock_original_activate_called = 0;
	original_sapi_activate = mock_original_activate;
	frankenphp_sapi_activate();
	ASSERT(mock_original_activate_called == 1);
}

TEST(activate_skips_when_debug_mode_off)
{
	reset_mocks();
	mock_xdebug_mode_step_debug = 0;
	mock_xdebug_globals.detached = 1;
	SG(request_info).cookie_data = "XDEBUG_SESSION=ide";
	frankenphp_sapi_activate();
	/* Should not reset flags when debug mode is off */
	ASSERT_EQ(mock_xdebug_globals.detached, 1);
	ASSERT_EQ(mock_xdebug_globals.context.do_connect_to_client, 0);
}

/*
 * ==========================================================================
 * SAPI deactivate tests (request end)
 * ==========================================================================
 */

TEST(deactivate_closes_active_connection)
{
	reset_mocks();
	mock_debug_connection_active = 1;
	mock_remote_deinit_called = 0;
	frankenphp_sapi_deactivate();
	ASSERT(mock_remote_deinit_called == 1);
	ASSERT(mock_debug_connection_active == 0);
}

TEST(deactivate_skips_inactive_connection)
{
	reset_mocks();
	mock_debug_connection_active = 0;
	mock_remote_deinit_called = 0;
	frankenphp_sapi_deactivate();
	ASSERT(mock_remote_deinit_called == 0);
}

TEST(deactivate_calls_original_hook)
{
	reset_mocks();
	mock_original_deactivate_called = 0;
	original_sapi_deactivate = mock_original_deactivate;
	frankenphp_sapi_deactivate();
	ASSERT(mock_original_deactivate_called == 1);
}

TEST(deactivate_skips_when_debug_mode_off)
{
	reset_mocks();
	mock_xdebug_mode_step_debug = 0;
	mock_debug_connection_active = 1;
	mock_remote_deinit_called = 0;
	frankenphp_sapi_deactivate();
	/* Should not close connection when debug mode is off */
	ASSERT(mock_remote_deinit_called == 0);
	ASSERT(mock_debug_connection_active == 1);
}

/*
 * ==========================================================================
 * Integration tests (full request cycle)
 * ==========================================================================
 */

TEST(full_cycle_with_trigger)
{
	reset_mocks();
	mock_sapi_module.name = "frankenphp";
	xdebug_frankenphp_minit();

	/* Simulate request with trigger */
	SG(request_info).cookie_data = "XDEBUG_SESSION=phpstorm";
	mock_xdebug_globals.detached = 1;
	mock_sapi_module.activate();

	ASSERT_EQ(mock_xdebug_globals.detached, 0);
	ASSERT_EQ(mock_xdebug_globals.context.do_connect_to_client, 1);

	/* Simulate debug session */
	mock_debug_connection_active = 1;

	/* End request */
	mock_sapi_module.deactivate();
	ASSERT(mock_debug_connection_active == 0);
	ASSERT(mock_remote_deinit_called == 1);
}

TEST(full_cycle_without_trigger)
{
	reset_mocks();
	mock_sapi_module.name = "frankenphp";
	xdebug_frankenphp_minit();

	/* Simulate request without trigger */
	SG(request_info).cookie_data = NULL;
	SG(request_info).query_string = "page=1";
	mock_sapi_module.activate();

	ASSERT_EQ(mock_xdebug_globals.context.do_connect_to_client, 0);

	/* End request (no connection to close) */
	mock_debug_connection_active = 0;
	mock_sapi_module.deactivate();
	ASSERT(mock_remote_deinit_called == 0);
}

TEST(multiple_requests_isolation)
{
	reset_mocks();
	mock_sapi_module.name = "frankenphp";
	xdebug_frankenphp_minit();

	/* Request 1: with trigger */
	SG(request_info).cookie_data = "XDEBUG_TRIGGER=1";
	mock_sapi_module.activate();
	ASSERT_EQ(mock_xdebug_globals.context.do_connect_to_client, 1);
	mock_debug_connection_active = 1;
	mock_sapi_module.deactivate();
	ASSERT(mock_debug_connection_active == 0);

	/* Request 2: without trigger - should not inherit state */
	mock_xdebug_globals.context.do_connect_to_client = 99; /* dirty state */
	SG(request_info).cookie_data = NULL;
	SG(request_info).query_string = NULL;
	mock_sapi_module.activate();
	ASSERT_EQ(mock_xdebug_globals.context.do_connect_to_client, 0);
}

/*
 * ==========================================================================
 * Main
 * ==========================================================================
 */

int main(void)
{
	printf("FrankenPHP worker mode support tests\n");
	printf("====================================\n\n");

	printf("Trigger detection:\n");
	RUN_TEST(trigger_cookie_xdebug_session);
	RUN_TEST(trigger_cookie_xdebug_session_middle);
	RUN_TEST(trigger_cookie_xdebug_trigger);
	RUN_TEST(trigger_cookie_no_match);
	RUN_TEST(trigger_cookie_in_value_no_match);
	RUN_TEST(trigger_cookie_prefix_no_match);
	RUN_TEST(trigger_cookie_null);
	RUN_TEST(trigger_query_xdebug_session);
	RUN_TEST(trigger_query_xdebug_trigger);
	RUN_TEST(trigger_query_empty_value);
	RUN_TEST(trigger_query_no_equals_no_match);
	RUN_TEST(trigger_has_debug_trigger_cookie);
	RUN_TEST(trigger_has_debug_trigger_query);
	RUN_TEST(trigger_has_debug_trigger_both);
	RUN_TEST(trigger_has_debug_trigger_none);

	printf("\nSAPI hook installation:\n");
	RUN_TEST(minit_detects_frankenphp);
	RUN_TEST(minit_ignores_other_sapi);
	RUN_TEST(minit_installs_activate_hook);
	RUN_TEST(minit_installs_deactivate_hook);
	RUN_TEST(mshutdown_restores_hooks);

	printf("\nSAPI activate (request start):\n");
	RUN_TEST(activate_resets_detached_flag);
	RUN_TEST(activate_resets_no_exec_flag);
	RUN_TEST(activate_enables_breakpoints);
	RUN_TEST(activate_resets_stepping_flags);
	RUN_TEST(activate_sets_connect_on_trigger);
	RUN_TEST(activate_no_connect_without_trigger);
	RUN_TEST(activate_calls_original_hook);
	RUN_TEST(activate_skips_when_debug_mode_off);

	printf("\nSAPI deactivate (request end):\n");
	RUN_TEST(deactivate_closes_active_connection);
	RUN_TEST(deactivate_skips_inactive_connection);
	RUN_TEST(deactivate_calls_original_hook);
	RUN_TEST(deactivate_skips_when_debug_mode_off);

	printf("\nIntegration (full request cycle):\n");
	RUN_TEST(full_cycle_with_trigger);
	RUN_TEST(full_cycle_without_trigger);
	RUN_TEST(multiple_requests_isolation);

	printf("\n====================================\n");
	printf("All tests passed (%d/%d)\n", tests_passed, tests_run);

	return 0;
}
