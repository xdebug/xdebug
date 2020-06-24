/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2020 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <derick@xdebug.org>                          |
   +----------------------------------------------------------------------+
 */

#include "php_xdebug.h"

#include "headers.h"

#include "lib_private.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

const char *xdebug_log_prefix[11] = {
	"", "E: ", "", "W: ", "", "", "", "I: ", "", "", "D: "
};



void xdebug_init_library_globals(xdebug_library_globals_t *xg)
{
	xg->headers              = NULL;

	xg->active_execute_data  = NULL;
	xg->opcode_handlers_set = xdebug_set_create(256);
	memset(xg->original_opcode_handlers, 0, sizeof(xg->original_opcode_handlers));
	memset(xg->opcode_multi_handlers, 0, sizeof(xg->opcode_multi_handlers));
}


void xdebug_library_zend_startup(void)
{
	xdebug_lib_zend_startup_overload_sapi_headers();
}

void xdebug_library_zend_shutdown(void)
{
	xdebug_lib_zend_shutdown_restore_sapi_headers();
}


void xdebug_library_minit(void)
{
	xdebug_set_opcode_multi_handler(ZEND_ASSIGN);
	xdebug_set_opcode_multi_handler(ZEND_QM_ASSIGN);
	xdebug_set_opcode_multi_handler(ZEND_INCLUDE_OR_EVAL);
}

static void xdebug_multi_opcode_handler_dtor(xdebug_multi_opcode_handler_t *ptr)
{
	if (ptr->next) {
		xdebug_multi_opcode_handler_dtor(ptr->next);
	}
	xdfree(ptr);
}

void xdebug_library_mshutdown(void)
{
	int i;

	/* Restore all opcode handlers that we have set */
	for (i = 0; i < 256; i++) {
		if (XG_LIB(opcode_multi_handlers)[i] != NULL) {
			xdebug_multi_opcode_handler_dtor(XG_LIB(opcode_multi_handlers)[i]);
		}
		xdebug_unset_opcode_handler(i);
	}

	xdebug_set_free(XG_LIB(opcode_handlers_set));
}

void xdebug_library_rinit(void)
{
	XG_LIB(headers) = xdebug_llist_alloc(xdebug_llist_string_dtor);
}

void xdebug_library_post_deactivate(void)
{
	/* Clean up collected headers */
	xdebug_llist_destroy(XG_LIB(headers), NULL);
	XG_LIB(headers) = NULL;
}


void xdebug_disable_opcache_optimizer(void)
{
	zend_string *key = zend_string_init(ZEND_STRL("opcache.optimization_level"), 1);
	zend_string *value = zend_string_init(ZEND_STRL("0"), 1);

	zend_alter_ini_entry(key, value, ZEND_INI_SYSTEM, ZEND_INI_STAGE_STARTUP);

	zend_string_release(key);
	zend_string_release(value);
}


static int xdebug_lib_set_mode_item(char *mode, int len)
{
	if (strncmp(mode, "off", len) == 0) {
		XG_LIB(mode) |= XDEBUG_MODE_OFF;
		return 1;
	}
	if (strncmp(mode, "develop", len) == 0) {
		XG_LIB(mode) |= XDEBUG_MODE_DEVELOP;
		return 1;
	}
	if (strncmp(mode, "coverage", len) == 0) {
		XG_LIB(mode) |= XDEBUG_MODE_COVERAGE;
		return 1;
	}
	if (strncmp(mode, "debug", len) == 0) {
		XG_LIB(mode) |= XDEBUG_MODE_STEP_DEBUG;
		return 1;
	}
	if (strncmp(mode, "gcstats", len) == 0) {
		XG_LIB(mode) |= XDEBUG_MODE_GCSTATS;
		return 1;
	}
	if (strncmp(mode, "profile", len) == 0) {
		XG_LIB(mode) |= XDEBUG_MODE_PROFILING;
		return 1;
	}
	if (strncmp(mode, "trace", len) == 0) {
		XG_LIB(mode) |= XDEBUG_MODE_TRACING;
		return 1;
	}

	php_error(E_WARNING, "Invalid mode '%s' set for 'xdebug.mode' configuration setting (See: https://xdebug.org/docs/all_settings#mode)", mode);

	return 0;
}

