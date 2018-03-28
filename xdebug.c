/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2018 Derick Rethans                               |
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
#include "xdebug_compat.h"

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
#include "Zend/zend_closures.h"


#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_code_coverage.h"
#include "xdebug_com.h"
#include "xdebug_filter.h"
#include "xdebug_gc_stats.h"
#include "xdebug_llist.h"
#include "xdebug_mm.h"
#include "xdebug_monitor.h"
#include "xdebug_var.h"
#include "xdebug_profiler.h"
#include "xdebug_stack.h"
#include "xdebug_superglobals.h"
#include "xdebug_tracing.h"
#include "usefulstuff.h"

/* execution redirection functions */
zend_op_array* (*old_compile_file)(zend_file_handle* file_handle, int type TSRMLS_DC);
zend_op_array* xdebug_compile_file(zend_file_handle*, int TSRMLS_DC);

void (*xdebug_old_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
void xdebug_execute_ex(zend_execute_data *execute_data TSRMLS_DC);

void (*xdebug_old_execute_internal)(zend_execute_data *current_execute_data, zval *return_value);
void xdebug_execute_internal(zend_execute_data *current_execute_data, zval *return_value);

/* error callback replacement functions */
void (*xdebug_old_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);
void (*xdebug_new_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);
void xdebug_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);

static int xdebug_header_handler(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s TSRMLS_DC);
static size_t xdebug_ub_write(const char *string, size_t length TSRMLS_DC);

static void xdebug_throw_exception_hook(zval *exception TSRMLS_DC);
int xdebug_exit_handler(zend_execute_data *execute_data);

int zend_xdebug_initialised = 0;
int zend_xdebug_cc_run_offset = -1;
int zend_xdebug_filter_offset = -1;

static int (*xdebug_orig_header_handler)(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s TSRMLS_DC);
static size_t (*xdebug_orig_ub_write)(const char *string, size_t len TSRMLS_DC);

