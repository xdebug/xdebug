/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2023 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "lib/php-header.h"
#include "main/php_version.h"
#include "lib/compat.h"

#if HAVE_XDEBUG

#ifndef PHP_WIN32
#include <sys/time.h>
#include <unistd.h>
#else
#include "win32/time.h"
#include <process.h>
#endif

#include "TSRM.h"
#include "SAPI.h"
#include "zend_extensions.h"
#include "main/php_ini.h"
#include "ext/standard/head.h"
#include "ext/standard/html.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "php_globals.h"
#include "main/php_output.h"
#include "ext/standard/php_var.h"

#include "php_xdebug.h"
#include "php_xdebug_arginfo.h"

#include "base/base.h"
#include "base/filter.h"
#include "coverage/code_coverage.h"
#include "develop/monitor.h"
#include "develop/stack.h"
#include "develop/superglobals.h"
#include "debugger/com.h"
#include "gcstats/gc_stats.h"
#include "lib/usefulstuff.h"
#include "lib/lib.h"
#include "lib/llist.h"
#include "lib/log.h"
#include "lib/mm.h"
#include "lib/var_export_html.h"
#include "lib/var_export_line.h"
#include "lib/var_export_text.h"
#include "profiler/profiler.h"
#include "tracing/tracing.h"

static zend_result (*xdebug_orig_post_startup_cb)(void);
static zend_result xdebug_post_startup(void);

int xdebug_include_or_eval_handler(zend_execute_data *execute_data);

/* True globals */
int zend_xdebug_initialised = 0;
int xdebug_global_mode = 0;

zend_module_entry xdebug_module_entry = {
	STANDARD_MODULE_HEADER,
	"xdebug",
	ext_functions,
	PHP_MINIT(xdebug),
	PHP_MSHUTDOWN(xdebug),
	PHP_RINIT(xdebug),
	PHP_RSHUTDOWN(xdebug),
	PHP_MINFO(xdebug),
	XDEBUG_VERSION,
	NO_MODULE_GLOBALS,
	ZEND_MODULE_POST_ZEND_DEACTIVATE_N(xdebug),
	STANDARD_MODULE_PROPERTIES_EX
};

ZEND_DECLARE_MODULE_GLOBALS(xdebug)

#if COMPILE_DL_XDEBUG
ZEND_GET_MODULE(xdebug)
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
# endif
#endif

static PHP_INI_MH(OnUpdateServer)
{
	DUMP_TOK(server);
}

static PHP_INI_MH(OnUpdateGet)
{
	DUMP_TOK(get);
}

static PHP_INI_MH(OnUpdatePost)
{
	DUMP_TOK(post);
}

static PHP_INI_MH(OnUpdateCookie)
{
	DUMP_TOK(cookie);
}

static PHP_INI_MH(OnUpdateFiles)
{
	DUMP_TOK(files);
}

static PHP_INI_MH(OnUpdateEnv)
{
	DUMP_TOK(env);
}

static PHP_INI_MH(OnUpdateRequest)
{
	DUMP_TOK(request);
}

static PHP_INI_MH(OnUpdateSession)
{
	DUMP_TOK(session);
}

static PHP_INI_MH(OnUpdateStartWithRequest)
{
	if (!new_value) {
		return FAILURE;
	}

	if (!xdebug_lib_set_start_with_request(ZSTR_VAL(new_value))) {
		return FAILURE;
	}

	return SUCCESS;
}

static PHP_INI_MH(OnUpdateStartUponError)
{
	if (!new_value) {
		return FAILURE;
	}

	if (!xdebug_lib_set_start_upon_error(ZSTR_VAL(new_value))) {
		return FAILURE;
	}

	return SUCCESS;
}

static PHP_INI_MH(OnUpdateRemovedSetting)
{
	if (! (EG(error_reporting) & E_DEPRECATED)) {
		return SUCCESS;
	}
	if (new_value && ZSTR_LEN(new_value) > 0 && strncmp("This setting", ZSTR_VAL(new_value), 11) != 0) {
		xdebug_log_ex(
			XLOG_CHAN_CONFIG, XLOG_CRIT, "REMOVED",
			"The setting '%s' has been removed, see the upgrading guide at %supgrade_guide#changed-%s",
			ZSTR_VAL(entry->name), xdebug_lib_docs_base(), ZSTR_VAL(entry->name)
		);
	}
	return FAILURE;
}