int xdebug_lib_set_mode(char *mode)
{
	char *mode_ptr = mode;
	char *comma    = NULL;
	int   errors   = 0;

	XG_LIB(mode) = 0;

	comma = strchr(mode_ptr, ',');
	while (comma) {
		errors += !xdebug_lib_set_mode_item(mode_ptr, comma - mode_ptr);
		mode_ptr = comma + 1;
		while (*mode_ptr == ' ') {
			mode_ptr++;
		}
		comma = strchr(mode_ptr, ',');
	}
	errors += !xdebug_lib_set_mode_item(mode_ptr, strlen(mode_ptr));

	return !errors;
}

int xdebug_lib_mode_is(int mode)
{
	if (XG_LIB(mode) & mode) {
		return 1;
	}

	return 0;
}

int xdebug_lib_set_start_with_request(char *value)
{
	if (strcmp(value, "default") == 0) {
		XG_LIB(start_with_request) = XDEBUG_START_WITH_REQUEST_DEFAULT;
		return 1;
	}
	if (strcmp(value, "yes") == 0 || strcmp(value, "1") == 0) {
		XG_LIB(start_with_request) = XDEBUG_START_WITH_REQUEST_YES;
		return 1;
	}
	if (strcmp(value, "no") == 0 || value[0] == '\0') {
		XG_LIB(start_with_request) = XDEBUG_START_WITH_REQUEST_NO;
		return 1;
	}
	if (strcmp(value, "trigger") == 0) {
		XG_LIB(start_with_request) = XDEBUG_START_WITH_REQUEST_TRIGGER;
		return 1;
	}

	return 0;
}

int xdebug_lib_start_with_request(void)
{
	if (XG_LIB(start_with_request) == XDEBUG_START_WITH_REQUEST_YES) {
		return 1;
	}

	if (XG_LIB(start_with_request) == XDEBUG_START_WITH_REQUEST_DEFAULT) {
		if (xdebug_lib_mode_is(XDEBUG_MODE_PROFILING)) {
			return 1;
		}
	}

	return 0;
}

int xdebug_lib_never_start_with_request(void)
{
	if (XG_LIB(start_with_request) == XDEBUG_START_WITH_REQUEST_NO) {
		return 1;
	}

	return 0;
}


int xdebug_lib_set_start_upon_error(char *value)
{
	if (strcmp(value, "default") == 0) {
		XG_LIB(start_upon_error) = XDEBUG_START_UPON_ERROR_DEFAULT;
		return 1;
	}
	if (strcmp(value, "yes") == 0 || strcmp(value, "1") == 0) {
		XG_LIB(start_upon_error) = XDEBUG_START_UPON_ERROR_YES;
		return 1;
	}
	if (strcmp(value, "no") == 0 || value[0] == '\0') {
		XG_LIB(start_upon_error) = XDEBUG_START_UPON_ERROR_NO;
		return 1;
	}

	return 0;
}

int xdebug_lib_start_upon_error(void)
{
	if (XG_LIB(start_upon_error) == XDEBUG_START_UPON_ERROR_YES) {
		return 1;
	}

	return 0;
}


static zval *find_in_globals(const char *element)
{
	zval *trigger_val = NULL;

	if (
		(
			(trigger_val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_ENV]), element, strlen(element))) != NULL
		) || (
			(trigger_val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_GET]), element, strlen(element))) != NULL
		) || (
			(trigger_val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_POST]), element, strlen(element))) != NULL
		) || (
			(trigger_val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_COOKIE]), element, strlen(element))) != NULL
		)
	) {
		return trigger_val;
	}

	return NULL;
}

static int trigger_enabled(void)
{
	char *trigger_value = XINI_LIB(trigger_value);
	zval *found_trigger_value = NULL;

	/* First we check for the generic 'XDEBUG_TRIGGER' option */
	found_trigger_value = find_in_globals("XDEBUG_TRIGGER");

	/* If not found, we fall back to the per-mode name for backwards compatibility reasons */
	if (!found_trigger_value) {
		const char *fallback_name = NULL;

		if (XG_LIB(mode) & XDEBUG_MODE_PROFILING) {
			fallback_name = "XDEBUG_PROFILE";
		} else if (XG_LIB(mode) & XDEBUG_MODE_TRACING) {
			fallback_name = "XDEBUG_TRACE";
		} else if (XG_LIB(mode) & XDEBUG_MODE_STEP_DEBUG) {
			fallback_name = "XDEBUG_SESSION";
		}

		if (fallback_name) {
			found_trigger_value = find_in_globals(fallback_name);
		}
	}

	if (!found_trigger_value) {
		return 0;
	}

	/* If the configured trigger value is empty, then it always triggers */
	if (trigger_value == NULL || trigger_value[0] == '\0') {
		return 1;
	}

	/* Check if the configured trigger value matches the one found in the
	 * trigger element */
	if (strcmp(trigger_value, Z_STRVAL_P(found_trigger_value)) == 0) {
		return 1;
	}

	return 0;
}