static int xdebug_trigger_enabled(int setting, const char *var_name, char *var_value TSRMLS_DC);

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
		XG(remote_mode) = XDEBUG_NONE;

	} else if (strcmp(STR_NAME_VAL(new_value), "jit") == 0) {
		XG(remote_mode) = XDEBUG_JIT;

	} else if (strcmp(STR_NAME_VAL(new_value), "req") == 0) {
		XG(remote_mode) = XDEBUG_REQ;

	} else {
		XG(remote_mode) = XDEBUG_NONE;
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
	STD_PHP_INI_BOOLEAN("xdebug.auto_trace",      "0",                  PHP_INI_ALL,    OnUpdateBool,   auto_trace,        zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.trace_enable_trigger", "0",             PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   trace_enable_trigger, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_enable_trigger_value", "",          PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString,   trace_enable_trigger_value, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_output_dir",  XDEBUG_TEMP_DIR,      PHP_INI_ALL,    OnUpdateString, trace_output_dir,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_output_name", "trace.%c",           PHP_INI_ALL,    OnUpdateString, trace_output_name, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_format",      "0",                  PHP_INI_ALL,    OnUpdateLong,   trace_format,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_options",     "0",                  PHP_INI_ALL,    OnUpdateLong,   trace_options,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.coverage_enable", "1",                  PHP_INI_SYSTEM, OnUpdateBool,   coverage_enable,   zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_includes","1",                  PHP_INI_ALL,    OnUpdateBool,   collect_includes,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.collect_params",  "0",                    PHP_INI_ALL,    OnUpdateLong,   collect_params,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_return",  "0",                  PHP_INI_ALL,    OnUpdateBool,   collect_return,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_vars",    "0",                  PHP_INI_ALL,    OnUpdateBool,   collect_vars,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_assignments", "0",              PHP_INI_ALL,    OnUpdateBool,   collect_assignments, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.default_enable",  "1",                  PHP_INI_ALL,    OnUpdateBool,   default_enable,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.extended_info",   "1",                  PHP_INI_SYSTEM, OnUpdateBool,   extended_info,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.file_link_format",  "",                   PHP_INI_ALL,    OnUpdateString, file_link_format,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.filename_format",   "",                   PHP_INI_ALL,    OnUpdateString, filename_format,   zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.force_display_errors", "0",             PHP_INI_SYSTEM, OnUpdateBool,   force_display_errors, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.force_error_reporting", "0",              PHP_INI_SYSTEM, OnUpdateLong,   force_error_reporting, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.halt_level",        "0",                  PHP_INI_ALL,    OnUpdateLong,   halt_level,        zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.max_nesting_level", "256",                PHP_INI_ALL,    OnUpdateLong,   max_nesting_level, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.max_stack_frames",  "-1",                 PHP_INI_ALL,    OnUpdateLong,   max_stack_frames,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.overload_var_dump", "2",                  PHP_INI_ALL,    OnUpdateLong,   overload_var_dump, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_error_trace",  "0",                PHP_INI_ALL,    OnUpdateBool,   show_error_trace,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_exception_trace",  "0",            PHP_INI_ALL,    OnUpdateBool,   show_ex_trace,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_local_vars", "0",                  PHP_INI_ALL,    OnUpdateBool,   show_local_vars,   zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_mem_delta",  "0",                  PHP_INI_ALL,    OnUpdateBool,   show_mem_delta,    zend_xdebug_globals, xdebug_globals)

	/* Dump superglobals settings */
	PHP_INI_ENTRY("xdebug.dump.COOKIE",           NULL,                 PHP_INI_ALL,    OnUpdateCookie)
	PHP_INI_ENTRY("xdebug.dump.ENV",              NULL,                 PHP_INI_ALL,    OnUpdateEnv)
	PHP_INI_ENTRY("xdebug.dump.FILES",            NULL,                 PHP_INI_ALL,    OnUpdateFiles)
	PHP_INI_ENTRY("xdebug.dump.GET",              NULL,                 PHP_INI_ALL,    OnUpdateGet)
	PHP_INI_ENTRY("xdebug.dump.POST",             NULL,                 PHP_INI_ALL,    OnUpdatePost)
	PHP_INI_ENTRY("xdebug.dump.REQUEST",          NULL,                 PHP_INI_ALL,    OnUpdateRequest)
	PHP_INI_ENTRY("xdebug.dump.SERVER",           NULL,                 PHP_INI_ALL,    OnUpdateServer)
	PHP_INI_ENTRY("xdebug.dump.SESSION",          NULL,                 PHP_INI_ALL,    OnUpdateSession)
	STD_PHP_INI_BOOLEAN("xdebug.dump_globals",    "1",                  PHP_INI_ALL,    OnUpdateBool,   dump_globals,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.dump_once",       "1",                  PHP_INI_ALL,    OnUpdateBool,   dump_once,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.dump_undefined",  "0",                  PHP_INI_ALL,    OnUpdateBool,   dump_undefined,    zend_xdebug_globals, xdebug_globals)

	/* Profiler settings */
	STD_PHP_INI_BOOLEAN("xdebug.profiler_enable",         "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler_enable,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.profiler_output_dir",       XDEBUG_TEMP_DIR,      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, profiler_output_dir,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.profiler_output_name",      "cachegrind.out.%p",  PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, profiler_output_name,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_enable_trigger", "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler_enable_trigger, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.profiler_enable_trigger_value", "",   PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString,   profiler_enable_trigger_value, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_append",         "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler_append,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_aggregate",      "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler_aggregate,      zend_xdebug_globals, xdebug_globals)

	/* Remote debugger settings */
	STD_PHP_INI_BOOLEAN("xdebug.remote_enable",   "0",   PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   remote_enable,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_handler",    "dbgp",               PHP_INI_ALL,    OnUpdateString, remote_handler,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_host",       "localhost",          PHP_INI_ALL,    OnUpdateString, remote_host,       zend_xdebug_globals, xdebug_globals)
	PHP_INI_ENTRY("xdebug.remote_mode",           "req",                PHP_INI_ALL,    OnUpdateDebugMode)
	STD_PHP_INI_ENTRY("xdebug.remote_port",       "9000",               PHP_INI_ALL,    OnUpdateLong,   remote_port,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.remote_autostart","0",                  PHP_INI_ALL,    OnUpdateBool,   remote_autostart,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.remote_connect_back","0",               PHP_INI_ALL,    OnUpdateBool,   remote_connect_back,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_log",        "",                   PHP_INI_ALL,    OnUpdateString, remote_log,        zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.idekey",            "",                   PHP_INI_ALL,    OnUpdateString, ide_key_setting,   zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_cookie_expire_time", "3600",       PHP_INI_ALL,    OnUpdateLong,   remote_cookie_expire_time, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_addr_header", "",                  PHP_INI_ALL,    OnUpdateString, remote_addr_header, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_timeout",    "200",                PHP_INI_ALL,    OnUpdateLong,   remote_connect_timeout, zend_xdebug_globals, xdebug_globals)

	/* Variable display settings */
	STD_PHP_INI_ENTRY("xdebug.var_display_max_children", "128",         PHP_INI_ALL,    OnUpdateLong,   display_max_children, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.var_display_max_data",     "512",         PHP_INI_ALL,    OnUpdateLong,   display_max_data,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.var_display_max_depth",    "3",           PHP_INI_ALL,    OnUpdateLong,   display_max_depth,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.cli_color",                "0",           PHP_INI_ALL,    OnUpdateBool,   cli_color,            zend_xdebug_globals, xdebug_globals)

	/* Scream support */
	STD_PHP_INI_BOOLEAN("xdebug.scream",                 "0",           PHP_INI_ALL,    OnUpdateBool,   do_scream,            zend_xdebug_globals, xdebug_globals)

	/* GC Stats support */
	STD_PHP_INI_BOOLEAN("xdebug.gc_stats_enable",    "0",               PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   gc_stats_enable,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.gc_stats_output_dir",  XDEBUG_TEMP_DIR,   PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, gc_stats_output_dir,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.gc_stats_output_name", "gcstats.%p",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, gc_stats_output_name, zend_xdebug_globals, xdebug_globals)
PHP_INI_END()

static void php_xdebug_init_globals (zend_xdebug_globals *xg TSRMLS_DC)
{
	xg->headers              = NULL;
	xg->stack                = NULL;
	xg->level                = 0;
	xg->do_trace             = 0;
	xg->trace_handler        = NULL;
	xg->trace_context        = NULL;
	xg->in_debug_info        = 0;
	xg->coverage_enable      = 0;
	xg->previous_filename    = NULL;
	xg->previous_file        = NULL;
	xg->previous_mark_filename = NULL;
	xg->previous_mark_file     = NULL;
	xg->paths_stack = NULL;
	xg->branches.size        = 0;
	xg->branches.last_branch_nr = NULL;
	xg->do_code_coverage     = 0;
	xg->breakpoint_count     = 0;
	xg->ide_key              = NULL;
	xg->output_is_tty        = OUTPUT_NOT_CHECKED;
	xg->stdout_mode          = 0;
	xg->in_at                = 0;
	xg->active_execute_data  = NULL;
	xg->no_exec              = 0;
	xg->context.program_name = NULL;
	xg->context.list.last_file = NULL;
	xg->context.list.last_line = 0;
	xg->context.do_break     = 0;
	xg->context.do_step      = 0;
	xg->context.do_next      = 0;
	xg->context.do_finish    = 0;
	xg->in_execution         = 0;
	xg->remote_enabled       = 0;
	xg->breakpoints_allowed  = 0;
	xg->profiler_enabled     = 0;
	xg->do_monitor_functions = 0;

	xg->filter_type_tracing       = XDEBUG_FILTER_NONE;
	xg->filter_type_profiler      = XDEBUG_FILTER_NONE;
	xg->filter_type_code_coverage = XDEBUG_FILTER_NONE;
	xg->filters_tracing           = NULL;
	xg->filters_code_coverage     = NULL;

	xg->gc_stats_file = NULL;
	xg->gc_stats_filename = NULL;
	xg->gc_stats_enabled = 0;

	xdebug_llist_init(&xg->server, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->get, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->post, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->cookie, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->files, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->env, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->request, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->session, xdebug_superglobals_dump_dtor);

	/* Get reserved offset */
	xg->dead_code_analysis_tracker_offset = zend_xdebug_cc_run_offset;
	xg->dead_code_last_start_id = 1;
	xg->code_coverage_filter_offset = zend_xdebug_filter_offset;

	/* Override header generation in SAPI */
	if (sapi_module.header_handler != xdebug_header_handler) {
		xdebug_orig_header_handler = sapi_module.header_handler;
		sapi_module.header_handler = xdebug_header_handler;
	}
	xg->headers = NULL;

	/* Capturing output */
	if (sapi_module.ub_write != xdebug_ub_write) {
		xdebug_orig_ub_write = sapi_module.ub_write;
		sapi_module.ub_write = xdebug_ub_write;
	}
}

static void php_xdebug_shutdown_globals (zend_xdebug_globals *xg TSRMLS_DC)
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

char *xdebug_env_key(TSRMLS_D)
{
	char *ide_key;

	ide_key = XG(ide_key_setting);
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
			if (XG(ide_key)) {
				xdfree(XG(ide_key));
			}
			XG(ide_key) = xdstrdup(envval);
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
	if (XG(do_code_coverage)) {
		execute_data->opline++;
		return ZEND_USER_OPCODE_CONTINUE;
	}
	return ZEND_USER_OPCODE_DISPATCH;
}
#endif

static int xdebug_silence_handler(zend_execute_data *execute_data)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	const zend_op *cur_opcode = EG(current_execute_data)->opline;

	if (!op_array->reserved[XG(code_coverage_filter_offset)] && XG(do_code_coverage)) {
		xdebug_print_opcode_info('S', execute_data, cur_opcode TSRMLS_CC);
	}
	if (XG(do_scream)) {
		execute_data->opline++;
		if (cur_opcode->opcode == ZEND_BEGIN_SILENCE) {
			XG(in_at) = 1;
		} else {
			XG(in_at) = 0;
		}
		return ZEND_USER_OPCODE_CONTINUE;
	}
	return ZEND_USER_OPCODE_DISPATCH;
}

static int xdebug_include_or_eval_handler(zend_execute_data *execute_data)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	const zend_op *opline = execute_data->opline;

	if (!op_array->reserved[XG(code_coverage_filter_offset)] && XG(do_code_coverage)) {
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
		if (XG(last_eval_statement)) {
			efree(XG(last_eval_statement));
		}
		XG(last_eval_statement) = estrndup(Z_STRVAL_P(inc_filename), Z_STRLEN_P(inc_filename));

		if (inc_filename == &tmp_inc_filename) {
			zval_dtor(&tmp_inc_filename);
		}
	}
	return ZEND_USER_OPCODE_DISPATCH;
}

int xdebug_is_output_tty(TSRMLS_D)
{
	if (XG(output_is_tty) == OUTPUT_NOT_CHECKED) {
#ifndef PHP_WIN32
		XG(output_is_tty) = isatty(STDOUT_FILENO);
#else
		XG(output_is_tty) = getenv("ANSICON");
#endif
	}
	return (XG(output_is_tty));
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

static int xdebug_closure_serialize_deny_wrapper(zval *object, unsigned char **buffer, size_t *buf_len, zend_serialize_data *data TSRMLS_DC)
{
	zend_class_entry *ce = Z_OBJCE_P(object);

	if (!XG(in_var_serialisation)) {
		zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Serialization of '%s' is not allowed", STR_NAME_VAL(ce->name));
	}
	return FAILURE;
}

PHP_MINIT_FUNCTION(xdebug)
{
	zend_extension dummy_ext;

	ZEND_INIT_MODULE_GLOBALS(xdebug, php_xdebug_init_globals, php_xdebug_shutdown_globals);
	REGISTER_INI_ENTRIES();

	/* initialize aggregate call information hash */
	zend_hash_init_ex(&XG(aggr_calls), 50, NULL, (dtor_func_t) xdebug_profile_aggr_call_entry_dtor, 1, 0);

	/* Redirect compile and execute functions to our own */
	old_compile_file = zend_compile_file;
	zend_compile_file = xdebug_compile_file;

	xdebug_old_execute_ex = zend_execute_ex;
	zend_execute_ex = xdebug_execute_ex;

	xdebug_old_execute_internal = zend_execute_internal;
	zend_execute_internal = xdebug_execute_internal;

	/* Replace error handler callback with our own */
	xdebug_old_error_cb = zend_error_cb;
	xdebug_new_error_cb = xdebug_error_cb;

    /* Replace garbage collection handler with our own */
    xdebug_old_gc_collect_cycles = gc_collect_cycles;
    gc_collect_cycles = xdebug_gc_collect_cycles;

	/* Get reserved offsets */
	zend_xdebug_cc_run_offset = zend_get_resource_handle(&dummy_ext);
	zend_xdebug_filter_offset = zend_get_resource_handle(&dummy_ext);

	/* Overload the "exit" opcode */
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(exit, ZEND_EXIT);

	/* Overload opcodes for code coverage */
	if (XG(coverage_enable)) {
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
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_CATCH);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BOOL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INIT_ARRAY);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_R);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_W);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_R);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_W);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_FUNC_ARG);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_FUNC_ARG);
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
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ADD_TRAIT);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BIND_TRAITS);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INSTANCEOF);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FAST_RET);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ROPE_ADD);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ROPE_END);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_COALESCE);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_TYPE_CHECK);
#if PHP_VERSION_ID >= 70100
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_GENERATOR_CREATE);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BIND_STATIC);
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

	zend_set_user_opcode_handler(ZEND_BEGIN_SILENCE, xdebug_silence_handler);
	zend_set_user_opcode_handler(ZEND_END_SILENCE, xdebug_silence_handler);

	/* Override all the other opcodes so that we can mark when we hit a branch
	 * start one */
	if (XG(coverage_enable)) {
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

	REGISTER_LONG_CONSTANT("XDEBUG_STACK_NO_DESC", XDEBUG_STACK_NO_DESC, CONST_CS | CONST_PERSISTENT);

	xdebug_filter_register_constants(INIT_FUNC_ARGS_PASSTHRU);

	XG(breakpoint_count) = 0;
	XG(output_is_tty) = OUTPUT_NOT_CHECKED;

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(xdebug)
{
	if (XG(profiler_aggregate)) {
		xdebug_profiler_output_aggr_data(NULL TSRMLS_CC);
	}

	/* Reset compile, execute and error callbacks */
	zend_compile_file = old_compile_file;
	zend_execute_ex = xdebug_old_execute_ex;
	zend_execute_internal = xdebug_old_execute_internal;
	zend_error_cb = xdebug_old_error_cb;
	gc_collect_cycles = xdebug_old_gc_collect_cycles;

	zend_hash_destroy(&XG(aggr_calls));

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
		if (XG(coverage_enable)) {
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
			zend_set_user_opcode_handler(ZEND_CATCH, NULL);
			zend_set_user_opcode_handler(ZEND_BOOL, NULL);
			zend_set_user_opcode_handler(ZEND_INIT_ARRAY, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_DIM_R, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_OBJ_R, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_OBJ_W, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_OBJ_FUNC_ARG, NULL);
			zend_set_user_opcode_handler(ZEND_FETCH_DIM_FUNC_ARG, NULL);
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
			zend_set_user_opcode_handler(ZEND_ADD_TRAIT, NULL);
			zend_set_user_opcode_handler(ZEND_BIND_TRAITS, NULL);
			zend_set_user_opcode_handler(ZEND_INSTANCEOF, NULL);
			zend_set_user_opcode_handler(ZEND_FAST_RET, NULL);
			zend_set_user_opcode_handler(ZEND_ROPE_ADD, NULL);
			zend_set_user_opcode_handler(ZEND_ROPE_END, NULL);
			zend_set_user_opcode_handler(ZEND_COALESCE, NULL);
			zend_set_user_opcode_handler(ZEND_TYPE_CHECK, NULL);
#if PHP_VERSION_ID >= 70100
			zend_set_user_opcode_handler(ZEND_GENERATOR_CREATE, NULL);
			zend_set_user_opcode_handler(ZEND_BIND_STATIC, NULL);
#endif
#ifndef ZTS
		}
#endif

		/* Override opcodes for variable assignments in traces */
		zend_set_user_opcode_handler(ZEND_INCLUDE_OR_EVAL, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_ADD, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_SUB, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_MUL, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_DIV, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_MOD, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_POW, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_SL, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_SR, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_CONCAT, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_BW_OR, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_BW_AND, NULL);
		zend_set_user_opcode_handler(ZEND_ASSIGN_BW_XOR, NULL);
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

static void xdebug_llist_string_dtor(void *dummy, void *elem)
{
	char *s = elem;

	if (s) {
		xdfree(s);
	}
}

static void xdebug_declared_var_dtor(void *dummy, void *elem)
{
	xdebug_str *s = (xdebug_str*) elem;

	xdebug_str_free(s);
}

static void function_stack_entry_dtor(void *dummy, void *elem)
{
	unsigned int          i;
	function_stack_entry *e = elem;

	e->refcount--;

	if (e->refcount == 0) {
		if (e->function.function) {
			xdfree(e->function.function);
		}
		if (e->function.class) {
			xdfree(e->function.class);
		}
		if (e->filename) {
			xdfree(e->filename);
		}

		if (e->var) {
			for (i = 0; i < e->varc; i++) {
				if (!Z_ISUNDEF(e->var[i].data)) {
					ZVAL_UNDEF(&e->var[i].data);
				}
				if (e->var[i].name) {
					xdfree(e->var[i].name);
				}
			}
			xdfree(e->var);
		}

		if (e->include_filename) {
			xdfree(e->include_filename);
		}

		if (e->declared_vars) {
			xdebug_llist_destroy(e->declared_vars, NULL);
			e->declared_vars = NULL;
		}

		if (e->profile.call_list) {
			xdebug_llist_destroy(e->profile.call_list, NULL);
			e->profile.call_list = NULL;
		}

		xdfree(e);
	}
}

size_t xdebug_ub_write(const char *string, size_t length TSRMLS_DC)
{
	if (XG(remote_enabled)) {
		if (-1 == XG(context).handler->remote_stream_output(string, length TSRMLS_CC)) {
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


static void xdebug_overloaded_functions_setup(TSRMLS_D)
{
	zend_function *orig;

	/* Override var_dump with our own function */
	orig = zend_hash_str_find_ptr(EG(function_table), "var_dump", sizeof("var_dump") - 1);
	XG(orig_var_dump_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_xdebug_var_dump;

	/* Override set_time_limit with our own function to prevent timing out while debugging */
	orig = zend_hash_str_find_ptr(EG(function_table), "set_time_limit", sizeof("set_time_limit") - 1);
	XG(orig_set_time_limit_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_xdebug_set_time_limit;

	/* Override pcntl_exec with our own function to be able to write profiling summary */
	orig = zend_hash_str_find_ptr(EG(function_table), "pcntl_exec", sizeof("pcntl_exec") - 1);
	if (orig) {
		XG(orig_pcntl_exec_func) = orig->internal_function.handler;
		orig->internal_function.handler = zif_xdebug_pcntl_exec;
	} else {
		XG(orig_pcntl_exec_func) = NULL;
	}
}

static void xdebug_overloaded_functions_restore(TSRMLS_D)
{
	zend_function *orig;

	orig = zend_hash_str_find_ptr(EG(function_table), "var_dump", sizeof("var_dump") - 1);
	orig->internal_function.handler = XG(orig_var_dump_func);

	orig = zend_hash_str_find_ptr(EG(function_table), "set_time_limit", sizeof("set_time_limit") - 1);
	orig->internal_function.handler = XG(orig_set_time_limit_func);;

	if (XG(orig_pcntl_exec_func)) {
		orig = zend_hash_str_find_ptr(EG(function_table), "pcntl_exec", sizeof("pcntl_exec") - 1);
		if (orig) {
			orig->internal_function.handler = XG(orig_pcntl_exec_func);
		}
	}
}


PHP_RINIT_FUNCTION(xdebug)
{
	char *idekey;

#if defined(ZTS) && defined(COMPILE_DL_XDEBUG)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	/* Get the ide key for this session */
	XG(ide_key) = NULL;
	idekey = xdebug_env_key(TSRMLS_C);
	if (idekey && *idekey) {
		if (XG(ide_key)) {
			xdfree(XG(ide_key));
		}
		XG(ide_key) = xdstrdup(idekey);
	}

	/* Get xdebug ini entries from the environment also,
	   this can override the idekey if one is set */
	xdebug_env_config(TSRMLS_C);

	XG(no_exec)       = 0;
	XG(level)         = 0;
	XG(do_trace)      = 0;
	XG(in_debug_info) = 0;
	XG(coverage_enable) = 0;
	XG(do_code_coverage) = 0;
	XG(code_coverage) = xdebug_hash_alloc(32, xdebug_coverage_file_dtor);
	XG(stack)         = xdebug_llist_alloc(function_stack_entry_dtor);
	XG(trace_handler) = NULL;
	XG(trace_context) = NULL;
	XG(profile_file)  = NULL;
	XG(profile_filename) = NULL;
	XG(profile_filename_refs) = xdebug_hash_alloc(128, NULL);
	XG(profile_functionname_refs) = xdebug_hash_alloc(128, NULL);
	XG(profile_last_filename_ref) = 0;
	XG(profile_last_functionname_ref) = 0;
	XG(prev_memory)   = 0;
	XG(function_count) = -1;
	XG(active_symbol_table) = NULL;
	XG(This) = NULL;
	XG(last_exception_trace) = NULL;
	XG(last_eval_statement) = NULL;
	XG(do_collect_errors) = 0;
	XG(collected_errors)  = xdebug_llist_alloc(xdebug_llist_string_dtor);
	XG(do_monitor_functions) = 0;
	XG(functions_to_monitor) = NULL;
	XG(monitored_functions_found) = xdebug_llist_alloc(xdebug_monitored_function_dtor);
	XG(dead_code_analysis_tracker_offset) = zend_xdebug_cc_run_offset;
	XG(dead_code_last_start_id) = 1;
	XG(code_coverage_filter_offset) = zend_xdebug_filter_offset;
	XG(previous_filename) = NULL;
	XG(previous_file) = NULL;
	XG(gc_stats_file) = NULL;
	XG(gc_stats_filename) = NULL;
	XG(gc_stats_enabled) = 0;

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
			xdebug_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), (char*) "", 0, time(NULL) + XG(remote_cookie_expire_time), "/", 1, NULL, 0, 0, 1, 0 TSRMLS_CC);
			XG(no_exec) = 1;
		}
		zend_string_release(stop_no_exec);
	}

	/* Only enabled extended info when it is not disabled */
	CG(compiler_options) = CG(compiler_options) | (XG(extended_info) ? ZEND_COMPILE_EXTENDED_INFO : 0);

	/* Hack: We check for a soap header here, if that's existing, we don't use
	 * Xdebug's error handler to keep soap fault from fucking up. */
	if (XG(default_enable) && zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_SOAPACTION", sizeof("HTTP_SOAPACTION") - 1) == NULL) {
		zend_error_cb = xdebug_new_error_cb;
		zend_throw_exception_hook = xdebug_throw_exception_hook;
	}

	XG(remote_enabled) = 0;
	XG(profiler_enabled) = 0;
	XG(breakpoints_allowed) = 1;

	/* Initialize some debugger context properties */
	XG(context).program_name   = NULL;
	XG(context).list.last_file = NULL;
	XG(context).list.last_line = 0;
	XG(context).do_break       = 0;
	XG(context).do_step        = 0;
	XG(context).do_next        = 0;
	XG(context).do_finish      = 0;

	/* Initialize dump superglobals */
	XG(dumped) = 0;

	/* Initialize visisted branches hash */
	XG(visited_branches) = xdebug_hash_alloc(2048, NULL);

	/* Initialize start time */
	XG(start_time) = xdebug_get_utime();

	/* Overload var_dump, set_time_limit, and pcntl_exec */
	xdebug_overloaded_functions_setup(TSRMLS_C);

	XG(headers) = xdebug_llist_alloc(xdebug_llist_string_dtor);

	XG(in_var_serialisation) = 0;
	zend_ce_closure->serialize = xdebug_closure_serialize_deny_wrapper;

	/* Signal that we're in a request now */
	XG(in_execution) = 1;

	XG(paths_stack) = xdebug_path_info_ctor();
	XG(branches).size = 0;
	XG(branches).last_branch_nr = NULL;

	/* filters */
	XG(filter_type_tracing)       = XDEBUG_FILTER_NONE;
	XG(filter_type_profiler)      = XDEBUG_FILTER_NONE;
	XG(filter_type_code_coverage) = XDEBUG_FILTER_NONE;
	XG(filters_tracing)           = xdebug_llist_alloc(xdebug_llist_string_dtor);
	XG(filters_code_coverage)     = xdebug_llist_alloc(xdebug_llist_string_dtor);

	return SUCCESS;
}

ZEND_MODULE_POST_ZEND_DEACTIVATE_D(xdebug)
{
	TSRMLS_FETCH();

	if (XG(remote_enabled)) {
		XG(context).handler->remote_deinit(&(XG(context)));
		xdebug_close_socket(XG(context).socket);
	}
	if (XG(context).program_name) {
		xdfree(XG(context).program_name);
	}

	xdebug_llist_destroy(XG(stack), NULL);
	XG(stack) = NULL;

	/* filters */
	xdebug_llist_destroy(XG(filters_tracing), NULL);
	xdebug_llist_destroy(XG(filters_code_coverage), NULL);
	XG(filters_tracing) = NULL;
	XG(filters_code_coverage) = NULL;

	if (XG(do_trace) && XG(trace_context)) {
		xdebug_stop_trace(TSRMLS_C);
	}

	if (XG(profile_file)) {
		fclose(XG(profile_file));
		XG(profile_file) = NULL;
	}

	if (XG(profile_filename)) {
		xdfree(XG(profile_filename));
	}

	XG(profiler_enabled) = 0;
	xdebug_hash_destroy(XG(profile_filename_refs));
	xdebug_hash_destroy(XG(profile_functionname_refs));
	XG(profile_filename_refs) = NULL;
	XG(profile_functionname_refs) = NULL;

	if (XG(gc_stats_enabled)) {
		xdebug_gc_stats_stop();
	}

	if (XG(gc_stats_filename)) {
		xdfree(XG(gc_stats_filename));
	}

	if (XG(ide_key)) {
		xdfree(XG(ide_key));
		XG(ide_key) = NULL;
	}

	XG(level)            = 0;
	XG(do_trace)         = 0;
	XG(in_debug_info)    = 0;
	XG(coverage_enable)  = 0;
	XG(do_code_coverage) = 0;

	xdebug_hash_destroy(XG(code_coverage));
	XG(code_coverage) = NULL;

	xdebug_hash_destroy(XG(visited_branches));
	XG(visited_branches) = NULL;

	if (XG(context.list.last_file)) {
		xdfree(XG(context).list.last_file);
		XG(context).list.last_file = NULL;
	}

	if (XG(last_exception_trace)) {
		xdfree(XG(last_exception_trace));
		XG(last_exception_trace) = NULL;
	}

	if (XG(last_eval_statement)) {
		efree(XG(last_eval_statement));
		XG(last_eval_statement) = NULL;
	}

	xdebug_llist_destroy(XG(collected_errors), NULL);
	XG(collected_errors) = NULL;

	xdebug_llist_destroy(XG(monitored_functions_found), NULL);
	XG(monitored_functions_found) = NULL;

	if (XG(functions_to_monitor)) {
		xdebug_hash_destroy(XG(functions_to_monitor));
		XG(functions_to_monitor) = NULL;
	}

	/* Restore original var_dump, set_time_limit, and pcntl_exec handlers */
	xdebug_overloaded_functions_restore(TSRMLS_C);

	/* Clean up collected headers */
	xdebug_llist_destroy(XG(headers), NULL);
	XG(headers) = NULL;

	/* Clean up path coverage array */
	if (XG(paths_stack)) {
		xdebug_path_info_dtor(XG(paths_stack));
		XG(paths_stack) = NULL;
	}
	if (XG(branches).last_branch_nr) {
		free(XG(branches).last_branch_nr);
		XG(branches).last_branch_nr = NULL;
		XG(branches).size = 0;
	}
	XG(previous_mark_filename) = NULL;

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(xdebug)
{
	/* Signal that we're no longer in a request */
	XG(in_execution) = 0;

	return SUCCESS;
}


PHP_MINFO_FUNCTION(xdebug)
{
	xdebug_remote_handler_info *ptr = xdebug_handlers_get();

	php_info_print_table_start();
	php_info_print_table_header(2, "xdebug support", "enabled");
	php_info_print_table_row(2, "Version", XDEBUG_VERSION);
	php_info_print_table_row(2, "IDE Key", XG(ide_key));
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

static int xdebug_trigger_enabled(int setting, const char *var_name, char *var_value TSRMLS_DC)
{
	zval *trigger_val;

	if (!setting) {
		return 0;
	}

	if (
		(
			(
				(trigger_val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_GET]), var_name, strlen(var_name))) != NULL
			) || (
				(trigger_val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_POST]), var_name, strlen(var_name))) != NULL
			) || (
				(trigger_val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_COOKIE]), var_name, strlen(var_name))) != NULL
			)
		) && (
			(var_value == NULL) || (var_value[0] == '\0') ||
			(strcmp(var_value, Z_STRVAL_P(trigger_val)) == 0)
		)
	) {
		return 1;
	}

	return 0;
}