static PHP_INI_MH(OnUpdateChangedSetting)
{
	if (! (EG(error_reporting) & E_DEPRECATED)) {
		return SUCCESS;
	}
	if (new_value && ZSTR_LEN(new_value) > 0 && strncmp("This setting", ZSTR_VAL(new_value), 11) != 0) {
		xdebug_log_ex(
			XLOG_CHAN_CONFIG, XLOG_CRIT, "CHANGED",
			"The setting '%s' has been renamed, see the upgrading guide at %supgrade_guide#changed-%s",
			ZSTR_VAL(entry->name), xdebug_lib_docs_base(), ZSTR_VAL(entry->name)
		);
	}
	return FAILURE;
}


#ifdef P_tmpdir
# define XDEBUG_TEMP_DIR P_tmpdir
#else
# ifdef PHP_WIN32
#  define XDEBUG_TEMP_DIR "C:\\Windows\\Temp"
# else
#  define XDEBUG_TEMP_DIR "/tmp"
# endif
#endif

ZEND_INI_DISP(display_removed_setting)
{
	ZEND_PUTS("(setting removed in Xdebug 3)");
}

ZEND_INI_DISP(display_changed_setting)
{
	ZEND_PUTS("(setting renamed in Xdebug 3)");
}

#define XDEBUG_REMOVED_INI_ENTRY(n) PHP_INI_ENTRY_EX(("" # n), "This setting has been removed, see the upgrading guide at https://xdebug.org/docs/upgrade_guide#removed-" # n, PHP_INI_ALL, OnUpdateRemovedSetting, display_removed_setting)
#define XDEBUG_CHANGED_INI_ENTRY(n) PHP_INI_ENTRY_EX(("" # n), "This setting has been changed, see the upgrading guide at https://xdebug.org/docs/upgrade_guide#changed-" # n, PHP_INI_ALL, OnUpdateChangedSetting, display_changed_setting)

static const char *xdebug_start_with_request_types[5] = { "", "default", "yes", "no", "trigger" };

ZEND_INI_DISP(display_start_with_request)
{
	char *value;

	if (type == ZEND_INI_DISPLAY_ORIG && ini_entry->modified) {
		value = ZSTR_VAL(ini_entry->orig_value);
	} else if (ini_entry->value) {
		value = ZSTR_VAL(ini_entry->value);
	} else {
		value = NULL;
	}
	if (value) {
		ZEND_PUTS(xdebug_start_with_request_types[xdebug_lib_get_start_with_request()]);
	} else {
		ZEND_PUTS("?");
	}
}


static const char *xdebug_start_upon_error_types[4] = { "", "default", "yes", "no" };

ZEND_INI_DISP(display_start_upon_error)
{
	char *value;

	if (type == ZEND_INI_DISPLAY_ORIG && ini_entry->modified) {
		value = ZSTR_VAL(ini_entry->orig_value);
	} else if (ini_entry->value) {
		value = ZSTR_VAL(ini_entry->value);
	} else {
		value = NULL;
	}
	if (value) {
		ZEND_PUTS(xdebug_start_upon_error_types[xdebug_lib_get_start_upon_error()]);
	} else {
		ZEND_PUTS("?");
	}
}


#if HAVE_XDEBUG_ZLIB
# define USE_COMPRESSION_DEFAULT "1"
#else
# define USE_COMPRESSION_DEFAULT "0"
#endif

