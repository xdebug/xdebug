/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2019 Derick Rethans                               |
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
   |          Ilia Alshanetsky <ilia@prohost.org>                         |
   |          Harald Radi <harald.radi@nme.at>                            |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
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
#include "main/php_ini.h"
#include "ext/standard/head.h"
#include "ext/standard/html.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "php_globals.h"
#include "main/php_output.h"
#include "ext/standard/php_var.h"

#include "php_xdebug.h"

#include "base/base.h"
#include "base/filter.h"
#include "base/monitor.h"
#include "base/stack.h"
#include "base/superglobals.h"
#include "coverage/code_coverage.h"
#include "debugger/com.h"
#include "gcstats/gc_stats.h"
#include "lib/usefulstuff.h"
#include "lib/llist.h"
#include "lib/mm.h"
#include "lib/private.h"
#include "lib/var.h"
#include "profiler/profiler.h"
#include "tracing/tracing.h"

#if PHP_VERSION_ID >= 70300
static int (*xdebug_orig_post_startup_cb)(void);
static int xdebug_post_startup(void);
#endif

static int xdebug_header_handler(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s TSRMLS_DC);
static size_t xdebug_ub_write(const char *string, size_t length TSRMLS_DC);

int xdebug_exit_handler(zend_execute_data *execute_data);

int zend_xdebug_initialised = 0;
int zend_xdebug_filter_offset = -1;
int zend_xdebug_cc_run_offset = -1;

static int (*xdebug_orig_header_handler)(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s TSRMLS_DC);
static size_t (*xdebug_orig_ub_write)(const char *string, size_t len TSRMLS_DC);

ZEND_BEGIN_ARG_INFO_EX(xdebug_void_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_print_function_stack_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, message)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_call_class_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, depth)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_call_function_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, depth)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_call_file_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, depth)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_call_line_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, depth)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_var_dump_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_debug_zval_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_debug_zval_stdout_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_start_trace_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, fname)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_dump_aggr_profiling_data_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, prefix)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_get_collected_errors_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, clear)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_start_function_monitor_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, functions_to_monitor)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_get_monitored_functions_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, clear)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_start_code_coverage_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_stop_code_coverage_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, cleanup)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_start_gcstats_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, fname)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_stop_gcstats_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(xdebug_set_filter_args, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 3)
	ZEND_ARG_INFO(0, filter_group)
	ZEND_ARG_INFO(0, filter_type)
	ZEND_ARG_INFO(0, array_of_filters)
ZEND_END_ARG_INFO()

zend_function_entry xdebug_functions[] = {
	PHP_FE(xdebug_get_stack_depth,       xdebug_void_args)
	PHP_FE(xdebug_get_function_stack,    xdebug_void_args)
	PHP_FE(xdebug_get_formatted_function_stack,    xdebug_void_args)
	PHP_FE(xdebug_print_function_stack,  xdebug_print_function_stack_args)
	PHP_FE(xdebug_get_declared_vars,     xdebug_void_args)
	PHP_FE(xdebug_call_class,            xdebug_call_class_args)
	PHP_FE(xdebug_call_function,         xdebug_call_function_args)
	PHP_FE(xdebug_call_file,             xdebug_call_file_args)
	PHP_FE(xdebug_call_line,             xdebug_call_line_args)

	PHP_FE(xdebug_var_dump,              xdebug_var_dump_args)
	PHP_FE(xdebug_debug_zval,            xdebug_debug_zval_args)
	PHP_FE(xdebug_debug_zval_stdout,     xdebug_debug_zval_stdout_args)

	PHP_FE(xdebug_enable,                xdebug_void_args)
	PHP_FE(xdebug_disable,               xdebug_void_args)
	PHP_FE(xdebug_is_enabled,            xdebug_void_args)
	PHP_FE(xdebug_is_debugger_active,    xdebug_void_args)
	PHP_FE(xdebug_break,                 xdebug_void_args)

	PHP_FE(xdebug_start_trace,           xdebug_start_trace_args)
	PHP_FE(xdebug_stop_trace,            xdebug_void_args)
	PHP_FE(xdebug_get_tracefile_name,    xdebug_void_args)

	PHP_FE(xdebug_get_profiler_filename, xdebug_void_args)
	PHP_FE(xdebug_dump_aggr_profiling_data, xdebug_dump_aggr_profiling_data_args)
	PHP_FE(xdebug_clear_aggr_profiling_data, xdebug_void_args)

	PHP_FE(xdebug_start_gcstats,         xdebug_start_gcstats_args)
	PHP_FE(xdebug_stop_gcstats,          xdebug_stop_gcstats_args)
	PHP_FE(xdebug_get_gcstats_filename,  xdebug_void_args)
	PHP_FE(xdebug_get_gc_run_count,      xdebug_void_args)
	PHP_FE(xdebug_get_gc_total_collected_roots, xdebug_void_args)

	PHP_FE(xdebug_memory_usage,          xdebug_void_args)
	PHP_FE(xdebug_peak_memory_usage,     xdebug_void_args)
	PHP_FE(xdebug_time_index,            xdebug_void_args)

	PHP_FE(xdebug_start_error_collection, xdebug_void_args)
	PHP_FE(xdebug_stop_error_collection, xdebug_void_args)
	PHP_FE(xdebug_get_collected_errors,  xdebug_get_collected_errors_args)

	PHP_FE(xdebug_start_function_monitor, xdebug_start_function_monitor_args)
	PHP_FE(xdebug_stop_function_monitor, xdebug_void_args)
	PHP_FE(xdebug_get_monitored_functions, xdebug_get_monitored_functions_args)

	PHP_FE(xdebug_start_code_coverage,   xdebug_start_code_coverage_args)
	PHP_FE(xdebug_stop_code_coverage,    xdebug_stop_code_coverage_args)
	PHP_FE(xdebug_get_code_coverage,     xdebug_void_args)
	PHP_FE(xdebug_code_coverage_started, xdebug_void_args)
	PHP_FE(xdebug_get_function_count,    xdebug_void_args)

	PHP_FE(xdebug_dump_superglobals,     xdebug_void_args)
	PHP_FE(xdebug_get_headers,           xdebug_void_args)

	PHP_FE(xdebug_set_filter,            xdebug_set_filter_args)
	{NULL, NULL, 0, 0, 0}
};