static void add_used_variables(function_stack_entry *fse, zend_op_array *op_array)
{
	unsigned int i = 0;

	if (!fse->declared_vars) {
		fse->declared_vars = xdebug_llist_alloc(xdebug_declared_var_dtor);
	}

	/* Check parameters */
	for (i = 0; i < fse->varc; i++) {
		if (fse->var[i].name) {
			xdebug_llist_insert_next(fse->declared_vars, XDEBUG_LLIST_TAIL(fse->declared_vars), xdebug_str_create(fse->var[i].name, fse->var[i].length));
		}
	}

	/* gather used variables from compiled vars information */
	while (i < (unsigned int) op_array->last_var) {
		xdebug_llist_insert_next(fse->declared_vars, XDEBUG_LLIST_TAIL(fse->declared_vars), xdebug_str_create(STR_NAME_VAL(op_array->vars[i]), STR_NAME_LEN(op_array->vars[i])));
		i++;
	}

	/* opcode scanning time */
	while (i < op_array->last) {
		char *cv = NULL;
		int cv_len;

		if (op_array->opcodes[i].op1_type == IS_CV) {
			cv = (char *) xdebug_get_compiled_variable_name(op_array, op_array->opcodes[i].op1.var, &cv_len);
			xdebug_llist_insert_next(fse->declared_vars, XDEBUG_LLIST_TAIL(fse->declared_vars), xdebug_str_create(cv, cv_len));
		}
		if (op_array->opcodes[i].op2_type == IS_CV) {
			cv = (char *) xdebug_get_compiled_variable_name(op_array, op_array->opcodes[i].op2.var, &cv_len);
			xdebug_llist_insert_next(fse->declared_vars, XDEBUG_LLIST_TAIL(fse->declared_vars), xdebug_str_create(cv, cv_len));
		}
		i++;
	}
}