PHP_INI_BEGIN()
	/* Library settings */
	STD_PHP_INI_ENTRY("xdebug.mode",               "develop",               PHP_INI_SYSTEM,                OnUpdateString, settings.library.requested_mode,   zend_xdebug_globals, xdebug_globals)
	PHP_INI_ENTRY_EX( "xdebug.start_with_request", "default",               PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateStartWithRequest, display_start_with_request)
	PHP_INI_ENTRY_EX( "xdebug.start_upon_error",   "default",               PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateStartUponError,   display_start_upon_error)
	STD_PHP_INI_ENTRY("xdebug.output_dir",         XDEBUG_TEMP_DIR,         PHP_INI_ALL,                   OnUpdateString, settings.library.output_dir,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.use_compression",    USE_COMPRESSION_DEFAULT, PHP_INI_ALL,                   OnUpdateBool,   settings.library.use_compression,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trigger_value",      "",                      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, settings.library.trigger_value,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.file_link_format",   "",                      PHP_INI_ALL,                   OnUpdateString, settings.library.file_link_format, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.filename_format",    "",                      PHP_INI_ALL,                   OnUpdateString, settings.library.filename_format,  zend_xdebug_globals, xdebug_globals)

	STD_PHP_INI_ENTRY("xdebug.log",       "",           PHP_INI_ALL, OnUpdateString, settings.library.log,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.log_level", XLOG_DEFAULT, PHP_INI_ALL, OnUpdateLong,   settings.library.log_level, zend_xdebug_globals, xdebug_globals)

	/* Variable display settings */
	STD_PHP_INI_ENTRY("xdebug.var_display_max_children", "128",     PHP_INI_ALL,    OnUpdateLong,   settings.library.display_max_children, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.var_display_max_data",     "512",     PHP_INI_ALL,    OnUpdateLong,   settings.library.display_max_data,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.var_display_max_depth",    "3",       PHP_INI_ALL,    OnUpdateLong,   settings.library.display_max_depth,    zend_xdebug_globals, xdebug_globals)

	/* Base settings */
	STD_PHP_INI_ENTRY("xdebug.max_nesting_level", "512",                PHP_INI_ALL,    OnUpdateLong,   settings.base.max_nesting_level, zend_xdebug_globals, xdebug_globals)

	/* Develop settings */
	STD_PHP_INI_ENTRY("xdebug.cli_color",         "0",                  PHP_INI_ALL,    OnUpdateLong,   settings.develop.cli_color,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.force_display_errors", "0",             PHP_INI_SYSTEM, OnUpdateBool,   settings.develop.force_display_errors, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.force_error_reporting", "0",              PHP_INI_SYSTEM, OnUpdateLong,   settings.develop.force_error_reporting, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.halt_level",        "0",                  PHP_INI_ALL,    OnUpdateLong,   settings.develop.halt_level,        zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.max_stack_frames",  "-1",                 PHP_INI_ALL,    OnUpdateLong,   settings.develop.max_stack_frames,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_error_trace",  "0",                PHP_INI_ALL,    OnUpdateBool,   settings.develop.show_error_trace,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_exception_trace",  "0",            PHP_INI_ALL,    OnUpdateBool,   settings.develop.show_ex_trace,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_local_vars", "0",                  PHP_INI_ALL,    OnUpdateBool,   settings.develop.show_local_vars,   zend_xdebug_globals, xdebug_globals)

	/* Dump superglobals settings */
	PHP_INI_ENTRY("xdebug.dump.COOKIE",           NULL,                 PHP_INI_ALL,    OnUpdateCookie)
	PHP_INI_ENTRY("xdebug.dump.ENV",              NULL,                 PHP_INI_ALL,    OnUpdateEnv)
	PHP_INI_ENTRY("xdebug.dump.FILES",            NULL,                 PHP_INI_ALL,    OnUpdateFiles)
	PHP_INI_ENTRY("xdebug.dump.GET",              NULL,                 PHP_INI_ALL,    OnUpdateGet)
	PHP_INI_ENTRY("xdebug.dump.POST",             NULL,                 PHP_INI_ALL,    OnUpdatePost)
	PHP_INI_ENTRY("xdebug.dump.REQUEST",          NULL,                 PHP_INI_ALL,    OnUpdateRequest)
	PHP_INI_ENTRY("xdebug.dump.SERVER",           NULL,                 PHP_INI_ALL,    OnUpdateServer)
	PHP_INI_ENTRY("xdebug.dump.SESSION",          NULL,                 PHP_INI_ALL,    OnUpdateSession)
	STD_PHP_INI_BOOLEAN("xdebug.dump_globals",    "1",                  PHP_INI_ALL,    OnUpdateBool,   settings.develop.dump_globals,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.dump_once",       "1",                  PHP_INI_ALL,    OnUpdateBool,   settings.develop.dump_once,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.dump_undefined",  "0",                  PHP_INI_ALL,    OnUpdateBool,   settings.develop.dump_undefined,    zend_xdebug_globals, xdebug_globals)

	/* Profiler settings */
	STD_PHP_INI_ENTRY("xdebug.profiler_output_name",      "cachegrind.out.%p",  PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, settings.profiler.profiler_output_name,          zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_append",         "0",                  PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   settings.profiler.profiler_append,               zend_xdebug_globals, xdebug_globals)

	/* Xdebug Cloud */
	STD_PHP_INI_ENTRY("xdebug.cloud_id", "", PHP_INI_SYSTEM, OnUpdateString, settings.debugger.cloud_id, zend_xdebug_globals, xdebug_globals)

	/* Step debugger settings */
	STD_PHP_INI_ENTRY("xdebug.client_host",             "localhost",                        PHP_INI_ALL, OnUpdateString, settings.debugger.client_host,             zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.client_port",             XDEBUG_CLIENT_PORT_S,               PHP_INI_ALL, OnUpdateLong,   settings.debugger.client_port,             zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.discover_client_host",  "0",                                PHP_INI_ALL, OnUpdateBool,   settings.debugger.discover_client_host,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.client_discovery_header", "HTTP_X_FORWARDED_FOR,REMOTE_ADDR", PHP_INI_ALL, OnUpdateString, settings.debugger.client_discovery_header, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.idekey",                  "",                                 PHP_INI_ALL, OnUpdateString, settings.debugger.ide_key_setting,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.connect_timeout_ms",      "200",                              PHP_INI_ALL, OnUpdateLong,   settings.debugger.connect_timeout_ms,      zend_xdebug_globals, xdebug_globals)

	/* Scream support */
	STD_PHP_INI_BOOLEAN("xdebug.scream",                 "0",           PHP_INI_ALL,    OnUpdateBool,   settings.develop.do_scream,            zend_xdebug_globals, xdebug_globals)

	/* GC Stats support */
	STD_PHP_INI_ENTRY("xdebug.gc_stats_output_name", "gcstats.%p",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, settings.gc_stats.output_name, zend_xdebug_globals, xdebug_globals)

	/* Tracing settings */
	STD_PHP_INI_ENTRY("xdebug.trace_output_name", "trace.%c",           PHP_INI_ALL,    OnUpdateString, settings.tracing.trace_output_name, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_format",      "0",                  PHP_INI_ALL,    OnUpdateLong,   settings.tracing.trace_format,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_options",     "0",                  PHP_INI_ALL,    OnUpdateLong,   settings.tracing.trace_options,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_assignments", "0",              PHP_INI_ALL,    OnUpdateBool,   settings.tracing.collect_assignments, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_params", "1",                   PHP_INI_ALL,    OnUpdateBool,   settings.tracing.collect_params,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_return",  "0",                  PHP_INI_ALL,    OnUpdateBool,   settings.tracing.collect_return,    zend_xdebug_globals, xdebug_globals)

	/* Removed/Changed settings */
	XDEBUG_CHANGED_INI_ENTRY(xdebug.auto_trace)
	XDEBUG_REMOVED_INI_ENTRY(xdebug.collect_includes)
	XDEBUG_REMOVED_INI_ENTRY(xdebug.collect_vars)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.coverage_enable)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.default_enable)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.gc_stats_enable)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.gc_stats_output_dir)
	XDEBUG_REMOVED_INI_ENTRY(xdebug.overload_var_dump)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.profiler_enable)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.profiler_enable_trigger)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.profiler_enable_trigger_value)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.profiler_output_dir)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.remote_autostart)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.remote_connect_back)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.remote_enable)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.remote_host)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.remote_log)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.remote_log_level)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.remote_mode)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.remote_port)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.remote_timeout)
	XDEBUG_REMOVED_INI_ENTRY(xdebug.show_mem_delta)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.trace_output_dir)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.trace_enable_trigger)
	XDEBUG_CHANGED_INI_ENTRY(xdebug.trace_enable_trigger_value)