static int is_mode_trigger_and_enabled(int force_trigger)
{
	if (XG_LIB(start_with_request) == XDEBUG_START_WITH_REQUEST_TRIGGER) {
		return force_trigger || trigger_enabled();
	}

	if (XG_LIB(start_with_request) == XDEBUG_START_WITH_REQUEST_DEFAULT) {
		if (
			xdebug_lib_mode_is(XDEBUG_MODE_STEP_DEBUG) ||
			xdebug_lib_mode_is(XDEBUG_MODE_TRACING)
		) {
			return force_trigger || trigger_enabled();
		}
	}

	return 0;
}

/* Returns 1 if the mode is 'trigger', or 'default', where the default mode for
 * a feature is to trigger, and the trigger is present. */
int xdebug_lib_start_with_trigger(void)
{
	return is_mode_trigger_and_enabled(0);
}

/* Returns 1 if the mode is 'trigger', or 'default', where the default mode for
 * a feature is to trigger. Does not check whether a trigger is present. */
int xdebug_lib_start_if_mode_is_trigger(void)
{
	return is_mode_trigger_and_enabled(1);
}


function_stack_entry *xdebug_get_stack_head(void)
{
	xdebug_llist_element *le;

	if (XG_BASE(stack)) {
		if ((le = XDEBUG_LLIST_HEAD(XG_BASE(stack)))) {
			return XDEBUG_LLIST_VALP(le);
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

function_stack_entry *xdebug_get_stack_frame(int nr)
{
	xdebug_llist_element *le;

	if (!XG_BASE(stack)) {
		return NULL;
	}

	if (!(le = XDEBUG_LLIST_TAIL(XG_BASE(stack)))) {
		return NULL;
	}

	if (nr < 0) {
		return NULL;
	}

	while (nr) {
		nr--;
		le = XDEBUG_LLIST_PREV(le);
		if (!le) {
			return NULL;
		}
	}
	return XDEBUG_LLIST_VALP(le);
}

function_stack_entry *xdebug_get_stack_tail(void)
{
	xdebug_llist_element *le;

	if (XG_BASE(stack)) {
		if ((le = XDEBUG_LLIST_TAIL(XG_BASE(stack)))) {
			return XDEBUG_LLIST_VALP(le);
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

static void xdebug_used_var_hash_from_llist_dtor(void *data)
{
	xdebug_str *var_name = (xdebug_str*) data;

	xdebug_str_free(var_name);
}

static int xdebug_compare_le_xdebug_str(const void *le1, const void *le2)
{
	return strcmp(
		((xdebug_str *) XDEBUG_LLIST_VALP(*(xdebug_llist_element **) le1))->d,
		((xdebug_str *) XDEBUG_LLIST_VALP(*(xdebug_llist_element **) le2))->d
	);
}

xdebug_hash* xdebug_declared_var_hash_from_llist(xdebug_llist *list)
{
	xdebug_hash          *tmp;
	xdebug_llist_element *le;
	xdebug_str           *var_name;

	tmp = xdebug_hash_alloc_with_sort(32, xdebug_used_var_hash_from_llist_dtor, xdebug_compare_le_xdebug_str);
	for (le = XDEBUG_LLIST_HEAD(list); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
		var_name = (xdebug_str*) XDEBUG_LLIST_VALP(le);
		xdebug_hash_add(tmp, var_name->d, var_name->l, xdebug_str_copy(var_name));
	}

	return tmp;
}

void xdebug_lib_set_active_data(zend_execute_data *execute_data)
{
	XG_LIB(active_execute_data) = execute_data;
}

void xdebug_lib_set_active_object(zval *object)
{
	XG_LIB(active_object) = object;
}

void xdebug_lib_set_active_stack_entry(function_stack_entry *fse)
{
	XG_LIB(active_stack_entry) = fse;
}

void xdebug_lib_set_active_symbol_table(HashTable *symbol_table)
{
	XG_LIB(active_symbol_table) = symbol_table;
}

int xdebug_lib_has_active_data(void)
{
	return !!XG_LIB(active_execute_data);
}

int xdebug_lib_has_active_function(void)
{
	return !!XG_LIB(active_execute_data)->func;
}

int xdebug_lib_has_active_object(void)
{
	return !!XG_LIB(active_object);
}

int xdebug_lib_has_active_symbol_table(void)
{
	return !!XG_LIB(active_symbol_table);
}

zend_execute_data *xdebug_lib_get_active_data(void)
{
	return XG_LIB(active_execute_data);
}

zend_op_array *xdebug_lib_get_active_func_oparray(void)
{
	return &XG_LIB(active_execute_data)->func->op_array;
}

function_stack_entry *xdebug_lib_get_active_stack_entry(void)
{
	return XG_LIB(active_stack_entry);
}

HashTable *xdebug_lib_get_active_symbol_table(void)
{
	return XG_LIB(active_symbol_table);
}

zval *xdebug_lib_get_active_object(void)
{
	return XG_LIB(active_object);
}

int xdebug_isset_opcode_handler(int opcode)
{
	return xdebug_set_in(XG_LIB(opcode_handlers_set), opcode);
}

void xdebug_set_opcode_handler(int opcode, user_opcode_handler_t handler)
{
	if (xdebug_isset_opcode_handler(opcode)) {
		abort();
	}
	XG_LIB(original_opcode_handlers[opcode]) = zend_get_user_opcode_handler(opcode);
	xdebug_set_add(XG_LIB(opcode_handlers_set), opcode);
	zend_set_user_opcode_handler(opcode, handler);
}

static int xdebug_opcode_multi_handler(zend_execute_data *execute_data)
{
	const zend_op *cur_opcode = execute_data->opline;

	xdebug_multi_opcode_handler_t *handler_ptr = XG_LIB(opcode_multi_handlers[cur_opcode->opcode]);

	while (handler_ptr) {
		handler_ptr->handler(execute_data);
		handler_ptr = handler_ptr->next;
	}

	return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}

void xdebug_set_opcode_multi_handler(int opcode)
{
	if (xdebug_isset_opcode_handler(opcode)) {
		abort();
	}
	XG_LIB(original_opcode_handlers[opcode]) = zend_get_user_opcode_handler(opcode);
	xdebug_set_add(XG_LIB(opcode_handlers_set), opcode);
	zend_set_user_opcode_handler(opcode, xdebug_opcode_multi_handler);
}

void xdebug_register_with_opcode_multi_handler(int opcode, user_opcode_handler_t handler)
{
	xdebug_multi_opcode_handler_t *ptr;
	xdebug_multi_opcode_handler_t *tmp = xdmalloc(sizeof(xdebug_multi_opcode_handler_t));
	tmp->handler = handler;
	tmp->next    = NULL;

	if (!xdebug_isset_opcode_handler(opcode)) {
		abort();
	}

	if (XG_LIB(opcode_multi_handlers)[opcode] == NULL) {
		XG_LIB(opcode_multi_handlers)[opcode] = tmp;
		return;
	}

	ptr = XG_LIB(opcode_multi_handlers)[opcode];
	while (ptr->next) {
		ptr = ptr->next;
	}

	ptr->next = tmp;
}

void xdebug_unset_opcode_handler(int opcode)
{
	if (xdebug_set_in(XG_LIB(opcode_handlers_set), opcode)) {
		zend_set_user_opcode_handler(opcode, XG_LIB(original_opcode_handlers[opcode]));
	}
}

int xdebug_call_original_opcode_handler_if_set(int opcode, XDEBUG_OPCODE_HANDLER_ARGS)
{
	if (xdebug_isset_opcode_handler(opcode)) {
		user_opcode_handler_t handler = XG_LIB(original_opcode_handlers[opcode]);

		if (handler) {
			return handler(XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
		}
	}

	return ZEND_USER_OPCODE_DISPATCH;
}

/* Does not duplicate the return value, don't free. Return NULL if it's
 * not-set, or an empty string */
char *xdebug_lib_get_output_dir(void)
{
	char *output_dir = XINI_LIB(output_dir);

	if (output_dir == NULL || output_dir[0] == '\0') {
		return NULL;
	}

	return output_dir;
}

void xdebug_llist_string_dtor(void *dummy, void *elem)
{
	char *s = elem;

	if (s) {
		xdfree(s);
	}
}