static void xdebug_throw_exception_hook(zval *exception TSRMLS_DC)
{
	zval *code, *message, *file, *line;
	zval *xdebug_message_trace, *previous_exception;
	zend_class_entry *default_ce, *exception_ce;
	xdebug_brk_info *extra_brk_info;
	char *code_str = NULL;
	char *exception_trace;
	xdebug_str tmp_str = XDEBUG_STR_INITIALIZER;

	if (!exception) {
		return;
	}

	default_ce = Z_OBJCE_P(exception);
	exception_ce = Z_OBJCE_P(exception);

	code =    xdebug_read_property(default_ce, exception, "code",    sizeof("code")-1,    0 TSRMLS_CC);
	message = xdebug_read_property(default_ce, exception, "message", sizeof("message")-1, 0 TSRMLS_CC);
	file =    xdebug_read_property(default_ce, exception, "file",    sizeof("file")-1,    0 TSRMLS_CC);
	line =    xdebug_read_property(default_ce, exception, "line",    sizeof("line")-1,    0 TSRMLS_CC);

	if (Z_TYPE_P(code) == IS_LONG) {
		if (Z_LVAL_P(code) != 0) {
			code_str = xdebug_sprintf("%lu", Z_LVAL_P(code));
		}
	} else if (Z_TYPE_P(code) != IS_STRING) {
		code_str = xdstrdup("");
	}

	convert_to_string_ex(message);
	convert_to_string_ex(file);
	convert_to_long_ex(line);

	previous_exception = xdebug_read_property(default_ce, exception, "previous", sizeof("previous")-1, 1 TSRMLS_CC);
	if (previous_exception && Z_TYPE_P(previous_exception) == IS_OBJECT) {
		xdebug_message_trace = xdebug_read_property(default_ce, previous_exception, "xdebug_message", sizeof("xdebug_message")-1, 1 TSRMLS_CC);
		if (xdebug_message_trace && Z_TYPE_P(xdebug_message_trace) != IS_NULL) {
			xdebug_str_add(&tmp_str, Z_STRVAL_P(xdebug_message_trace), 0);
		}
	}

	if (!PG(html_errors)) {
		xdebug_str_addl(&tmp_str, "\n", 1, 0);
	}
	xdebug_append_error_description(&tmp_str, PG(html_errors), STR_NAME_VAL(exception_ce->name), Z_STRVAL_P(message), Z_STRVAL_P(file), Z_LVAL_P(line) TSRMLS_CC);
	xdebug_append_printable_stack(&tmp_str, PG(html_errors) TSRMLS_CC);
	exception_trace = tmp_str.d;
	zend_update_property_string(default_ce, exception, "xdebug_message", sizeof("xdebug_message")-1, exception_trace TSRMLS_CC);

	if (XG(last_exception_trace)) {
		xdfree(XG(last_exception_trace));
	}
	XG(last_exception_trace) = exception_trace;

	if (XG(show_ex_trace) || (instanceof_function(exception_ce, zend_ce_error) && XG(show_error_trace))) {
		if (PG(log_errors)) {
			xdebug_log_stack(STR_NAME_VAL(exception_ce->name), Z_STRVAL_P(message), Z_STRVAL_P(file), Z_LVAL_P(line) TSRMLS_CC);
		}
		if (PG(display_errors)) {
			xdebug_str displ_tmp_str = XDEBUG_STR_INITIALIZER;
			xdebug_append_error_head(&displ_tmp_str, PG(html_errors), "exception" TSRMLS_CC);
			xdebug_str_add(&displ_tmp_str, exception_trace, 0);
			xdebug_append_error_footer(&displ_tmp_str, PG(html_errors) TSRMLS_CC);

			php_printf("%s", displ_tmp_str.d);
			xdebug_str_dtor(displ_tmp_str);
		}
	}

	/* Start JIT if requested and not yet enabled */
	xdebug_do_jit(TSRMLS_C);

	if (XG(remote_enabled)) {
		int exception_breakpoint_found = 0;

		/* Check if we have a wild card exception breakpoint */
		if (xdebug_hash_find(XG(context).exception_breakpoints, "*", 1, (void *) &extra_brk_info)) {
			exception_breakpoint_found = 1;
		} else {
			/* Check if we have a breakpoint on this exception or its parent classes */
			zend_class_entry *ce_ptr = exception_ce;

			/* Check if we have a breakpoint on this exception or its parent classes */
			do {
				if (xdebug_hash_find(XG(context).exception_breakpoints, (char *) STR_NAME_VAL(ce_ptr->name), STR_NAME_LEN(ce_ptr->name), (void *) &extra_brk_info)) {
					exception_breakpoint_found = 1;
				}
				ce_ptr = ce_ptr->parent;
			} while (!exception_breakpoint_found && ce_ptr);
		}

		if (exception_breakpoint_found && xdebug_handle_hit_value(extra_brk_info)) {
			if (!XG(context).handler->remote_breakpoint(
				&(XG(context)), XG(stack),
				Z_STRVAL_P(file), Z_LVAL_P(line), XDEBUG_BREAK,
				(char*) STR_NAME_VAL(exception_ce->name),
				code_str ? code_str : ((code && Z_TYPE_P(code) == IS_STRING) ? Z_STRVAL_P(code) : NULL),
				Z_STRVAL_P(message))
			) {
				XG(remote_enabled) = 0;
			}
		}
	}

	/* Free code_str if necessary */
	if (code_str) {
		xdfree(code_str);
	}
}