PHP_INI_END()

static void xdebug_init_base_globals(xdebug_base_globals_t *xg)
{
	xg->stack                = NULL;
	xg->in_debug_info        = 0;
	xg->output_is_tty        = OUTPUT_NOT_CHECKED;
	xg->in_execution         = 0;
	xg->in_var_serialisation = 0;
	xg->error_reporting_override   = 0;
	xg->error_reporting_overridden = 0;

	xg->filter_type_code_coverage = XDEBUG_FILTER_NONE;
	xg->filter_type_stack         = XDEBUG_FILTER_NONE;
	xg->filter_type_tracing       = XDEBUG_FILTER_NONE;
	xg->filters_code_coverage     = NULL;
	xg->filters_stack             = NULL;
	xg->filters_tracing           = NULL;

	xg->php_version_compile_time = PHP_VERSION;
	xg->php_version_run_time     = zend_get_module_version("standard");

	xdebug_nanotime_init(xg);
}


static void php_xdebug_init_globals(zend_xdebug_globals *xg)
{
	memset(&xg->globals, 0, sizeof(xg->globals));

	xdebug_init_library_globals(&xg->globals.library);
	xdebug_init_base_globals(&xg->globals.base);

	if (XDEBUG_MODE_IS(XDEBUG_MODE_COVERAGE)) {
		xdebug_init_coverage_globals(&xg->globals.coverage);
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_init_debugger_globals(&xg->globals.debugger);
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		xdebug_init_develop_globals(&xg->globals.develop);
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_PROFILING)) {
		xdebug_init_profiler_globals(&xg->globals.profiler);
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_GCSTATS)) {
		xdebug_init_gc_stats_globals(&xg->globals.gc_stats);
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_TRACING)) {
		xdebug_init_tracing_globals(&xg->globals.tracing);
	}
}