zend_module_entry xdebug_module_entry = {
	STANDARD_MODULE_HEADER,
	"xdebug",
	xdebug_functions,
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

static PHP_INI_MH(OnUpdateDebugMode)
{
	if (!new_value) {
		XINI_DBG(remote_mode) = XDEBUG_NONE;

	} else if (strcmp(STR_NAME_VAL(new_value), "jit") == 0) {
		XINI_DBG(remote_mode) = XDEBUG_JIT;

	} else if (strcmp(STR_NAME_VAL(new_value), "req") == 0) {
		XINI_DBG(remote_mode) = XDEBUG_REQ;

	} else {
		XINI_DBG(remote_mode) = XDEBUG_NONE;
	}
	return SUCCESS;
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

PHP_INI_BEGIN()
	/* Debugger settings */
	STD_PHP_INI_BOOLEAN("xdebug.auto_trace",      "0",                  PHP_INI_ALL,    OnUpdateBool,   trace.settings.auto_trace,        zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.trace_enable_trigger", "0",             PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   trace.settings.trace_enable_trigger, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_enable_trigger_value", "",          PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString,   trace.settings.trace_enable_trigger_value, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_output_dir",  XDEBUG_TEMP_DIR,      PHP_INI_ALL,    OnUpdateString, trace.settings.trace_output_dir,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_output_name", "trace.%c",           PHP_INI_ALL,    OnUpdateString, trace.settings.trace_output_name, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_format",      "0",                  PHP_INI_ALL,    OnUpdateLong,   trace.settings.trace_format,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_options",     "0",                  PHP_INI_ALL,    OnUpdateLong,   trace.settings.trace_options,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.coverage_enable", "1",                  PHP_INI_SYSTEM, OnUpdateBool,   coverage.settings.code_coverage_enable, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_includes","1",                  PHP_INI_ALL,    OnUpdateBool,   base.settings.collect_includes,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.collect_params",  "0",                    PHP_INI_ALL,    OnUpdateLong,   base.settings.collect_params,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_return",  "0",                  PHP_INI_ALL,    OnUpdateBool,   base.settings.collect_return,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_vars",    "0",                  PHP_INI_ALL,    OnUpdateBool,   base.settings.collect_vars,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_assignments", "0",              PHP_INI_ALL,    OnUpdateBool,   base.settings.collect_assignments, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.default_enable",  "1",                  PHP_INI_ALL,    OnUpdateBool,   base.settings.default_enable,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.file_link_format",  "",                   PHP_INI_ALL,    OnUpdateString, base.settings.file_link_format,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.filename_format",   "",                   PHP_INI_ALL,    OnUpdateString, base.settings.filename_format,   zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.force_display_errors", "0",             PHP_INI_SYSTEM, OnUpdateBool,   base.settings.force_display_errors, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.force_error_reporting", "0",              PHP_INI_SYSTEM, OnUpdateLong,   base.settings.force_error_reporting, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.halt_level",        "0",                  PHP_INI_ALL,    OnUpdateLong,   base.settings.halt_level,        zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.max_nesting_level", "256",                PHP_INI_ALL,    OnUpdateLong,   base.settings.max_nesting_level, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.max_stack_frames",  "-1",                 PHP_INI_ALL,    OnUpdateLong,   base.settings.max_stack_frames,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.overload_var_dump", "2",                  PHP_INI_ALL,    OnUpdateLong,   base.settings.overload_var_dump, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_error_trace",  "0",                PHP_INI_ALL,    OnUpdateBool,   base.settings.show_error_trace,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_exception_trace",  "0",            PHP_INI_ALL,    OnUpdateBool,   base.settings.show_ex_trace,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_local_vars", "0",                  PHP_INI_ALL,    OnUpdateBool,   base.settings.show_local_vars,   zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_mem_delta",  "0",                  PHP_INI_ALL,    OnUpdateBool,   base.settings.show_mem_delta,    zend_xdebug_globals, xdebug_globals)

	/* Dump superglobals settings */
	PHP_INI_ENTRY("xdebug.dump.COOKIE",           NULL,                 PHP_INI_ALL,    OnUpdateCookie)
	PHP_INI_ENTRY("xdebug.dump.ENV",              NULL,                 PHP_INI_ALL,    OnUpdateEnv)
	PHP_INI_ENTRY("xdebug.dump.FILES",            NULL,                 PHP_INI_ALL,    OnUpdateFiles)
	PHP_INI_ENTRY("xdebug.dump.GET",              NULL,                 PHP_INI_ALL,    OnUpdateGet)
	PHP_INI_ENTRY("xdebug.dump.POST",             NULL,                 PHP_INI_ALL,    OnUpdatePost)
	PHP_INI_ENTRY("xdebug.dump.REQUEST",          NULL,                 PHP_INI_ALL,    OnUpdateRequest)
	PHP_INI_ENTRY("xdebug.dump.SERVER",           NULL,                 PHP_INI_ALL,    OnUpdateServer)
	PHP_INI_ENTRY("xdebug.dump.SESSION",          NULL,                 PHP_INI_ALL,    OnUpdateSession)
	STD_PHP_INI_BOOLEAN("xdebug.dump_globals",    "1",                  PHP_INI_ALL,    OnUpdateBool,   base.settings.dump_globals,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.dump_once",       "1",                  PHP_INI_ALL,    OnUpdateBool,   base.settings.dump_once,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.dump_undefined",  "0",                  PHP_INI_ALL,    OnUpdateBool,   base.settings.dump_undefined,    zend_xdebug_globals, xdebug_globals)

	/* Profiler settings */
	STD_PHP_INI_BOOLEAN("xdebug.profiler_enable",         "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler.settings.profiler_enable,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.profiler_output_dir",       XDEBUG_TEMP_DIR,      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, profiler.settings.profiler_output_dir,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.profiler_output_name",      "cachegrind.out.%p",  PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, profiler.settings.profiler_output_name,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_enable_trigger", "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler.settings.profiler_enable_trigger, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.profiler_enable_trigger_value", "",   PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString,   profiler.settings.profiler_enable_trigger_value, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_append",         "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler.settings.profiler_append,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_aggregate",      "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler.settings.profiler_aggregate,      zend_xdebug_globals, xdebug_globals)

	/* Remote debugger settings */
	STD_PHP_INI_BOOLEAN("xdebug.remote_enable",   "0",   PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   stepdbg.settings.remote_enable,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_handler",    "dbgp",               PHP_INI_ALL,    OnUpdateString, stepdbg.settings.remote_handler,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_host",       "localhost",          PHP_INI_ALL,    OnUpdateString, stepdbg.settings.remote_host,       zend_xdebug_globals, xdebug_globals)
	PHP_INI_ENTRY("xdebug.remote_mode",           "req",                PHP_INI_ALL,    OnUpdateDebugMode)
	STD_PHP_INI_ENTRY("xdebug.remote_port",       "9000",               PHP_INI_ALL,    OnUpdateLong,   stepdbg.settings.remote_port,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.remote_autostart","0",                  PHP_INI_ALL,    OnUpdateBool,   stepdbg.settings.remote_autostart,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.remote_connect_back","0",               PHP_INI_ALL,    OnUpdateBool,   stepdbg.settings.remote_connect_back,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_log",        "",                   PHP_INI_ALL,    OnUpdateString, stepdbg.settings.remote_log,        zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_log_level",  XDEBUG_LOG_DEFAULT,   PHP_INI_ALL,    OnUpdateLong,   stepdbg.settings.remote_log_level,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.idekey",            "",                   PHP_INI_ALL,    OnUpdateString, stepdbg.settings.ide_key_setting,   zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_cookie_expire_time", "3600",       PHP_INI_ALL,    OnUpdateLong,   stepdbg.settings.remote_cookie_expire_time, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_addr_header", "",                  PHP_INI_ALL,    OnUpdateString, stepdbg.settings.remote_addr_header, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_timeout",    "200",                PHP_INI_ALL,    OnUpdateLong,   stepdbg.settings.remote_connect_timeout, zend_xdebug_globals, xdebug_globals)

	/* Variable display settings */
	STD_PHP_INI_ENTRY("xdebug.var_display_max_children", "128",         PHP_INI_ALL,    OnUpdateLong,   base.settings.display_max_children, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.var_display_max_data",     "512",         PHP_INI_ALL,    OnUpdateLong,   base.settings.display_max_data,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.var_display_max_depth",    "3",           PHP_INI_ALL,    OnUpdateLong,   base.settings.display_max_depth,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.cli_color",                "0",           PHP_INI_ALL,    OnUpdateLong,   base.settings.cli_color,            zend_xdebug_globals, xdebug_globals)

	/* Scream support */
	STD_PHP_INI_BOOLEAN("xdebug.scream",                 "0",           PHP_INI_ALL,    OnUpdateBool,   base.settings.do_scream,            zend_xdebug_globals, xdebug_globals)

	/* GC Stats support */
	STD_PHP_INI_BOOLEAN("xdebug.gc_stats_enable",    "0",               PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   settings.gc_stats.enable,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.gc_stats_output_dir",  XDEBUG_TEMP_DIR,   PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, settings.gc_stats.output_dir,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.gc_stats_output_name", "gcstats.%p",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, settings.gc_stats.output_name, zend_xdebug_globals, xdebug_globals)
PHP_INI_END()

static void xdebug_init_base_globals(struct xdebug_base_info *xg)
{
	xg->level                = 0;
	xg->stack                = NULL;
	xg->headers              = NULL;
	xg->in_debug_info        = 0;
	xg->output_is_tty        = OUTPUT_NOT_CHECKED;
	xg->do_monitor_functions = 0;
	xg->headers              = NULL;
	xg->in_at                = 0; /* scream */
	xg->in_execution         = 0;
	xg->in_var_serialisation = 0;
	xg->error_reporting_override   = 0;
	xg->error_reporting_overridden = 0;

	xg->filter_type_tracing       = XDEBUG_FILTER_NONE;
	xg->filter_type_profiler      = XDEBUG_FILTER_NONE;
	xg->filter_type_code_coverage = XDEBUG_FILTER_NONE;
	xg->filters_tracing           = NULL;
	xg->filters_code_coverage     = NULL;

	xdebug_llist_init(&xg->server, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->get, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->post, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->cookie, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->files, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->env, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->request, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->session, xdebug_superglobals_dump_dtor);
}

static void xdebug_init_stepdbg_globals(struct xdebug_stepdbg_info *xg)
{
	xg->breakpoint_count     = 0;
	xg->ide_key              = NULL;
	xg->stdout_mode          = 0;
	xg->no_exec              = 0;
	xg->context.program_name = NULL;
	xg->context.list.last_file = NULL;
	xg->context.list.last_line = 0;
	xg->context.do_break     = 0;
	xg->context.do_step      = 0;
	xg->context.do_next      = 0;
	xg->context.do_finish    = 0;
	xg->active_execute_data  = NULL;

	xg->remote_connection_enabled = 0;
	xg->remote_connection_pid     = 0;
	xg->remote_log_file           = 0;
	xg->breakpoints_allowed       = 0;
}

static void xdebug_init_trace_globals(struct xdebug_trace_info *xg)
{
	xg->trace_handler        = NULL;
	xg->trace_context        = NULL;
}

static void xdebug_init_coverage_globals(struct xdebug_coverage_info *xg)
{
	xg->previous_filename    = NULL;
	xg->previous_file        = NULL;
	xg->previous_mark_filename = NULL;
	xg->previous_mark_file     = NULL;
	xg->paths_stack = NULL;
	xg->branches.size        = 0;
	xg->branches.last_branch_nr = NULL;
	xg->code_coverage_active = 0;

	/* Get reserved offset */
	xg->dead_code_analysis_tracker_offset = zend_xdebug_cc_run_offset;
	xg->dead_code_last_start_id = 1;
	xg->code_coverage_filter_offset = zend_xdebug_filter_offset;
}

static void xdebug_init_profiler_globals(struct xdebug_profiler_info *xg)
{
	xg->profiler_enabled     = 0;
}

static void php_xdebug_init_globals (zend_xdebug_globals *xg TSRMLS_DC)
{
	xdebug_init_base_globals(&xg->base);
	xdebug_init_stepdbg_globals(&xg->stepdbg);
	xdebug_init_trace_globals(&xg->trace);
	xdebug_init_coverage_globals(&xg->coverage);
	xdebug_init_profiler_globals(&xg->profiler);
	xdebug_init_gc_stats_globals(&xg->globals.gc_stats);

	/* Override header generation in SAPI */
	if (sapi_module.header_handler != xdebug_header_handler) {
		xdebug_orig_header_handler = sapi_module.header_handler;
		sapi_module.header_handler = xdebug_header_handler;
	}

	/* Capturing output */
	if (sapi_module.ub_write != xdebug_ub_write) {
		xdebug_orig_ub_write = sapi_module.ub_write;
		sapi_module.ub_write = xdebug_ub_write;
	}
}

static void xdebug_deinit_base_globals(struct xdebug_base_info *xg)
{
	xdebug_llist_empty(&xg->server, NULL);
	xdebug_llist_empty(&xg->get, NULL);
	xdebug_llist_empty(&xg->post, NULL);
	xdebug_llist_empty(&xg->cookie, NULL);
	xdebug_llist_empty(&xg->files, NULL);
	xdebug_llist_empty(&xg->env, NULL);
	xdebug_llist_empty(&xg->request, NULL);
	xdebug_llist_empty(&xg->session, NULL);
}

static void php_xdebug_shutdown_globals (zend_xdebug_globals *xg TSRMLS_DC)
{
	xdebug_deinit_base_globals(&xg->base);
}

char *xdebug_env_key(TSRMLS_D)
{
	char *ide_key;

	ide_key = XINI_DBG(ide_key_setting);
	if (ide_key && *ide_key) {
		return ide_key;
	}

	ide_key = getenv("DBGP_IDEKEY");
	if (ide_key && *ide_key) {
		return ide_key;
	}

	ide_key = getenv("USER");
	if (ide_key && *ide_key) {
		return ide_key;
	}

	ide_key = getenv("USERNAME");
	if (ide_key && *ide_key) {
		return ide_key;
	}

	return NULL;
}

void xdebug_env_config(TSRMLS_D)
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

	parts = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));
	xdebug_arg_init(parts);
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

		if (strcasecmp(envvar, "remote_connect_back") == 0) {
			name = "xdebug.remote_connect_back";
		} else
		if (strcasecmp(envvar, "remote_enable") == 0) {
			name = "xdebug.remote_enable";
		} else
		if (strcasecmp(envvar, "remote_port") == 0) {
			name = "xdebug.remote_port";
		} else
		if (strcasecmp(envvar, "remote_host") == 0) {
			name = "xdebug.remote_host";
		} else
		if (strcasecmp(envvar, "remote_handler") == 0) {
			name = "xdebug.remote_handler";
		} else
		if (strcasecmp(envvar, "remote_mode") == 0) {
			name = "xdebug.remote_mode";
		} else
		if (strcasecmp(envvar, "idekey") == 0) {
			if (XG_DBG(ide_key)) {
				xdfree(XG_DBG(ide_key));
			}
			XG_DBG(ide_key) = xdstrdup(envval);
		} else
		if (strcasecmp(envvar, "profiler_enable") == 0) {
			name = "xdebug.profiler_enable";
		} else
		if (strcasecmp(envvar, "profiler_output_dir") == 0) {
			name = "xdebug.profiler_output_dir";
		} else
		if (strcasecmp(envvar, "profiler_output_name") == 0) {
			name = "xdebug.profiler_output_name";
		} else
		if (strcasecmp(envvar, "profiler_enable_trigger") == 0) {
			name = "xdebug.profiler_enable_trigger";
		} else
		if (strcasecmp(envvar, "trace_enable") == 0) {
			name = "xdebug.trace_enable";
		} else
		if (strcasecmp(envvar, "remote_log") == 0) {
			name = "xdebug.remote_log";
		} else
		if (strcasecmp(envvar, "remote_log_level") == 0) {
			name = "xdebug.remote_log_level";
		} else
		if (strcasecmp(envvar, "remote_cookie_expire_time") == 0) {
			name = "xdebug.remote_cookie_expire_time";
		}
		else if (strcasecmp(envvar, "cli_color") == 0) {
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

#if PHP_VERSION_ID >= 70200
static int xdebug_switch_handler(zend_execute_data *execute_data)
{
	if (XG_COV(code_coverage_active)) {
		execute_data->opline++;
		return ZEND_USER_OPCODE_CONTINUE;
	}
	return ZEND_USER_OPCODE_DISPATCH;
}
#endif

static int xdebug_include_or_eval_handler(zend_execute_data *execute_data)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	const zend_op *opline = execute_data->opline;

	if (!op_array->reserved[XG_COV(code_coverage_filter_offset)] && XG_COV(code_coverage_active)) {
		const zend_op *cur_opcode = EG(current_execute_data)->opline;
		xdebug_print_opcode_info('I', execute_data, cur_opcode TSRMLS_CC);
	}
	if (opline->extended_value == ZEND_EVAL) {
		zval *inc_filename;
		zval tmp_inc_filename;
		int  is_var;

		inc_filename = xdebug_get_zval(execute_data, opline->op1_type, &opline->op1, &is_var);

		/* If there is no inc_filename, we're just bailing out instead */
		if (!inc_filename) {
			return ZEND_USER_OPCODE_DISPATCH;
		}

		if (Z_TYPE_P(inc_filename) != IS_STRING) {
			tmp_inc_filename = *inc_filename;
			zval_copy_ctor(&tmp_inc_filename);
			convert_to_string(&tmp_inc_filename);
			inc_filename = &tmp_inc_filename;
		}

		/* Now let's store this info */
		if (XG_BASE(last_eval_statement)) {
			efree(XG_BASE(last_eval_statement));
		}
		XG_BASE(last_eval_statement) = estrndup(Z_STRVAL_P(inc_filename), Z_STRLEN_P(inc_filename));

		if (inc_filename == &tmp_inc_filename) {
			zval_dtor(&tmp_inc_filename);
		}
	}
	return ZEND_USER_OPCODE_DISPATCH;
}

int xdebug_is_output_tty(TSRMLS_D)
{
	if (XG_BASE(output_is_tty) == OUTPUT_NOT_CHECKED) {
#ifndef PHP_WIN32
		XG_BASE(output_is_tty) = isatty(STDOUT_FILENO);
#else
		XG_BASE(output_is_tty) = getenv("ANSICON");
#endif
	}
	return (XG_BASE(output_is_tty));
}

#if 0
int static xdebug_stack_insert_top(zend_stack *stack, const void *element, int size)
{
	int i;

    if (stack->top >= stack->max) {    /* we need to allocate more memory */
        stack->elements = (void **) erealloc(stack->elements,
                   (sizeof(void **) * (stack->max += 64)));
        if (!stack->elements) {
            return FAILURE;
        }
    }

	/* move all existing ones up */
	for (i = stack->top; i >= 0; i--) {
		stack->elements[i + 1] = stack->elements[i];
	}

	/* replace top handler */
    stack->elements[0] = (void *) emalloc(size);
    memcpy(stack->elements[0], element, size);
    return stack->top++;
}
#endif

PHP_MINIT_FUNCTION(xdebug)
{
	zend_extension dummy_ext;

	ZEND_INIT_MODULE_GLOBALS(xdebug, php_xdebug_init_globals, php_xdebug_shutdown_globals);
	REGISTER_INI_ENTRIES();

	xdebug_base_minit(INIT_FUNC_ARGS_PASSTHRU);
	xdebug_gcstats_minit();
	xdebug_profiler_minit();

	/* Get reserved offsets */
	zend_xdebug_cc_run_offset = zend_get_resource_handle(&dummy_ext);
	zend_xdebug_filter_offset = zend_get_resource_handle(&dummy_ext);

	/* Overload the "exit" opcode */
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(exit, ZEND_EXIT);

	/* Overload opcodes for code coverage */
	if (XINI_COV(code_coverage_enable)) {
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_JMP);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_JMPZ);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_JMPZ_EX);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_JMPNZ);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_IDENTICAL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_NOT_IDENTICAL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_EQUAL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_NOT_EQUAL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_SMALLER);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_SMALLER_OR_EQUAL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BOOL_NOT);

		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ADD);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SUB);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_MUL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_DIV);

		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ADD_ARRAY_ELEMENT);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_RETURN);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_RETURN_BY_REF);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_EXT_STMT);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAR);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAR_NO_REF);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_REF);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAL_EX);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAR_EX);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_NEW);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_EXT_FCALL_BEGIN);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INIT_METHOD_CALL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INIT_FCALL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_CATCH);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BOOL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INIT_ARRAY);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_R);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_W);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_R);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_W);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_FUNC_ARG);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_FUNC_ARG);