static int handle_breakpoints(function_stack_entry *fse, int breakpoint_type)
{
	xdebug_brk_info *extra_brk_info = NULL;
	char            *tmp_name = NULL;
	size_t           tmp_len = 0;
	TSRMLS_FETCH();

	/* Function breakpoints */
	if (fse->function.type == XFUNC_NORMAL) {
		if (xdebug_hash_find(XG(context).function_breakpoints, fse->function.function, strlen(fse->function.function), (void *) &extra_brk_info)) {
			/* Yup, breakpoint found, we call the handler when it's not
			 * disabled AND handle_hit_value is happy */
			if (!extra_brk_info->disabled && (extra_brk_info->function_break_type == breakpoint_type)) {
				if (xdebug_handle_hit_value(extra_brk_info)) {
					if (fse->user_defined == XDEBUG_INTERNAL || (breakpoint_type == XDEBUG_BRK_FUNC_RETURN)) {
						if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), fse->filename, fse->lineno, XDEBUG_BREAK, NULL, 0, NULL)) {
							return 0;
						}
					} else {
						XG(context).do_break = 1;
					}
				}
			}
		}
	}
	/* class->function breakpoints */
	else if (fse->function.type == XFUNC_MEMBER || fse->function.type == XFUNC_STATIC_MEMBER) {
		/* We intentionally do not use xdebug_sprintf because it can create a bottleneck in large
		   codebases due to setlocale calls. We don't care about the locale here. */
		tmp_len = strlen(fse->function.class) + strlen(fse->function.function) + 3;
		tmp_name = xdmalloc(tmp_len);
		snprintf(tmp_name, tmp_len, "%s::%s", fse->function.class, fse->function.function);

		if (xdebug_hash_find(XG(context).function_breakpoints, tmp_name, tmp_len - 1, (void *) &extra_brk_info)) {
			/* Yup, breakpoint found, call handler if the breakpoint is not
			 * disabled AND handle_hit_value is happy */
			if (!extra_brk_info->disabled && (extra_brk_info->function_break_type == breakpoint_type)) {
				if (xdebug_handle_hit_value(extra_brk_info)) {
					XG(context).do_break = 1;
				}
			}
		}
		xdfree(tmp_name);
	}
	return 1;
}