static void php_xdebug_shutdown_globals(zend_xdebug_globals *xg)
{
	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		xdebug_deinit_develop_globals(&xg->globals.develop);
	}
}


static void xdebug_env_config(void)
{
	char       *config = getenv("XDEBUG_CONFIG");
	xdebug_arg *parts;
	int			i;
	/*
		XDEBUG_CONFIG format:
		XDEBUG_CONFIG=var=val var=val
	*/
	if (!config) {
		return;
	}

	parts = xdebug_arg_ctor();
	xdebug_explode(" ", config, parts, -1);

	for (i = 0; i < parts->c; ++i) {
		const char *name = NULL;
		char *envvar = parts->args[i];
		char *envval = NULL;
		char *eq = strchr(envvar, '=');
		if (!eq || !*eq) {
			continue;
		}
		*eq = 0;
		envval = eq + 1;
		if (!*envval) {
			continue;
		}

		if (strcasecmp(envvar, "discover_client_host") == 0) {
			name = "xdebug.discover_client_host";
		} else
		if (strcasecmp(envvar, "client_port") == 0) {
			name = "xdebug.client_port";
		} else
		if (strcasecmp(envvar, "client_host") == 0) {
			name = "xdebug.client_host";
		} else
		if (strcasecmp(envvar, "cloud_id") == 0) {
			name = "xdebug.cloud_id";
		} else
		if (strcasecmp(envvar, "idekey") == 0) {
			xdebug_debugger_reset_ide_key(envval);
		} else
		if (strcasecmp(envvar, "output_dir") == 0) {
			name = "xdebug.output_dir";
		} else
		if (strcasecmp(envvar, "profiler_output_name") == 0) {
			name = "xdebug.profiler_output_name";
		} else
		if (strcasecmp(envvar, "log") == 0) {
			name = "xdebug.log";
		} else
		if (strcasecmp(envvar, "log_level") == 0) {
			name = "xdebug.log_level";
		} else
		if (strcasecmp(envvar, "cli_color") == 0) {
			name = "xdebug.cli_color";
		}

		if (name) {
			zend_string *ini_name = zend_string_init(name, strlen(name), 0);
			zend_string *ini_val = zend_string_init(envval, strlen(envval), 0);
			zend_alter_ini_entry(ini_name, ini_val, PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
			zend_string_release(ini_val);
			zend_string_release(ini_name);
		}
	}

	xdebug_arg_dtor(parts);
}

int xdebug_is_output_tty(void)
{
	if (XG_BASE(output_is_tty) == OUTPUT_NOT_CHECKED) {
#ifndef PHP_WIN32
		XG_BASE(output_is_tty) = isatty(STDOUT_FILENO);
#else
		XG_BASE(output_is_tty) = getenv("ANSICON") != NULL;
#endif
	}
	return (XG_BASE(output_is_tty));
}

PHP_MINIT_FUNCTION(xdebug)
{
	ZEND_INIT_MODULE_GLOBALS(xdebug, php_xdebug_init_globals, php_xdebug_shutdown_globals);
	REGISTER_INI_ENTRIES();

	/* Locking in mode as it currently is */
	if (!xdebug_lib_set_mode(XG(settings.library.requested_mode))) {
		xdebug_lib_set_mode("develop");
	}

	if (XDEBUG_MODE_IS_OFF()) {
		return SUCCESS;
	}

	xdebug_library_minit();
	xdebug_base_minit(INIT_FUNC_ARGS_PASSTHRU);

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_debugger_minit();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		xdebug_develop_minit(INIT_FUNC_ARGS_PASSTHRU);
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_GCSTATS)) {
		xdebug_gcstats_minit();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_PROFILING)) {
		xdebug_profiler_minit();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_TRACING)) {
		xdebug_tracing_minit(INIT_FUNC_ARGS_PASSTHRU);
	}

	/* Overload the "include_or_eval" opcode if the mode is 'debug' or 'trace' */
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG) || XDEBUG_MODE_IS(XDEBUG_MODE_TRACING)) {
		xdebug_register_with_opcode_multi_handler(ZEND_INCLUDE_OR_EVAL, xdebug_include_or_eval_handler);
	}

	/* Coverage must be last, as it has a catch all override for opcodes */
	if (XDEBUG_MODE_IS(XDEBUG_MODE_COVERAGE)) {
		xdebug_coverage_minit(INIT_FUNC_ARGS_PASSTHRU);
	}

	if (zend_xdebug_initialised == 0) {
		zend_error(E_WARNING, "Xdebug MUST be loaded as a Zend extension");
	}

	xdebug_coverage_register_constants(INIT_FUNC_ARGS_PASSTHRU);
	xdebug_filter_register_constants(INIT_FUNC_ARGS_PASSTHRU);
	xdebug_tracing_register_constants(INIT_FUNC_ARGS_PASSTHRU);

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(xdebug)
{
	if (XDEBUG_MODE_IS_OFF()) {
#ifdef ZTS
		ts_free_id(xdebug_globals_id);
#endif
		return SUCCESS;
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_GCSTATS)) {
		xdebug_gcstats_mshutdown();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_PROFILING)) {
		xdebug_profiler_mshutdown();
	}

	xdebug_library_mshutdown();