#if PHP_VERSION_ID >= 70100
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_STATIC_PROP_FUNC_ARG);
#endif
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_UNSET);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_UNSET);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_CLASS);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_CONSTANT);
#if PHP_VERSION_ID >= 70100
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_CLASS_CONSTANT);
#endif
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_CONCAT);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ISSET_ISEMPTY_DIM_OBJ);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ISSET_ISEMPTY_PROP_OBJ);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_PRE_INC_OBJ);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_CASE);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_QM_ASSIGN);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_DECLARE_LAMBDA_FUNCTION);
#if PHP_VERSION_ID < 70400
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ADD_TRAIT);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BIND_TRAITS);
#endif
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INSTANCEOF);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FAST_RET);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ROPE_ADD);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ROPE_END);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_COALESCE);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_TYPE_CHECK);
#if PHP_VERSION_ID >= 70100
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_GENERATOR_CREATE);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BIND_STATIC);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BIND_LEXICAL);
#endif
#if PHP_VERSION_ID >= 70400
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_DECLARE_CLASS);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_DECLARE_CLASS_DELAYED);
#endif
#if PHP_VERSION_ID >= 70200
		zend_set_user_opcode_handler(ZEND_SWITCH_STRING, xdebug_switch_handler);
		zend_set_user_opcode_handler(ZEND_SWITCH_LONG, xdebug_switch_handler);