void xdebug_execute_ex(zend_execute_data *execute_data TSRMLS_DC)
{
	zend_op_array        *op_array = &(execute_data->func->op_array);
	zend_execute_data    *edata = execute_data->prev_execute_data;
	zval                 *dummy;
	function_stack_entry *fse, *xfse;
	char                 *magic_cookie = NULL;
	int                   do_return = (XG(do_trace) && XG(trace_context));
	int                   function_nr = 0;
	xdebug_llist_element *le;
	xdebug_func           code_coverage_func_info;
	char                 *code_coverage_function_name = NULL;
	char                 *code_coverage_file_name = NULL;
	int                   code_coverage_init = 0;

	/* For PHP 7, we need to reset the opline to the start, so that all opcode
	 * handlers are being hit. But not for generators, as that would make an
	 * endless loop. TODO: Fix RECV handling with generators. */
	if (!(EX(func)->op_array.fn_flags & ZEND_ACC_GENERATOR)) {
		EX(opline) = EX(func)->op_array.opcodes;
	}

	/* We need to do this first before the executable clauses are called */
	if (XG(no_exec) == 1) {
		php_printf("DEBUG SESSION ENDED");
		return;
	}

	/* If we're evaluating for the debugger's eval capability, just bail out */
	if (op_array && op_array->filename && strcmp("xdebug://debug-eval", STR_NAME_VAL(op_array->filename)) == 0) {
		xdebug_old_execute_ex(execute_data TSRMLS_CC);
		return;
	}

	/* if we're in a ZEND_EXT_STMT, we ignore this function call as it's likely
	   that it's just being called to check for breakpoints with conditions */
	if (edata && edata->func && ZEND_USER_CODE(edata->func->type) && edata->opline && edata->opline->opcode == ZEND_EXT_STMT) {
		xdebug_old_execute_ex(execute_data TSRMLS_CC);
		return;
	}

	if (!XG(context).program_name) {
		XG(context).program_name = xdstrdup(STR_NAME_VAL(op_array->filename));
	}

	if (XG(level) == 0 && XG(in_execution)) {
		/* Set session cookie if requested */
		if (
			((
				(dummy = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_GET]), "XDEBUG_SESSION_START", sizeof("XDEBUG_SESSION_START") - 1)) != NULL
			) || (
				(dummy = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_POST]), "XDEBUG_SESSION_START", sizeof("XDEBUG_SESSION_START") - 1)) != NULL
			))
			&& !SG(headers_sent)
		) {
			convert_to_string_ex(dummy);
			magic_cookie = xdstrdup(Z_STRVAL_P(dummy));
			if (XG(ide_key)) {
				xdfree(XG(ide_key));
			}
			XG(ide_key) = xdstrdup(magic_cookie);
			xdebug_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), Z_STRVAL_P(dummy), Z_STRLEN_P(dummy), time(NULL) + XG(remote_cookie_expire_time), "/", 1, NULL, 0, 0, 1, 0 TSRMLS_CC);
		} else if (
			(dummy = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_COOKIE]), "XDEBUG_SESSION", sizeof("XDEBUG_SESSION") - 1)) != NULL
		) {
			convert_to_string_ex(dummy);
			magic_cookie = xdstrdup(Z_STRVAL_P(dummy));
			if (XG(ide_key)) {
				xdfree(XG(ide_key));
			}
			XG(ide_key) = xdstrdup(magic_cookie);
		} else if (getenv("XDEBUG_CONFIG")) {
			magic_cookie = xdstrdup(getenv("XDEBUG_CONFIG"));
			if (XG(ide_key) && *XG(ide_key) && !SG(headers_sent)) {
				xdebug_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), XG(ide_key), strlen(XG(ide_key)), time(NULL) + XG(remote_cookie_expire_time), "/", 1, NULL, 0, 0, 1, 0 TSRMLS_CC);
			}
		}


		/* Remove session cookie if requested */
		if (
			((
				zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_GET]), "XDEBUG_SESSION_STOP", sizeof("XDEBUG_SESSION_STOP") - 1) != NULL
			) || (
				zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_POST]), "XDEBUG_SESSION_STOP", sizeof("XDEBUG_SESSION_STOP") - 1) != NULL
			))
			&& !SG(headers_sent)
		) {
			if (magic_cookie) {
				xdfree(magic_cookie);
				magic_cookie = NULL;
			}
			xdebug_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), (char*) "", 0, time(NULL) + XG(remote_cookie_expire_time), "/", 1, NULL, 0, 0, 1, 0 TSRMLS_CC);
		}

		/* Start remote context if requested */
		if (
			(magic_cookie || XG(remote_autostart)) &&
			!XG(remote_enabled) &&
			XG(remote_enable) &&
			(XG(remote_mode) == XDEBUG_REQ)
		) {
			xdebug_init_debugger(TSRMLS_C);
		}
		if (magic_cookie) {
			xdfree(magic_cookie);
			magic_cookie = NULL;
		}

		/* Check for special GET/POST parameter to start profiling */
		if (
			!XG(profiler_enabled) &&
			(XG(profiler_enable) || xdebug_trigger_enabled(XG(profiler_enable_trigger), "XDEBUG_PROFILE", XG(profiler_enable_trigger_value) TSRMLS_CC))
		) {
			if (xdebug_profiler_init((char*) STR_NAME_VAL(op_array->filename) TSRMLS_CC) == SUCCESS) {
				if (!SG(headers_sent)) {
					sapi_header_line ctr = {0};

					ctr.line = xdebug_sprintf("X-Xdebug-Profile-Filename: %s", XG(profile_filename));
					ctr.line_len = strlen(ctr.line);
					sapi_header_op(SAPI_HEADER_REPLACE, &ctr);
					xdfree(ctr.line);
				}

				XG(profiler_enabled) = 1;
			}
		}

		if (
			(XG(auto_trace) || xdebug_trigger_enabled(XG(trace_enable_trigger), "XDEBUG_TRACE", XG(trace_enable_trigger_value) TSRMLS_CC))
			&& XG(trace_output_dir) && strlen(XG(trace_output_dir))
		) {
			/* In case we do an auto-trace we are not interested in the return
			 * value, but we still have to free it. */
			xdfree(xdebug_start_trace(NULL, STR_NAME_VAL(op_array->filename), XG(trace_options) TSRMLS_CC));
		}

		if (!XG(gc_stats_enabled) && XG(gc_stats_enable)) {
			if (xdebug_gc_stats_init(NULL, STR_NAME_VAL(op_array->filename)) == SUCCESS) {
				XG(gc_stats_enabled) = 1;
			}
		}
	}

	XG(level)++;
	if ((signed long) XG(level) > XG(max_nesting_level) && (XG(max_nesting_level) != -1)) {
		zend_throw_exception_ex(zend_ce_error, 0, "Maximum function nesting level of '%ld' reached, aborting!", XG(max_nesting_level));
	}

	fse = xdebug_add_stack_frame(edata, op_array, XDEBUG_EXTERNAL TSRMLS_CC);
	fse->function.internal = 0;

	/* A hack to make __call work with profiles. The function *is* user defined after all. */
	if (fse && fse->prev && fse->function.function && (strcmp(fse->function.function, "__call") == 0)) {
		fse->prev->user_defined = XDEBUG_EXTERNAL;
	}

	function_nr = XG(function_count);
	if (!fse->filtered_tracing && XG(do_trace) && XG(trace_context) && (XG(trace_handler)->function_entry)) {
		XG(trace_handler)->function_entry(XG(trace_context), fse, function_nr TSRMLS_CC);
	}

	fse->execute_data = EG(current_execute_data)->prev_execute_data;
#if PHP_VERSION_ID >= 70100
	if (ZEND_CALL_INFO(EG(current_execute_data)) & ZEND_CALL_HAS_SYMBOL_TABLE) {
		fse->symbol_table = EG(current_execute_data)->symbol_table;
	}
#else
	fse->symbol_table = EG(current_execute_data)->symbol_table;