#ifdef ZTS
	ts_free_id(xdebug_globals_id);
#else
	php_xdebug_shutdown_globals(&xdebug_globals);
#endif

	return SUCCESS;
}

static void xdebug_init_auto_globals(void)
{
	zend_is_auto_global_str((char*) ZEND_STRL("_ENV"));
	zend_is_auto_global_str((char*) ZEND_STRL("_GET"));
	zend_is_auto_global_str((char*) ZEND_STRL("_POST"));
	zend_is_auto_global_str((char*) ZEND_STRL("_COOKIE"));
	zend_is_auto_global_str((char*) ZEND_STRL("_REQUEST"));
	zend_is_auto_global_str((char*) ZEND_STRL("_FILES"));
	zend_is_auto_global_str((char*) ZEND_STRL("_SERVER"));
	zend_is_auto_global_str((char*) ZEND_STRL("_SESSION"));
}


PHP_RINIT_FUNCTION(xdebug)
{
#if defined(ZTS) && defined(COMPILE_DL_XDEBUG)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	if (XDEBUG_MODE_IS_OFF()) {
		return SUCCESS;
	}

	xdebug_library_rinit();

	if (XDEBUG_MODE_IS(XDEBUG_MODE_COVERAGE)) {
		xdebug_coverage_rinit();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_debugger_rinit();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		xdebug_develop_rinit();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_GCSTATS)) {
		xdebug_gcstats_rinit();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_PROFILING)) {
		xdebug_profiler_rinit();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_TRACING)) {
		xdebug_tracing_rinit();
	}

	/* Get xdebug ini entries from the environment also,
	   this can override the idekey if one is set */
	xdebug_env_config();

	xdebug_init_auto_globals();

	/* Only enabled extended info when it is not disabled */
	CG(compiler_options) = CG(compiler_options) | ZEND_COMPILE_EXTENDED_STMT;

	xdebug_base_rinit();

	return SUCCESS;
}