#endif
	}

	/* Override opcodes for variable assignments in traces */
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(include_or_eval, ZEND_INCLUDE_OR_EVAL);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign, ZEND_ASSIGN);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(qm_assign, ZEND_QM_ASSIGN);
#if PHP_VERSION_ID >= 70400
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_op, ZEND_ASSIGN_OP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_dim_op, ZEND_ASSIGN_DIM_OP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_obj_op, ZEND_ASSIGN_OBJ_OP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_static_prop_op, ZEND_ASSIGN_STATIC_PROP_OP);
#else
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_add, ZEND_ASSIGN_ADD);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_sub, ZEND_ASSIGN_SUB);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_mul, ZEND_ASSIGN_MUL);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_div, ZEND_ASSIGN_DIV);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_mod, ZEND_ASSIGN_MOD);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_pow, ZEND_ASSIGN_POW);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_sl, ZEND_ASSIGN_SL);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_sr, ZEND_ASSIGN_SR);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_concat, ZEND_ASSIGN_CONCAT);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_bw_or, ZEND_ASSIGN_BW_OR);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_bw_and, ZEND_ASSIGN_BW_AND);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_bw_xor, ZEND_ASSIGN_BW_XOR);
#endif
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_dim, ZEND_ASSIGN_DIM);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_obj, ZEND_ASSIGN_OBJ);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_ref, ZEND_ASSIGN_REF);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_inc, ZEND_PRE_INC);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_inc, ZEND_POST_INC);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_dec, ZEND_PRE_DEC);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_dec, ZEND_POST_DEC);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_inc_obj, ZEND_PRE_INC_OBJ);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_inc_obj, ZEND_POST_INC_OBJ);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_dec_obj, ZEND_PRE_DEC_OBJ);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_dec_obj, ZEND_POST_DEC_OBJ);
#if PHP_VERSION_ID >= 70400
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_obj_ref, ZEND_ASSIGN_OBJ_REF);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_static_prop, ZEND_ASSIGN_STATIC_PROP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_static_prop_ref, ZEND_ASSIGN_STATIC_PROP_REF);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_inc_static_prop, ZEND_PRE_INC_STATIC_PROP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_dec_static_prop, ZEND_PRE_DEC_STATIC_PROP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_inc_static_prop, ZEND_POST_INC_STATIC_PROP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_dec_static_prop, ZEND_POST_DEC_STATIC_PROP);
#endif

	/* Override all the other opcodes so that we can mark when we hit a branch
	 * start one */
	if (XINI_COV(code_coverage_enable)) {
		int i;

		for (i = 0; i < 256; i++) {
			if (zend_get_user_opcode_handler(i) == NULL) {
				if (i == ZEND_HANDLE_EXCEPTION) {
					continue;
				}
				zend_set_user_opcode_handler(i, xdebug_check_branch_entry_handler);
			}
		}
	}

	if (zend_xdebug_initialised == 0) {
		zend_error(E_WARNING, "Xdebug MUST be loaded as a Zend extension");
	}

	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_APPEND", XDEBUG_TRACE_OPTION_APPEND, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_COMPUTERIZED", XDEBUG_TRACE_OPTION_COMPUTERIZED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_HTML", XDEBUG_TRACE_OPTION_HTML, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_NAKED_FILENAME", XDEBUG_TRACE_OPTION_NAKED_FILENAME, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("XDEBUG_CC_UNUSED", XDEBUG_CC_OPTION_UNUSED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_CC_DEAD_CODE", XDEBUG_CC_OPTION_DEAD_CODE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_CC_BRANCH_CHECK", XDEBUG_CC_OPTION_BRANCH_CHECK, CONST_CS | CONST_PERSISTENT);

	xdebug_filter_register_constants(INIT_FUNC_ARGS_PASSTHRU);

	XG_DBG(breakpoint_count) = 0;

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(xdebug)
{
	if (XINI_PROF(profiler_aggregate)) {
		xdebug_profiler_output_aggr_data(NULL TSRMLS_CC);
	}

	xdebug_gcstats_mshutdown();

	zend_hash_destroy(&XG_PROF(aggr_calls));

#ifdef ZTS
	ts_free_id(xdebug_globals_id);
#else
	php_xdebug_shutdown_globals(&xdebug_globals TSRMLS_CC);
#endif

	{
		int i = 0;
		zend_set_user_opcode_handler(ZEND_EXIT, NULL);

#ifndef ZTS
		/* Overload opcodes for code coverage */
		if (XINI_COV(code_coverage_enable)) {
#endif
			zend_set_user_opcode_handler(ZEND_JMP, NULL);
			zend_set_user_opcode_handler(ZEND_JMPZ, NULL);
			zend_set_user_opcode_handler(ZEND_JMPZ_EX, NULL);
			zend_set_user_opcode_handler(ZEND_JMPNZ, NULL);
			zend_set_user_opcode_handler(ZEND_IS_IDENTICAL, NULL);
			zend_set_user_opcode_handler(ZEND_IS_NOT_IDENTICAL, NULL);
			zend_set_user_opcode_handler(ZEND_IS_EQUAL, NULL);
			zend_set_user_opcode_handler(ZEND_IS_NOT_EQUAL, NULL);
			zend_set_user_opcode_handler(ZEND_IS_SMALLER, NULL);
			zend_set_user_opcode_handler(ZEND_IS_SMALLER_OR_EQUAL, NULL);
			zend_set_user_opcode_handler(ZEND_BOOL_NOT, NULL);

			zend_set_user_opcode_handler(ZEND_ADD, NULL);
			zend_set_user_opcode_handler(ZEND_SUB, NULL);
			zend_set_user_opcode_handler(ZEND_MUL, NULL);
			zend_set_user_opcode_handler(ZEND_DIV, NULL);

			zend_set_user_opcode_handler(ZEND_ADD_ARRAY_ELEMENT, NULL);
			zend_set_user_opcode_handler(ZEND_RETURN, NULL);
			zend_set_user_opcode_handler(ZEND_RETURN_BY_REF, NULL);
			zend_set_user_opcode_handler(ZEND_EXT_STMT, NULL);
			zend_set_user_opcode_handler(ZEND_SEND_VAR, NULL);
			zend_set_user_opcode_handler(ZEND_SEND_VAR_NO_REF, NULL);
			zend_set_user_opcode_handler(ZEND_SEND_REF, NULL);
			zend_set_user_opcode_handler(ZEND_SEND_VAL, NULL);
			zend_set_user_opcode_handler(ZEND_SEND_VAL_EX, NULL);
			zend_set_user_opcode_handler(ZEND_SEND_VAR_EX, NULL);
			zend_set_user_opcode_handler(ZEND_NEW, NULL);
			zend_set_user_opcode_handler(ZEND_EXT_FCALL_BEGIN, NULL);
			zend_set_user_opcode_handler(ZEND_INIT_METHOD_CALL, NULL);
			zend_set_user_opcode_handler(ZEND_INIT_FCALL, NULL);
			zend_set_user_opcode_handler(ZEND_CATCH, NULL);
			zend_set_user_opcode_handler(ZEND_BOOL, NULL);
			zend_set_user_opcode_handler(ZEND_INIT_ARRAY, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_DIM_R, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_OBJ_R, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_OBJ_W, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_OBJ_FUNC_ARG, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_DIM_FUNC_ARG, NULL);
#if PHP_VERSION_ID >= 70100
			zend_set_user_opcode_handler(ZEND_FETCH_STATIC_PROP_FUNC_ARG, NULL);
#endif
			zend_set_user_opcode_handler(ZEND_FETCH_DIM_UNSET, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_OBJ_UNSET, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_CLASS, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_CONSTANT, NULL);
#if PHP_VERSION_ID >= 70100
			zend_set_user_opcode_handler(ZEND_FETCH_CLASS_CONSTANT, NULL);
#endif
			zend_set_user_opcode_handler(ZEND_CONCAT, NULL);
			zend_set_user_opcode_handler(ZEND_ISSET_ISEMPTY_DIM_OBJ, NULL);
			zend_set_user_opcode_handler(ZEND_ISSET_ISEMPTY_PROP_OBJ, NULL);
			zend_set_user_opcode_handler(ZEND_PRE_INC_OBJ, NULL);
			zend_set_user_opcode_handler(ZEND_CASE, NULL);
			zend_set_user_opcode_handler(ZEND_QM_ASSIGN, NULL);
			zend_set_user_opcode_handler(ZEND_DECLARE_LAMBDA_FUNCTION, NULL);
#if PHP_VERSION_ID < 70400
			zend_set_user_opcode_handler(ZEND_ADD_TRAIT, NULL);
			zend_set_user_opcode_handler(ZEND_BIND_TRAITS, NULL);
#endif
			zend_set_user_opcode_handler(ZEND_INSTANCEOF, NULL);
			zend_set_user_opcode_handler(ZEND_FAST_RET, NULL);
			zend_set_user_opcode_handler(ZEND_ROPE_ADD, NULL);
			zend_set_user_opcode_handler(ZEND_ROPE_END, NULL);
			zend_set_user_opcode_handler(ZEND_COALESCE, NULL);
			zend_set_user_opcode_handler(ZEND_TYPE_CHECK, NULL);
#if PHP_VERSION_ID >= 70100
			zend_set_user_opcode_handler(ZEND_GENERATOR_CREATE, NULL);
			zend_set_user_opcode_handler(ZEND_BIND_STATIC, NULL);
			zend_set_user_opcode_handler(ZEND_BIND_LEXICAL, NULL);
#endif
#if PHP_VERSION_ID >= 70400
			zend_set_user_opcode_handler(ZEND_DECLARE_CLASS, NULL);
			zend_set_user_opcode_handler(ZEND_DECLARE_CLASS_DELAYED, NULL);
#endif
#ifndef ZTS
		}
#endif

		/* Override opcodes for variable assignments in traces */
		zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN, NULL);
#if PHP_VERSION_ID >= 70400
		zend_set_user_opcode_handler(ZEND_ASSIGN_OP, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_DIM_OP, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_OBJ_OP, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_STATIC_PROP_OP, NULL);
#else
		zend_set_user_opcode_handler(ZEND_ASSIGN_ADD, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_SUB, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_MUL, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_DIV, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_MOD, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_SL, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_SR, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_CONCAT, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_BW_OR, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_BW_AND, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_BW_XOR, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_POW, NULL);
#endif
		zend_set_user_opcode_handler(ZEND_ASSIGN_DIM, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_OBJ, NULL);
		zend_set_user_opcode_handler(ZEND_PRE_INC, NULL);
		zend_set_user_opcode_handler(ZEND_POST_INC, NULL);
		zend_set_user_opcode_handler(ZEND_PRE_DEC, NULL);
		zend_set_user_opcode_handler(ZEND_POST_DEC, NULL);
		zend_set_user_opcode_handler(ZEND_PRE_INC_OBJ, NULL);
		zend_set_user_opcode_handler(ZEND_POST_INC_OBJ, NULL);
		zend_set_user_opcode_handler(ZEND_PRE_DEC_OBJ, NULL);
		zend_set_user_opcode_handler(ZEND_POST_DEC_OBJ, NULL);

		zend_set_user_opcode_handler(ZEND_BEGIN_SILENCE, NULL);
		zend_set_user_opcode_handler(ZEND_END_SILENCE, NULL);

		/* cleanup handlers set in MINIT to xdebug_check_branch_entry_handler */
		for (i = 0; i < 256; i++) {
			if (zend_get_user_opcode_handler(i) == xdebug_check_branch_entry_handler) {
				zend_set_user_opcode_handler(i, NULL);
			}
		}
	}
	return SUCCESS;
}