#endif
	if (Z_OBJ(EG(current_execute_data)->This)) {
		fse->This = &EG(current_execute_data)->This;
	} else {
		fse->This = NULL;
	}

	if (XG(stack) && (XG(remote_enabled) || XG(collect_vars) || XG(show_local_vars))) {
		/* Because include/require is treated as a stack level, we have to add used
		 * variables in include/required files to all the stack levels above, until
		 * we hit a function or the top level stack.  This is so that the variables
		 * show up correctly where they should be.  We always call
		 * add_used_variables on the current stack level, otherwise vars in include
		 * files do not show up in the locals list.  */
		for (le = XDEBUG_LLIST_TAIL(XG(stack)); le != NULL; le = XDEBUG_LLIST_PREV(le)) {
			xfse = XDEBUG_LLIST_VALP(le);
			add_used_variables(xfse, op_array);
			if (XDEBUG_IS_FUNCTION(xfse->function.type)) {
				break;
			}
		}
	}

	if (!fse->filtered_code_coverage && XG(do_code_coverage) && XG(code_coverage_unused)) {
		code_coverage_file_name = xdstrdup(STR_NAME_VAL(op_array->filename));
		xdebug_build_fname_from_oparray(&code_coverage_func_info, op_array TSRMLS_CC);
		code_coverage_function_name = xdebug_func_format(&code_coverage_func_info TSRMLS_CC);
		xdebug_code_coverage_start_of_function(op_array, code_coverage_function_name TSRMLS_CC);

		if (code_coverage_func_info.class) {
			xdfree(code_coverage_func_info.class);
		}
		if (code_coverage_func_info.function) {
			xdfree(code_coverage_func_info.function);
		}
		code_coverage_init = 1;
	}

	/* If we're in an eval, we need to create an ID for it. This ID however
	 * depends on the debugger mechanism in use so we need to call a function
	 * in the handler for it */
	if (XG(remote_enabled) && XG(context).handler->register_eval_id && fse->function.type == XFUNC_EVAL) {
		XG(context).handler->register_eval_id(&(XG(context)), fse);
	}

	/* Check for entry breakpoints */
	if (XG(remote_enabled) && XG(breakpoints_allowed)) {
		if (!handle_breakpoints(fse, XDEBUG_BRK_FUNC_CALL)) {
			XG(remote_enabled) = 0;
		}
	}

	if (XG(profiler_enabled)) {
		/* Calculate all elements for profile entries */
		xdebug_profiler_add_function_details_user(fse, op_array TSRMLS_CC);
		xdebug_profiler_function_begin(fse TSRMLS_CC);
	}

	xdebug_old_execute_ex(execute_data TSRMLS_CC);

	if (XG(profiler_enabled)) {
		xdebug_profiler_function_end(fse TSRMLS_CC);
		xdebug_profiler_free_function_details(fse TSRMLS_CC);
	}

	/* Check which path has been used */
	if (!fse->filtered_code_coverage && XG(do_code_coverage) && XG(code_coverage_unused) && code_coverage_init) {
		xdebug_code_coverage_end_of_function(op_array, code_coverage_file_name, code_coverage_function_name TSRMLS_CC);
		xdfree(code_coverage_function_name);
		xdfree(code_coverage_file_name);
	}


	if (!fse->filtered_tracing && XG(do_trace) && XG(trace_context) && (XG(trace_handler)->function_exit)) {
		XG(trace_handler)->function_exit(XG(trace_context), fse, function_nr TSRMLS_CC);
	}

	/* Store return value in the trace file */
	if (!fse->filtered_tracing && XG(collect_return) && do_return && XG(do_trace) && XG(trace_context)) {
		if (execute_data && execute_data->return_value) {
			if (op_array->fn_flags & ZEND_ACC_GENERATOR) {
				if (XG(trace_handler)->generator_return_value) {
					XG(trace_handler)->generator_return_value(XG(trace_context), fse, function_nr, (zend_generator*) execute_data->return_value TSRMLS_CC);
				}
			} else {
				if (XG(trace_handler)->return_value) {
					XG(trace_handler)->return_value(XG(trace_context), fse, function_nr, execute_data->return_value TSRMLS_CC);
				}
			}
		}
	}

	/* Check for return breakpoints */
	if (XG(remote_enabled) && XG(breakpoints_allowed)) {
		if (!handle_breakpoints(fse, XDEBUG_BRK_FUNC_RETURN)) {
			XG(remote_enabled) = 0;
		}
	}

	fse->symbol_table = NULL;
	fse->execute_data = NULL;
	if (XG(stack)) {
		xdebug_llist_remove(XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), function_stack_entry_dtor);
	}
	XG(level)--;
}

static int check_soap_call(function_stack_entry *fse)
{
	if (fse->function.class &&
		(
			(strstr(fse->function.class, "SoapClient") != NULL) ||
			(strstr(fse->function.class, "SoapServer") != NULL)
		) &&
		(zend_hash_str_find_ptr(&module_registry, "soap", sizeof("soap") - 1) != NULL)
	) {
		return 1;
	}
	return 0;
}

void xdebug_execute_internal(zend_execute_data *current_execute_data, zval *return_value)
{
	zend_execute_data    *edata = EG(current_execute_data);
	function_stack_entry *fse;
	int                   do_return = (XG(do_trace) && XG(trace_context));
	int                   function_nr = 0;

	int                   restore_error_handler_situation = 0;
	void                (*tmp_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args) = NULL;

	XG(level)++;
	if ((signed long) XG(level) > XG(max_nesting_level) && (XG(max_nesting_level) != -1)) {
		zend_throw_exception_ex(zend_ce_error, 0, "Maximum function nesting level of '%ld' reached, aborting!", XG(max_nesting_level));
	}

	fse = xdebug_add_stack_frame(edata, &edata->func->op_array, XDEBUG_INTERNAL TSRMLS_CC);
	fse->function.internal = 1;

	function_nr = XG(function_count);

	if (!fse->filtered_tracing && XG(do_trace) && fse->function.type != XFUNC_ZEND_PASS && XG(trace_context) && (XG(trace_handler)->function_entry)) {
		XG(trace_handler)->function_entry(XG(trace_context), fse, function_nr TSRMLS_CC);
	}

	/* Check for entry breakpoints */
	if (XG(remote_enabled) && XG(breakpoints_allowed)) {
		if (!handle_breakpoints(fse, XDEBUG_BRK_FUNC_CALL)) {
			XG(remote_enabled) = 0;
		}
	}

	/* Check for SOAP */
	if (check_soap_call(fse)) {
		restore_error_handler_situation = 1;
		tmp_error_cb = zend_error_cb;
		zend_error_cb = xdebug_old_error_cb;
	}

	if (XG(profiler_enabled)) {
		xdebug_profiler_add_function_details_internal(fse TSRMLS_CC);
		xdebug_profiler_function_begin(fse TSRMLS_CC);
	}

	if (xdebug_old_execute_internal) {
		xdebug_old_execute_internal(current_execute_data, return_value TSRMLS_CC);
	} else {
		execute_internal(current_execute_data, return_value TSRMLS_CC);
	}

	if (XG(profiler_enabled)) {
		xdebug_profiler_function_end(fse TSRMLS_CC);
		xdebug_profiler_free_function_details(fse TSRMLS_CC);
	}

	/* Restore SOAP situation if needed */
	if (restore_error_handler_situation) {
		zend_error_cb = tmp_error_cb;
	}

	if (!fse->filtered_tracing && XG(do_trace) && fse->function.type != XFUNC_ZEND_PASS && XG(trace_context) && (XG(trace_handler)->function_exit)) {
		XG(trace_handler)->function_exit(XG(trace_context), fse, function_nr TSRMLS_CC);
	}

	/* Store return value in the trace file */
	if (!fse->filtered_tracing && XG(collect_return) && do_return && XG(do_trace) && fse->function.type != XFUNC_ZEND_PASS && XG(trace_context) && return_value && XG(trace_handler)->return_value) {
		XG(trace_handler)->return_value(XG(trace_context), fse, function_nr, return_value TSRMLS_CC);
	}

	/* Check for return breakpoints */
	if (XG(remote_enabled) && XG(breakpoints_allowed)) {
		if (!handle_breakpoints(fse, XDEBUG_BRK_FUNC_RETURN)) {
			XG(remote_enabled) = 0;
		}
	}

	if (XG(stack)) {
		xdebug_llist_remove(XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), function_stack_entry_dtor);
	}
	XG(level)--;
}

/* Opcode handler for exit, to be able to clean up the profiler */
int xdebug_exit_handler(zend_execute_data *execute_data)
{
	if (XG(profiler_enabled)) {
		xdebug_profiler_deinit(TSRMLS_C);
	}

	return ZEND_USER_OPCODE_DISPATCH;
}

/* {{{ zend_op_array srm_compile_file (file_handle, type)
 *    This function provides a hook for the execution of bananas */
zend_op_array *xdebug_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC)
{
	zend_op_array *op_array;

	op_array = old_compile_file(file_handle, type TSRMLS_CC);

	if (op_array) {
		if (XG(do_code_coverage) && XG(code_coverage_unused) && (op_array->fn_flags & ZEND_ACC_DONE_PASS_TWO)) {
			xdebug_prefill_code_coverage(op_array TSRMLS_CC);
		}
	}
	return op_array;
}
/* }}} */