ZEND_MODULE_POST_ZEND_DEACTIVATE_D(xdebug)
{
	if (XDEBUG_MODE_IS_OFF()) {
		return SUCCESS;
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_COVERAGE)) {
		xdebug_coverage_post_deactivate();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_debugger_post_deactivate();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		xdebug_develop_post_deactivate();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_PROFILING)) {
		xdebug_profiler_post_deactivate();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_TRACING)) {
		xdebug_tracing_post_deactivate();
	}

	xdebug_base_post_deactivate();
	xdebug_library_post_deactivate();

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(xdebug)
{
	if (XDEBUG_MODE_IS_OFF()) {
		return SUCCESS;
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		xdebug_develop_rshutdown();
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_GCSTATS)) {
		xdebug_gcstats_rshutdown();
	}

	xdebug_base_rshutdown();

	return SUCCESS;
}

PHP_MINFO_FUNCTION(xdebug)
{
	xdebug_print_info();

	if (zend_xdebug_initialised == 0) {
		php_info_print_table_start();
		php_info_print_table_header(1, "XDEBUG NOT LOADED AS ZEND EXTENSION");
		php_info_print_table_end();
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_debugger_minfo();
	}

	DISPLAY_INI_ENTRIES();
}

ZEND_DLEXPORT void xdebug_statement_call(zend_execute_data *frame)
{
	zend_op_array *op_array = &frame->func->op_array;
	int                   lineno;

	if (XDEBUG_MODE_IS_OFF()) {
		return;
	}

	if (!EG(current_execute_data)) {
		return;
	}

	lineno = EG(current_execute_data)->opline->lineno;

	if (XDEBUG_MODE_IS(XDEBUG_MODE_COVERAGE)) {
		xdebug_coverage_count_line_if_active(op_array, op_array->filename, lineno);
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_debugger_statement_call(op_array->filename, lineno);
	}
}

ZEND_DLEXPORT int xdebug_zend_startup(zend_extension *extension)
{
	xdebug_library_zend_startup();
	xdebug_debugger_zend_startup();

	zend_xdebug_initialised = 1;

	xdebug_orig_post_startup_cb = zend_post_startup_cb;
	zend_post_startup_cb = xdebug_post_startup;

	return zend_startup_module(&xdebug_module_entry);
}

static zend_result xdebug_post_startup(void)
{
	if (xdebug_orig_post_startup_cb) {
		int (*cb)(void) = xdebug_orig_post_startup_cb;

		xdebug_orig_post_startup_cb = NULL;
		if (cb() != SUCCESS) {
			return FAILURE;
		}
	}

	xdebug_base_post_startup();

	return SUCCESS;
}

ZEND_DLEXPORT void xdebug_zend_shutdown(zend_extension *extension)
{
	xdebug_debugger_zend_shutdown();

	xdebug_library_zend_shutdown();
}

ZEND_DLEXPORT void xdebug_init_oparray(zend_op_array *op_array)
{
	if (XDEBUG_MODE_IS_OFF()) {
		return;
	}

	xdebug_coverage_init_oparray(op_array);
}

#ifndef ZEND_EXT_API
#define ZEND_EXT_API    ZEND_DLEXPORT
#endif

ZEND_EXT_API zend_extension_version_info extension_version_info = { ZEND_EXTENSION_API_NO, (char*) ZEND_EXTENSION_BUILD_ID };

ZEND_DLEXPORT zend_extension zend_extension_entry = {
	(char*) XDEBUG_NAME,
	(char*) XDEBUG_VERSION,
	(char*) XDEBUG_AUTHOR,
	(char*) XDEBUG_URL_FAQ,
	(char*) XDEBUG_COPYRIGHT_SHORT,
	xdebug_zend_startup,
	xdebug_zend_shutdown,
	NULL,           /* activate_func_t */
	NULL,           /* deactivate_func_t */
	NULL,           /* message_handler_func_t */
	NULL,           /* op_array_handler_func_t */
	xdebug_statement_call, /* statement_handler_func_t */
	NULL,           /* fcall_begin_handler_func_t */
	NULL,           /* fcall_end_handler_func_t */
	xdebug_init_oparray,   /* op_array_ctor_func_t */
	NULL,           /* op_array_dtor_func_t */
	STANDARD_ZEND_EXTENSION_PROPERTIES
};

#endif