size_t xdebug_ub_write(const char *string, size_t length TSRMLS_DC)
{
	if (xdebug_is_debug_connection_active_for_current_pid()) {
		if (-1 == XG_DBG(context).handler->remote_stream_output(string, length TSRMLS_CC)) {
			return 0;
		}
	}
	return xdebug_orig_ub_write(string, length TSRMLS_CC);
}

static void xdebug_init_auto_globals(TSRMLS_D)
{
	zend_is_auto_global_str((char*) ZEND_STRL("_ENV") TSRMLS_CC);
	zend_is_auto_global_str((char*) ZEND_STRL("_GET") TSRMLS_CC);
	zend_is_auto_global_str((char*) ZEND_STRL("_POST") TSRMLS_CC);
	zend_is_auto_global_str((char*) ZEND_STRL("_COOKIE") TSRMLS_CC);
	zend_is_auto_global_str((char*) ZEND_STRL("_REQUEST") TSRMLS_CC);
	zend_is_auto_global_str((char*) ZEND_STRL("_FILES") TSRMLS_CC);
	zend_is_auto_global_str((char*) ZEND_STRL("_SERVER") TSRMLS_CC);
	zend_is_auto_global_str((char*) ZEND_STRL("_SESSION") TSRMLS_CC);
}


PHP_RINIT_FUNCTION(xdebug)
{
	char *idekey;

#if defined(ZTS) && defined(COMPILE_DL_XDEBUG)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

/* PHP Bug #77287 causes Xdebug to segfault if OPcache has the "compact
 * literals" optimisation turned on. So force the optimisation off for PHP
 * 7.3.0 and 7.3.1.
 *
 * Otherwise, only turn off optimisation when we're debugging. */
#if PHP_VERSION_ID >= 70300 && PHP_VERSION_ID <= 70301
	{
#else
	if (XINI_DBG(remote_enable)) {
#endif
		zend_string *key = zend_string_init(ZEND_STRL("opcache.optimization_level"), 1);
		zend_string *value = zend_string_init(ZEND_STRL("0"), 1);

		zend_alter_ini_entry(key, value, ZEND_INI_SYSTEM, ZEND_INI_STAGE_STARTUP);

		zend_string_release(key);
		zend_string_release(value);
	}

	/* Get the ide key for this session */
	XG_DBG(ide_key) = NULL;
	idekey = xdebug_env_key(TSRMLS_C);
	if (idekey && *idekey) {
		if (XG_DBG(ide_key)) {
			xdfree(XG_DBG(ide_key));
		}
		XG_DBG(ide_key) = xdstrdup(idekey);
	}

	/* Get xdebug ini entries from the environment also,
	   this can override the idekey if one is set */
	xdebug_env_config(TSRMLS_C);

	XG_DBG(no_exec)        = 0;
	XG_COV(code_coverage_active) = 0;
	XG_COV(code_coverage_info) = xdebug_hash_alloc(32, xdebug_coverage_file_dtor);
	XG_TRACE(trace_handler) = NULL;
	XG_TRACE(trace_context) = NULL;
	XG_PROF(profile_file)  = NULL;
	XG_PROF(profile_filename) = NULL;
	XG_PROF(profile_filename_refs) = NULL;
	XG_PROF(profile_functionname_refs) = NULL;
	XG_PROF(profile_last_filename_ref) = 0;
	XG_PROF(profile_last_functionname_ref) = 0;
	XG_DBG(active_symbol_table) = NULL;
	XG_DBG(This) = NULL;
	XG_COV(dead_code_analysis_tracker_offset) = zend_xdebug_cc_run_offset;
	XG_COV(dead_code_last_start_id) = 1;
	XG_COV(code_coverage_filter_offset) = zend_xdebug_filter_offset;
	XG_COV(previous_filename) = NULL;
	XG_COV(previous_file) = NULL;

	xdebug_gcstats_rinit();

	xdebug_init_auto_globals(TSRMLS_C);

	/* Check if we have this special get variable that stops a debugging
	 * request without executing any code */
	{
		zend_string *stop_no_exec = zend_string_init(ZEND_STRL("XDEBUG_SESSION_STOP_NO_EXEC"), 0);
		if (
			(
				(
					zend_hash_find(Z_ARR(PG(http_globals)[TRACK_VARS_GET]), stop_no_exec) != NULL
				) || (
					zend_hash_find(Z_ARR(PG(http_globals)[TRACK_VARS_POST]), stop_no_exec) != NULL
				)
			)
			&& !SG(headers_sent)
		) {
			xdebug_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), (char*) "", 0, time(NULL) + XINI_DBG(remote_cookie_expire_time), "/", 1, NULL, 0, 0, 1, 0 TSRMLS_CC);
			XG_DBG(no_exec) = 1;
		}
		zend_string_release(stop_no_exec);
	}

	/* Only enabled extended info when it is not disabled */
	CG(compiler_options) = CG(compiler_options) | ZEND_COMPILE_EXTENDED_STMT;

	xdebug_base_rinit();

	xdebug_mark_debug_connection_not_active();
	XG_DBG(breakpoints_allowed) = 1;
	XG_DBG(remote_log_file) = NULL;
	XG_PROF(profiler_enabled) = 0;

	/* Initialize some debugger context properties */
	XG_DBG(context).program_name   = NULL;
	XG_DBG(context).list.last_file = NULL;
	XG_DBG(context).list.last_line = 0;
	XG_DBG(context).do_break       = 0;
	XG_DBG(context).do_step        = 0;
	XG_DBG(context).do_next        = 0;
	XG_DBG(context).do_finish      = 0;

	/* Initialize visited classes and branches hash */
	XG_COV(visited_classes) = xdebug_hash_alloc(2048, NULL);
	XG_COV(visited_branches) = xdebug_hash_alloc(2048, NULL);

	XG_COV(paths_stack) = xdebug_path_info_ctor();
	XG_COV(branches).size = 0;
	XG_COV(branches).last_branch_nr = NULL;

	return SUCCESS;
}