static void xdebug_header_remove_with_prefix(xdebug_llist *headers, char *prefix, size_t prefix_len TSRMLS_DC)
{
	xdebug_llist_element *le;
	char                 *header;

	for (le = XDEBUG_LLIST_HEAD(XG(headers)); le != NULL; /* intentionally left blank*/) {
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
	if (XG(headers)) {
		switch (op) {
			case SAPI_HEADER_ADD:
				xdebug_llist_insert_next(XG(headers), XDEBUG_LLIST_TAIL(XG(headers)), xdstrdup(h->header));
				break;
			case SAPI_HEADER_REPLACE: {
				char *colon_offset = strchr(h->header, ':');

				if (colon_offset) {
					char save = *colon_offset;

					*colon_offset = '\0';
					xdebug_header_remove_with_prefix(XG(headers), h->header, strlen(h->header) TSRMLS_CC);
					*colon_offset = save;
				}

				xdebug_llist_insert_next(XG(headers), XDEBUG_LLIST_TAIL(XG(headers)), xdstrdup(h->header));
			} break;
			case SAPI_HEADER_DELETE_ALL:
				xdebug_llist_empty(XG(headers), NULL);
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
	if (!XG(remote_enabled)) {
		XG(orig_set_time_limit_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	}
}
/* }}} */

/* {{{ proto void xdebug_pcntl_exec(void)
   Dummy function to prevent time limit from being set within the script */
PHP_FUNCTION(xdebug_pcntl_exec)
{
	/* We need to stop the profiler and trace files here */
	if (XG(profiler_enabled)) {
		xdebug_profiler_deinit(TSRMLS_C);
	}

	XG(orig_pcntl_exec_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
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
		!XG(overload_var_dump)
		&& (strcmp("xdebug_var_dump", execute_data->func->common.function_name->val) != 0)
	) {
		XG(orig_var_dump_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		return;
	}

	argc = ZEND_NUM_ARGS();

	args = safe_emalloc(argc, sizeof(zval), 0);
	if (ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
		efree(args);
		WRONG_PARAM_COUNT;
	}

	for (i = 0; i < argc; i++) {
		if (XG(default_enable) == 0) {
			xdebug_php_var_dump(&args[i], 1 TSRMLS_CC);
		}
		else if (PG(html_errors)) {
			val = xdebug_get_zval_value_fancy(NULL, (zval*) &args[i], 0, NULL);
			PHPWRITE(val->d, val->l);
			xdebug_str_free(val);
		}
		else if ((XG(cli_color) == 1 && xdebug_is_output_tty(TSRMLS_C)) || (XG(cli_color) == 2)) {
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

			XG(active_symbol_table) = EG(current_execute_data)->prev_execute_data->symbol_table;
			XG(active_execute_data) = EG(current_execute_data)->prev_execute_data;

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
				else if ((XG(cli_color) == 1 && xdebug_is_output_tty(TSRMLS_C)) || (XG(cli_color) == 2)) {
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

			XG(active_symbol_table) = EG(current_execute_data)->symbol_table;

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

PHP_FUNCTION(xdebug_enable)
{
	zend_error_cb = xdebug_new_error_cb;
	zend_throw_exception_hook = xdebug_throw_exception_hook;
}

PHP_FUNCTION(xdebug_disable)
{
	zend_error_cb = xdebug_old_error_cb;
	zend_throw_exception_hook = NULL;
}

PHP_FUNCTION(xdebug_is_enabled)
{
	RETURN_BOOL(zend_error_cb == xdebug_new_error_cb);
}

PHP_FUNCTION(xdebug_is_debugger_active)
{
	RETURN_BOOL(XG(remote_enabled));
}

PHP_FUNCTION(xdebug_break)
{
	/* Start JIT if requested and not yet enabled */
	xdebug_do_jit(TSRMLS_C);

	XG(context).do_break = 1;
	RETURN_TRUE;
}

PHP_FUNCTION(xdebug_start_error_collection)
{
	if (XG(do_collect_errors) == 1) {
		php_error(E_NOTICE, "Error collection was already started");
	}
	XG(do_collect_errors) = 1;
}

PHP_FUNCTION(xdebug_stop_error_collection)
{
	if (XG(do_collect_errors) == 0) {
		php_error(E_NOTICE, "Error collection was not started");
	}
	XG(do_collect_errors) = 0;
}

PHP_FUNCTION(xdebug_get_collected_errors)
{
	xdebug_llist_element *le;
	char                 *string;
	zend_bool             clear = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &clear) == FAILURE) {
		return;
	}

	array_init(return_value);
	for (le = XDEBUG_LLIST_HEAD(XG(collected_errors)); le != NULL; le = XDEBUG_LLIST_NEXT(le))	{
		string = XDEBUG_LLIST_VALP(le);
		add_next_index_string(return_value, string);
	}

	if (clear) {
		xdebug_llist_destroy(XG(collected_errors), NULL);
		XG(collected_errors) = xdebug_llist_alloc(xdebug_llist_string_dtor);
	}
}


PHP_FUNCTION(xdebug_get_headers)
{
	xdebug_llist_element *le;
	char                 *string;

	array_init(return_value);
	for (le = XDEBUG_LLIST_HEAD(XG(headers)); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
		string = XDEBUG_LLIST_VALP(le);
		add_next_index_string(return_value, string);
	}
}


PHP_FUNCTION(xdebug_get_profiler_filename)
{
	if (XG(profile_filename)) {
		RETURN_STRING(XG(profile_filename));
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_dump_aggr_profiling_data)
{
	char *prefix = NULL;
	size_t prefix_len;

	if (!XG(profiler_aggregate)) {
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
	if (!XG(profiler_aggregate)) {
		RETURN_FALSE;
	}

	zend_hash_clean(&XG(aggr_calls));

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
	RETURN_DOUBLE(xdebug_get_utime() - XG(start_time));
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
	int                   level = 0;
	int                   func_nr = 0;
	TSRMLS_FETCH();

	if (!EG(current_execute_data)) {
		return;
	}

	lineno = EG(current_execute_data)->opline->lineno;

	file = (char*) STR_NAME_VAL(op_array->filename);

	if (!op_array->reserved[XG(code_coverage_filter_offset)] && XG(do_code_coverage)) {
		xdebug_count_line(file, lineno, 0, 0 TSRMLS_CC);
	}

	if (XG(remote_enabled)) {

		if (XG(context).do_break) {
			XG(context).do_break = 0;

			if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_BREAK, NULL, 0, NULL)) {
				XG(remote_enabled) = 0;
				return;
			}
		}

		/* Get latest stack level and function number */
		if (XG(stack)) {
			le = XDEBUG_LLIST_TAIL(XG(stack));
			fse = XDEBUG_LLIST_VALP(le);
			level = fse->level;
			func_nr = fse->function_nr;
		} else {
			level = 0;
			func_nr = 0;
		}

		/* Check for "finish" */
		if (
			XG(context).do_finish &&
			(
				(level < XG(context).finish_level) ||
				((level == XG(context).finish_level) && (func_nr > XG(context).finish_func_nr))
			)
		) {
			XG(context).do_finish = 0;

			if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_STEP, NULL, 0, NULL)) {
				XG(remote_enabled) = 0;
				return;
			}
			return;
		}

		/* Check for "next" */
		if (XG(context).do_next && XG(context).next_level >= level) {
			XG(context).do_next = 0;

			if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_STEP, NULL, 0, NULL)) {
				XG(remote_enabled) = 0;
				return;
			}
			return;
		}

		/* Check for "step" */
		if (XG(context).do_step) {
			XG(context).do_step = 0;

			if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_STEP, NULL, 0, NULL)) {
				XG(remote_enabled) = 0;
				return;
			}
			return;
		}

		if (XG(context).line_breakpoints) {
			int   break_ok;
			int   old_error_reporting;
			zval  retval;
			int   file_len = strlen(file);

			for (le = XDEBUG_LLIST_HEAD(XG(context).line_breakpoints); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
				extra_brk_info = XDEBUG_LLIST_VALP(le);

#if 0
				printf("b->d: %d; ln: %d; b->l: %d; b->f: %s; f: %s, f_l: %d; b->f_l: %d\n",
						extra_brk_info->disabled, lineno, extra_brk_info->lineno, extra_brk_info->file, file, file_len, extra_brk_info->file_len);
#endif
				if (!extra_brk_info->disabled && lineno == extra_brk_info->lineno && file_len >= extra_brk_info->file_len && strncasecmp(extra_brk_info->file, file + file_len - extra_brk_info->file_len, extra_brk_info->file_len) == 0) {
					break_ok = 1; /* Breaking is allowed by default */

					/* Check if we have a condition set for it */
					if (extra_brk_info->condition) {
						/* If there is a condition, we disable breaking by
						 * default and only enabled it when the code evaluates
						 * to TRUE */
						break_ok = 0;

						/* Remember error reporting level */
						old_error_reporting = EG(error_reporting);
						EG(error_reporting) = 0;
						XG(context).inhibit_notifications = 1;

						/* Check the condition */
						if (zend_eval_string(extra_brk_info->condition, &retval, (char*) "xdebug conditional breakpoint" TSRMLS_CC) == SUCCESS) {
							break_ok = Z_TYPE(retval) == IS_TRUE;
							zval_dtor(&retval);
						}

						/* Restore error reporting level */
						EG(error_reporting) = old_error_reporting;
						XG(context).inhibit_notifications = 0;
					}
					if (break_ok && xdebug_handle_hit_value(extra_brk_info)) {
						if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_BREAK, NULL, 0, NULL)) {
							XG(remote_enabled) = 0;
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

	return zend_startup_module(&xdebug_module_entry);
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