ZEND_MODULE_POST_ZEND_DEACTIVATE_D(xdebug)
{
	TSRMLS_FETCH();

	if (XG_DBG(remote_connection_enabled)) {
		XG_DBG(context).handler->remote_deinit(&(XG_DBG(context)));
		xdebug_close_socket(XG_DBG(context).socket);
	}
	if (XG_DBG(context).program_name) {
		xdfree(XG_DBG(context).program_name);
	}

	/* profiler */
	if (XG_PROF(profiler_enabled)) {
		xdebug_profiler_deinit();
	}

	if (XG_TRACE(trace_context)) {
		xdebug_stop_trace(TSRMLS_C);
	}

	xdebug_gcstats_post_deactivate();

	if (XG_DBG(ide_key)) {
		xdfree(XG_DBG(ide_key));
		XG_DBG(ide_key) = NULL;
	}

	XG_TRACE(trace_context)    = NULL;
	XG_COV(code_coverage_active) = 0;

	xdebug_hash_destroy(XG_COV(code_coverage_info));
	XG_COV(code_coverage_info) = NULL;

	xdebug_hash_destroy(XG_COV(visited_classes));
	XG_COV(visited_classes) = NULL;
	xdebug_hash_destroy(XG_COV(visited_branches));
	XG_COV(visited_branches) = NULL;

	if (XG_DBG(context.list.last_file)) {
		xdfree(XG_DBG(context).list.last_file);
		XG_DBG(context).list.last_file = NULL;
	}

	/* Clean up path coverage array */
	if (XG_COV(paths_stack)) {
		xdebug_path_info_dtor(XG_COV(paths_stack));
		XG_COV(paths_stack) = NULL;
	}
	if (XG_COV(branches).last_branch_nr) {
		free(XG_COV(branches).last_branch_nr);
		XG_COV(branches).last_branch_nr = NULL;
		XG_COV(branches).size = 0;
	}
	XG_COV(previous_mark_filename) = NULL;

	xdebug_base_post_deactivate();

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(xdebug)
{
	xdebug_base_rshutdown();

	return SUCCESS;
}

static int xdebug_info_printf(const char *fmt, ...) /* {{{ */
{
	char *buf;
	size_t len, written;
	va_list argv;

	va_start(argv, fmt);
	len = vspprintf(&buf, 0, fmt, argv);
	va_end(argv);

	written = php_output_write(buf, len);
	efree(buf);
	return written;
}
/* }}} */

PHP_MINFO_FUNCTION(xdebug)
{
	xdebug_remote_handler_info *ptr = xdebug_handlers_get();

	php_info_print_table_start();
	php_info_print_table_header(2, "xdebug support", "enabled");
	php_info_print_table_row(2, "Version", XDEBUG_VERSION);
	php_info_print_table_row(2, "IDE Key", XG_DBG(ide_key));
	php_info_print_table_end();

	php_info_print_table_start();
	if (!sapi_module.phpinfo_as_text) {
		php_info_print_table_header(1, "Support Xdebug on Patreon");
		xdebug_info_printf("<tr><td style='background-color: orangered; text-align: center'>%s</td></tr>\n", "<a style='font-size: large; color: white; background-color: transparent; font-weight: bold; text-decoration: underline' href='https://www.patreon.com/bePatron?u=7864328'>BECOME A PATRON</a>");
	} else {
		xdebug_info_printf("Support Xdebug on Patreon: https://www.patreon.com/bePatron?u=7864328\n");
	}
	php_info_print_table_end();

	if (zend_xdebug_initialised == 0) {
		php_info_print_table_start();
		php_info_print_table_header(1, "XDEBUG NOT LOADED AS ZEND EXTENSION");
		php_info_print_table_end();
	}

	php_info_print_table_start();
	php_info_print_table_header(1, "Supported protocols");
	while (ptr->name) {
		php_info_print_table_row(1, ptr->description);
		ptr++;
	}
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

static void xdebug_header_remove_with_prefix(xdebug_llist *headers, char *prefix, size_t prefix_len TSRMLS_DC)
{
	xdebug_llist_element *le;
	char                 *header;

	for (le = XDEBUG_LLIST_HEAD(XG_BASE(headers)); le != NULL; /* intentionally left blank*/) {
		header = XDEBUG_LLIST_VALP(le);

		if ((strlen(header) > prefix_len + 1) && (header[prefix_len] == ':') && (strncasecmp(header, prefix, prefix_len) == 0)) {
			xdebug_llist_element *current = le;

			le = XDEBUG_LLIST_NEXT(le);
			xdebug_llist_remove(headers, current, NULL);
		} else {
			le = XDEBUG_LLIST_NEXT(le);
		}
	}
}

static int xdebug_header_handler(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s TSRMLS_DC)
{
	if (XG_BASE(headers)) {
		switch (op) {
			case SAPI_HEADER_ADD:
				xdebug_llist_insert_next(XG_BASE(headers), XDEBUG_LLIST_TAIL(XG_BASE(headers)), xdstrdup(h->header));
				break;
			case SAPI_HEADER_REPLACE: {
				char *colon_offset = strchr(h->header, ':');

				if (colon_offset) {
					char save = *colon_offset;

					*colon_offset = '\0';
					xdebug_header_remove_with_prefix(XG_BASE(headers), h->header, strlen(h->header) TSRMLS_CC);
					*colon_offset = save;
				}

				xdebug_llist_insert_next(XG_BASE(headers), XDEBUG_LLIST_TAIL(XG_BASE(headers)), xdstrdup(h->header));
			} break;
			case SAPI_HEADER_DELETE_ALL:
				xdebug_llist_empty(XG_BASE(headers), NULL);
			case SAPI_HEADER_DELETE:
			case SAPI_HEADER_SET_STATUS:
				break;
		}
	}
	if (xdebug_orig_header_handler) {
		return xdebug_orig_header_handler(h, op, s TSRMLS_CC);
	}
	return SAPI_HEADER_ADD;
}


/* {{{ proto void xdebug_set_time_limit(void)
   Dummy function to prevent time limit from being set within the script */
PHP_FUNCTION(xdebug_set_time_limit)
{
	if (!xdebug_is_debug_connection_active_for_current_pid()) {
		XG_BASE(orig_set_time_limit_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	}
}
/* }}} */


/* {{{ proto int xdebug_error_reporting(void)
   Dummy function to return original error reporting level when 'eval' has turned it into 0 */
PHP_FUNCTION(xdebug_error_reporting)
{
	if (ZEND_NUM_ARGS() == 0 && XG_BASE(error_reporting_overridden) && xdebug_is_debug_connection_active_for_current_pid()) {
		RETURN_LONG(XG_BASE(error_reporting_override));
	}
	XG_BASE(orig_error_reporting_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void xdebug_pcntl_exec(void)
   Dummy function to prevent time limit from being set within the script */
PHP_FUNCTION(xdebug_pcntl_exec)
{
	/* We need to stop the profiler and trace files here */
	if (XG_PROF(profiler_enabled)) {
		xdebug_profiler_deinit(TSRMLS_C);
	}

	XG_BASE(orig_pcntl_exec_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void xdebug_var_dump(mixed var [, ...] )
   Outputs a fancy string representation of a variable */
PHP_FUNCTION(xdebug_var_dump)
{
	zval       *args;
	int         argc;
	int         i;
	xdebug_str *val;

	/* Ignore our new shiny function if overload_var_dump is set to 0 *and* the
	 * function is not being called as xdebug_var_dump() (usually, that'd be
	 * the overloaded var_dump() of course). Fixes issue 1262. */
	if (
		!XINI_BASE(overload_var_dump)
		&& (strcmp("xdebug_var_dump", execute_data->func->common.function_name->val) != 0)
	) {
		XG_BASE(orig_var_dump_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		return;
	}

	argc = ZEND_NUM_ARGS();

	args = safe_emalloc(argc, sizeof(zval), 0);
	if (ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
		efree(args);
		WRONG_PARAM_COUNT;
	}

	for (i = 0; i < argc; i++) {
		if (XINI_BASE(default_enable) == 0) {
			xdebug_php_var_dump(&args[i], 1 TSRMLS_CC);
		}
		else if (PG(html_errors)) {
			val = xdebug_get_zval_value_fancy(NULL, (zval*) &args[i], 0, NULL);
			PHPWRITE(val->d, val->l);
			xdebug_str_free(val);
		}
		else if ((XINI_BASE(cli_color) == 1 && xdebug_is_output_tty(TSRMLS_C)) || (XINI_BASE(cli_color) == 2)) {
			val = xdebug_get_zval_value_ansi((zval*) &args[i], 0, NULL);
			PHPWRITE(val->d, val->l);
			xdebug_str_free(val);
		}
		else {
			val = xdebug_get_zval_value_text((zval*) &args[i], 0, NULL);
			PHPWRITE(val->d, val->l);
			xdebug_str_free(val);
		}
	}

	efree(args);
}
/* }}} */

/* {{{ proto void xdebug_debug_zval(mixed var [, ...] )
   Outputs a fancy string representation of a variable */
PHP_FUNCTION(xdebug_debug_zval)
{
	zval       *args;
	int         argc;
	int         i;
	xdebug_str *val;

	argc = ZEND_NUM_ARGS();

	args = safe_emalloc(argc, sizeof(zval), 0);
	if (ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
		efree(args);
		WRONG_PARAM_COUNT;
	}

#if PHP_VERSION_ID >= 70100
	if (!(ZEND_CALL_INFO(EG(current_execute_data)->prev_execute_data) & ZEND_CALL_HAS_SYMBOL_TABLE)) {
#else
	if (!EG(current_execute_data)->prev_execute_data->symbol_table) {
#endif
		zend_rebuild_symbol_table(TSRMLS_C);
	}

	for (i = 0; i < argc; i++) {
		if (Z_TYPE(args[i]) == IS_STRING) {
			zval debugzval;
			xdebug_str *tmp_name;

			XG_DBG(active_symbol_table) = EG(current_execute_data)->prev_execute_data->symbol_table;
			XG_DBG(active_execute_data) = EG(current_execute_data)->prev_execute_data;

			tmp_name = xdebug_str_create(Z_STRVAL(args[i]), Z_STRLEN(args[i]));
			xdebug_get_php_symbol(&debugzval, tmp_name);
			xdebug_str_free(tmp_name);

			/* Reduce refcount for dumping */
			Z_TRY_DELREF(debugzval);

			php_printf("%s: ", Z_STRVAL(args[i]));
			if (Z_TYPE(debugzval) != IS_UNDEF) {
				if (PG(html_errors)) {
					val = xdebug_get_zval_value_fancy(NULL, &debugzval, 1, NULL);
					PHPWRITE(val->d, val->l);
				}
				else if ((XINI_BASE(cli_color) == 1 && xdebug_is_output_tty(TSRMLS_C)) || (XINI_BASE(cli_color) == 2)) {
					val = xdebug_get_zval_value_ansi(&debugzval, 1, NULL);
					PHPWRITE(val->d, val->l);
				}
				else {
					val = xdebug_get_zval_value(&debugzval, 1, NULL);
					PHPWRITE(val->d, val->l);
				}
				xdfree(val);
				PHPWRITE("\n", 1);
			} else {
				PHPWRITE("no such symbol\n", 15);
			}

			/* Restore original refcount */
			Z_TRY_ADDREF(debugzval);
			zval_ptr_dtor_nogc(&debugzval);
		}
	}

	efree(args);
}
/* }}} */

/* {{{ proto void xdebug_debug_zval_stdout(mixed var [, ...] )
   Outputs a fancy string representation of a variable */
PHP_FUNCTION(xdebug_debug_zval_stdout)
{
	zval   *args;
	int     argc;
	int     i;

	argc = ZEND_NUM_ARGS();

	args = safe_emalloc(argc, sizeof(zval), 0);
	if (ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
		efree(args);
		WRONG_PARAM_COUNT;
	}

#if PHP_VERSION_ID >= 70100
	if (!(ZEND_CALL_INFO(EG(current_execute_data)->prev_execute_data) & ZEND_CALL_HAS_SYMBOL_TABLE)) {
#else
	if (!EG(current_execute_data)->prev_execute_data->symbol_table) {
#endif
		zend_rebuild_symbol_table(TSRMLS_C);
	}

	for (i = 0; i < argc; i++) {
		if (Z_TYPE(args[i]) == IS_STRING) {
			zval        debugzval;
			xdebug_str *tmp_name;
			xdebug_str *val;

			XG_DBG(active_symbol_table) = EG(current_execute_data)->symbol_table;

			tmp_name = xdebug_str_create(Z_STRVAL(args[i]), Z_STRLEN(args[i]));
			xdebug_get_php_symbol(&debugzval, tmp_name);
			xdebug_str_free(tmp_name);

			/* Reduce refcount for dumping */
			Z_TRY_DELREF(debugzval);

			printf("%s: ", Z_STRVAL(args[i]));
			if (Z_TYPE(debugzval) != IS_UNDEF) {
				val = xdebug_get_zval_value(&debugzval, 1, NULL);
				printf("%s(%zd)", val->d, val->l);
				xdebug_str_free(val);
				printf("\n");
			} else {
				printf("no such symbol\n\n");
			}

			/* Restore original refcount */
			Z_TRY_ADDREF(debugzval);
			zval_ptr_dtor_nogc(&debugzval);
		}
	}

	efree(args);
}
/* }}} */

PHP_FUNCTION(xdebug_is_debugger_active)
{
	RETURN_BOOL(xdebug_is_debug_connection_active_for_current_pid());
}

PHP_FUNCTION(xdebug_break)
{
	/* Start JIT if requested and not yet enabled */
	xdebug_do_jit(TSRMLS_C);

	XG_DBG(context).do_break = 1;
	RETURN_TRUE;
}

PHP_FUNCTION(xdebug_start_error_collection)
{
	if (XG_BASE(do_collect_errors) == 1) {
		php_error(E_NOTICE, "Error collection was already started");
	}
	XG_BASE(do_collect_errors) = 1;
}

PHP_FUNCTION(xdebug_stop_error_collection)
{
	if (XG_BASE(do_collect_errors) == 0) {
		php_error(E_NOTICE, "Error collection was not started");
	}
	XG_BASE(do_collect_errors) = 0;
}

PHP_FUNCTION(xdebug_get_headers)
{
	xdebug_llist_element *le;
	char                 *string;

	array_init(return_value);
	for (le = XDEBUG_LLIST_HEAD(XG_BASE(headers)); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
		string = XDEBUG_LLIST_VALP(le);
		add_next_index_string(return_value, string);
	}
}

PHP_FUNCTION(xdebug_get_profiler_filename)
{
	if (XG_PROF(profile_filename)) {
		RETURN_STRING(XG_PROF(profile_filename));
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_dump_aggr_profiling_data)
{
	char *prefix = NULL;
	size_t prefix_len;

	if (!XINI_PROF(profiler_aggregate)) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &prefix, &prefix_len) == FAILURE) {
		return;
	}

	if (xdebug_profiler_output_aggr_data(prefix TSRMLS_CC) == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_clear_aggr_profiling_data)
{
	if (!XINI_PROF(profiler_aggregate)) {
		RETURN_FALSE;
	}

	zend_hash_clean(&XG_PROF(aggr_calls));

	RETURN_TRUE;
}

PHP_FUNCTION(xdebug_memory_usage)
{
	RETURN_LONG(zend_memory_usage(0 TSRMLS_CC));
}

PHP_FUNCTION(xdebug_peak_memory_usage)
{
	RETURN_LONG(zend_memory_peak_usage(0 TSRMLS_CC));
}

PHP_FUNCTION(xdebug_time_index)
{
	RETURN_DOUBLE(xdebug_get_utime() - XG_BASE(start_time));
}

#if PHP_VERSION_ID >= 70100
ZEND_DLEXPORT void xdebug_statement_call(zend_execute_data *frame)
{
	zend_op_array *op_array = &frame->func->op_array;
#else
ZEND_DLEXPORT void xdebug_statement_call(zend_op_array *op_array)
{
#endif
	xdebug_llist_element *le;
	xdebug_brk_info      *extra_brk_info;
	function_stack_entry *fse;
	int                   lineno;
	char                 *file;
	int                   file_len;
	int                   level = 0;
	int                   func_nr = 0;
	TSRMLS_FETCH();

	if (!EG(current_execute_data)) {
		return;
	}

	lineno = EG(current_execute_data)->opline->lineno;

	file = (char*) STR_NAME_VAL(op_array->filename);
	file_len = STR_NAME_LEN(op_array->filename);

	if (!op_array->reserved[XG_COV(code_coverage_filter_offset)] && XG_COV(code_coverage_active)) {
		xdebug_count_line(file, lineno, 0, 0 TSRMLS_CC);
	}

	if (xdebug_is_debug_connection_active_for_current_pid()) {

		if (XG_DBG(context).do_break) {
			XG_DBG(context).do_break = 0;

			if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), file, lineno, XDEBUG_BREAK, NULL, 0, NULL)) {
				xdebug_mark_debug_connection_not_active();
				return;
			}
		}

		/* Get latest stack level and function number */
		if (XG_BASE(stack) && XDEBUG_LLIST_TAIL(XG_BASE(stack))) {
			le = XDEBUG_LLIST_TAIL(XG_BASE(stack));
			fse = XDEBUG_LLIST_VALP(le);
			level = fse->level;
			func_nr = fse->function_nr;
		} else {
			level = 0;
			func_nr = 0;
		}

		/* Check for "finish" */
		if (
			XG_DBG(context).do_finish &&
			(
				(level < XG_DBG(context).finish_level) ||
				((level == XG_DBG(context).finish_level) && (func_nr > XG_DBG(context).finish_func_nr))
			)
		) {
			XG_DBG(context).do_finish = 0;

			if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), file, lineno, XDEBUG_STEP, NULL, 0, NULL)) {
				xdebug_mark_debug_connection_not_active();
				return;
			}
			return;
		}

		/* Check for "next" */
		if (XG_DBG(context).do_next && XG_DBG(context).next_level >= level) {
			XG_DBG(context).do_next = 0;

			if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), file, lineno, XDEBUG_STEP, NULL, 0, NULL)) {
				xdebug_mark_debug_connection_not_active();
				return;
			}
			return;
		}

		/* Check for "step" */
		if (XG_DBG(context).do_step) {
			XG_DBG(context).do_step = 0;

			if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), file, lineno, XDEBUG_STEP, NULL, 0, NULL)) {
				xdebug_mark_debug_connection_not_active();
				return;
			}
			return;
		}

		if (XG_DBG(context).line_breakpoints) {
			int   break_ok;
			zval  retval;

			for (le = XDEBUG_LLIST_HEAD(XG_DBG(context).line_breakpoints); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
				extra_brk_info = XDEBUG_LLIST_VALP(le);

				if (XG_DBG(context).handler->break_on_line(&(XG_DBG(context)), extra_brk_info, file, file_len, lineno)) {
					break_ok = 1; /* Breaking is allowed by default */

					/* Check if we have a condition set for it */
					if (extra_brk_info->condition) {
						/* If there is a condition, we disable breaking by
						 * default and only enabled it when the code evaluates
						 * to TRUE */
						break_ok = 0;

						/* Remember error reporting level */
						XG_BASE(error_reporting_override) = EG(error_reporting);
						XG_BASE(error_reporting_overridden) = 1;
						EG(error_reporting) = 0;
						XG_DBG(context).inhibit_notifications = 1;

						/* Check the condition */
						if (zend_eval_string(extra_brk_info->condition, &retval, (char*) "xdebug conditional breakpoint" TSRMLS_CC) == SUCCESS) {
							break_ok = Z_TYPE(retval) == IS_TRUE;
							zval_dtor(&retval);
						}

						/* Restore error reporting level */
						EG(error_reporting) = XG_BASE(error_reporting_override);
						XG_BASE(error_reporting_overridden) = 0;
						XG_DBG(context).inhibit_notifications = 0;
					}
					if (break_ok && xdebug_handle_hit_value(extra_brk_info)) {
						if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), file, lineno, XDEBUG_BREAK, NULL, 0, NULL)) {
							xdebug_mark_debug_connection_not_active();
							break;
						}
						return;
					}
				}
			}
		}
	}
}

static void xdebug_hook_output_handlers()
{
	/* Override header handler in SAPI */
	if (xdebug_orig_header_handler == NULL) {
		xdebug_orig_header_handler = sapi_module.header_handler;
		sapi_module.header_handler = xdebug_header_handler;
	}

	/* Override output handler for capturing output */
	if (xdebug_orig_ub_write == NULL) {
		xdebug_orig_ub_write = sapi_module.ub_write;
		sapi_module.ub_write = xdebug_ub_write;
	}
}

static void xdebug_unhook_output_handlers()
{
	/* Restore original header handler in SAPI */
	sapi_module.header_handler = xdebug_orig_header_handler;
	xdebug_orig_header_handler = NULL;

	/* Restore original output handler */
	sapi_module.ub_write = xdebug_orig_ub_write;
	xdebug_orig_ub_write = NULL;
}

ZEND_DLEXPORT int xdebug_zend_startup(zend_extension *extension)
{
	/* Hook output handlers (header and output writer) */
	xdebug_hook_output_handlers();

	zend_xdebug_initialised = 1;

#if PHP_VERSION_ID >= 70300
	xdebug_orig_post_startup_cb = zend_post_startup_cb;
	zend_post_startup_cb = xdebug_post_startup;

	return zend_startup_module(&xdebug_module_entry);
}

static int xdebug_post_startup(void)
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
#else
	return zend_startup_module(&xdebug_module_entry);
#endif
}

ZEND_DLEXPORT void xdebug_zend_shutdown(zend_extension *extension)
{
	/* Remove our hooks to output handlers (header and output writer) */
	xdebug_unhook_output_handlers();
}

ZEND_DLEXPORT void xdebug_init_oparray(zend_op_array *op_array)
{
	TSRMLS_FETCH();

	xdebug_filter_run_code_coverage(op_array);
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
