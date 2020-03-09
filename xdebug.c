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

#include "base/base.h"
#include "base/filter.h"
#include "base/monitor.h"
#include "base/stack.h"
#include "base/superglobals.h"
#include "coverage/code_coverage.h"
#include "gcstats/gc_stats.h"
#include "lib/usefulstuff.h"
#include "lib/lib.h"
#include "lib/llist.h"
#include "lib/mm.h"
#include "lib/var_export_html.h"
#include "lib/var_export_line.h"
#include "lib/var_export_text.h"
#include "profiler/profiler.h"
#include "tracing/tracing.h"

#if PHP_VERSION_ID >= 70300
static int (*xdebug_orig_post_startup_cb)(void);
static int xdebug_post_startup(void);
#endif

static int xdebug_header_handler(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s);

int xdebug_exit_handler(zend_execute_data *execute_data);

int zend_xdebug_initialised = 0;

static int (*xdebug_orig_header_handler)(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s);

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
	STD_PHP_INI_BOOLEAN("xdebug.coverage_enable", "1",                  PHP_INI_SYSTEM, OnUpdateBool,   settings.coverage.enable,         zend_xdebug_globals, xdebug_globals)
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
	STD_PHP_INI_BOOLEAN("xdebug.profiler_enable",         "0",                  PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   settings.profiler.profiler_enable,               zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.profiler_output_dir",       XDEBUG_TEMP_DIR,      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, settings.profiler.profiler_output_dir,           zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.profiler_output_name",      "cachegrind.out.%p",  PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, settings.profiler.profiler_output_name,          zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_enable_trigger", "0",                  PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   settings.profiler.profiler_enable_trigger,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.profiler_enable_trigger_value", "",               PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, settings.profiler.profiler_enable_trigger_value, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_append",         "0",                  PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   settings.profiler.profiler_append,               zend_xdebug_globals, xdebug_globals)

	/* Remote debugger settings */
	STD_PHP_INI_BOOLEAN("xdebug.remote_enable",   "0",   PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   settings.debugger.remote_enable,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_host",       "localhost",          PHP_INI_ALL,    OnUpdateString, settings.debugger.remote_host,       zend_xdebug_globals, xdebug_globals)
	PHP_INI_ENTRY("xdebug.remote_mode",           "req",                PHP_INI_ALL,    OnUpdateDebugMode)
	STD_PHP_INI_ENTRY("xdebug.remote_port",       "9000",               PHP_INI_ALL,    OnUpdateLong,   settings.debugger.remote_port,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.remote_autostart","0",                  PHP_INI_ALL,    OnUpdateBool,   settings.debugger.remote_autostart,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.remote_connect_back","0",               PHP_INI_ALL,    OnUpdateBool,   settings.debugger.remote_connect_back,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_log",        "",                   PHP_INI_ALL,    OnUpdateString, settings.debugger.remote_log,        zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_log_level",  XDEBUG_LOG_DEFAULT,   PHP_INI_ALL,    OnUpdateLong,   settings.debugger.remote_log_level,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.idekey",            "",                   PHP_INI_ALL,    OnUpdateString, settings.debugger.ide_key_setting,   zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_cookie_expire_time", "3600",       PHP_INI_ALL,    OnUpdateLong,   settings.debugger.remote_cookie_expire_time, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_addr_header", "",                  PHP_INI_ALL,    OnUpdateString, settings.debugger.remote_addr_header, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_timeout",    "200",                PHP_INI_ALL,    OnUpdateLong,   settings.debugger.remote_connect_timeout, zend_xdebug_globals, xdebug_globals)

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

	/* Tracing settings */
	STD_PHP_INI_BOOLEAN("xdebug.auto_trace",      "0",                  PHP_INI_ALL,    OnUpdateBool,   settings.tracing.auto_trace,        zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.trace_enable_trigger", "0",             PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   settings.tracing.trace_enable_trigger, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_enable_trigger_value", "",          PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, settings.tracing.trace_enable_trigger_value, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_output_dir",  XDEBUG_TEMP_DIR,      PHP_INI_ALL,    OnUpdateString, settings.tracing.trace_output_dir,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_output_name", "trace.%c",           PHP_INI_ALL,    OnUpdateString, settings.tracing.trace_output_name, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_format",      "0",                  PHP_INI_ALL,    OnUpdateLong,   settings.tracing.trace_format,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_options",     "0",                  PHP_INI_ALL,    OnUpdateLong,   settings.tracing.trace_options,     zend_xdebug_globals, xdebug_globals)
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

static void php_xdebug_init_globals (zend_xdebug_globals *xg)
{
	xdebug_init_base_globals(&xg->base);
	xdebug_init_coverage_globals(&xg->globals.coverage);
	xdebug_init_debugger_globals(&xg->globals.debugger);
	xdebug_init_library_globals(&xg->globals.library);
	xdebug_init_profiler_globals(&xg->globals.profiler);
	xdebug_init_gc_stats_globals(&xg->globals.gc_stats);
	xdebug_init_tracing_globals(&xg->globals.tracing);

	/* Override header generation in SAPI */
	if (sapi_module.header_handler != xdebug_header_handler) {
		xdebug_orig_header_handler = sapi_module.header_handler;
		sapi_module.header_handler = xdebug_header_handler;
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

static void php_xdebug_shutdown_globals (zend_xdebug_globals *xg)
{
	xdebug_deinit_base_globals(&xg->base);
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
			xdebug_debugger_reset_ide_key(envval);
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

int xdebug_is_output_tty(void)
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

PHP_MINIT_FUNCTION(xdebug)
{
	ZEND_INIT_MODULE_GLOBALS(xdebug, php_xdebug_init_globals, php_xdebug_shutdown_globals);
	REGISTER_INI_ENTRIES();

	xdebug_library_minit();

	xdebug_base_minit(INIT_FUNC_ARGS_PASSTHRU);
	xdebug_debugger_minit();
	xdebug_gcstats_minit();
	xdebug_profiler_minit();
	xdebug_tracing_minit(INIT_FUNC_ARGS_PASSTHRU);

	/* Overload the "exit" opcode */
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(exit, ZEND_EXIT);

	/* Coverage must be last, as it has a catch all override for opcodes */
	xdebug_coverage_minit(INIT_FUNC_ARGS_PASSTHRU);

	if (zend_xdebug_initialised == 0) {
		zend_error(E_WARNING, "Xdebug MUST be loaded as a Zend extension");
	}

	xdebug_filter_register_constants(INIT_FUNC_ARGS_PASSTHRU);

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(xdebug)
{
	xdebug_gcstats_mshutdown();
	xdebug_profiler_mshutdown();

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

	xdebug_coverage_rinit();
	xdebug_debugger_rinit();
	xdebug_gcstats_rinit();
	xdebug_profiler_rinit();
	xdebug_tracing_rinit();

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
	xdebug_coverage_post_deactivate();
	xdebug_debugger_post_deactivate();
	xdebug_gcstats_post_deactivate();
	xdebug_profiler_post_deactivate();
	xdebug_tracing_post_deactivate();

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
	php_info_print_table_start();
	php_info_print_table_header(2, "xdebug support", "enabled");
	php_info_print_table_row(2, "Version", XDEBUG_VERSION);
	if (!sapi_module.phpinfo_as_text) {
		xdebug_info_printf("<tr><td colspan='2' style='background-color: white; text-align: center'>%s</td></tr>\n", "<a style='color: #317E1E; background-color: transparent; font-weight: bold; text-decoration: underline' href='https://xdebug.org/support'>Support Xdebug on Patreon, GitHub, or as a business</a>");
	} else {
		xdebug_info_printf("Support Xdebug on Patreon, GitHub, or as a business: https://xdebug.org/support\n");
	}
	php_info_print_table_end();

	if (zend_xdebug_initialised == 0) {
		php_info_print_table_start();
		php_info_print_table_header(1, "XDEBUG NOT LOADED AS ZEND EXTENSION");
		php_info_print_table_end();
	}

	xdebug_debugger_minfo();

	DISPLAY_INI_ENTRIES();
}

static void xdebug_header_remove_with_prefix(xdebug_llist *headers, char *prefix, size_t prefix_len)
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

static int xdebug_header_handler(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s)
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
					xdebug_header_remove_with_prefix(XG_BASE(headers), h->header, strlen(h->header));
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
		return xdebug_orig_header_handler(h, op, s);
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
	xdebug_profiler_pcntl_exec_handler();

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
			xdebug_php_var_dump(&args[i], 1);
		}
		else if (PG(html_errors)) {
			val = xdebug_get_zval_value_html(NULL, (zval*) &args[i], 0, NULL);
			PHPWRITE(val->d, val->l);
			xdebug_str_free(val);
		}
		else if ((XINI_BASE(cli_color) == 1 && xdebug_is_output_tty()) || (XINI_BASE(cli_color) == 2)) {
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

	if (!(ZEND_CALL_INFO(EG(current_execute_data)->prev_execute_data) & ZEND_CALL_HAS_SYMBOL_TABLE)) {
		zend_rebuild_symbol_table();
	}

	for (i = 0; i < argc; i++) {
		if (Z_TYPE(args[i]) == IS_STRING) {
			zval debugzval;
			xdebug_str *tmp_name;

			xdebug_lib_set_active_symbol_table(EG(current_execute_data)->prev_execute_data->symbol_table);
			xdebug_lib_set_active_data(EG(current_execute_data)->prev_execute_data);

			tmp_name = xdebug_str_create(Z_STRVAL(args[i]), Z_STRLEN(args[i]));
			xdebug_get_php_symbol(&debugzval, tmp_name);
			xdebug_str_free(tmp_name);

			/* Reduce refcount for dumping */
			Z_TRY_DELREF(debugzval);

			php_printf("%s: ", Z_STRVAL(args[i]));
			if (Z_TYPE(debugzval) != IS_UNDEF) {
				if (PG(html_errors)) {
					val = xdebug_get_zval_value_html(NULL, &debugzval, 1, NULL);
					PHPWRITE(val->d, val->l);
				}
				else if ((XINI_BASE(cli_color) == 1 && xdebug_is_output_tty()) || (XINI_BASE(cli_color) == 2)) {
					val = xdebug_get_zval_value_ansi(&debugzval, 1, NULL);
					PHPWRITE(val->d, val->l);
				}
				else {
					val = xdebug_get_zval_value_line(&debugzval, 1, NULL);
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

	if (!(ZEND_CALL_INFO(EG(current_execute_data)->prev_execute_data) & ZEND_CALL_HAS_SYMBOL_TABLE)) {
		zend_rebuild_symbol_table();
	}

	for (i = 0; i < argc; i++) {
		if (Z_TYPE(args[i]) == IS_STRING) {
			zval        debugzval;
			xdebug_str *tmp_name;
			xdebug_str *val;

			xdebug_lib_set_active_symbol_table(EG(current_execute_data)->prev_execute_data->symbol_table);
			xdebug_lib_set_active_data(EG(current_execute_data)->prev_execute_data);

			tmp_name = xdebug_str_create(Z_STRVAL(args[i]), Z_STRLEN(args[i]));
			xdebug_get_php_symbol(&debugzval, tmp_name);
			xdebug_str_free(tmp_name);

			/* Reduce refcount for dumping */
			Z_TRY_DELREF(debugzval);

			printf("%s: ", Z_STRVAL(args[i]));
			if (Z_TYPE(debugzval) != IS_UNDEF) {
				val = xdebug_get_zval_value_line(&debugzval, 1, NULL);
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

PHP_FUNCTION(xdebug_memory_usage)
{
	RETURN_LONG(zend_memory_usage(0));
}

PHP_FUNCTION(xdebug_peak_memory_usage)
{
	RETURN_LONG(zend_memory_peak_usage(0));
}

PHP_FUNCTION(xdebug_time_index)
{
	RETURN_DOUBLE(xdebug_get_utime() - XG_BASE(start_time));
}

ZEND_DLEXPORT void xdebug_statement_call(zend_execute_data *frame)
{
	zend_op_array *op_array = &frame->func->op_array;
	int                   lineno;
	char                 *file;
	int                   file_len;

	if (!EG(current_execute_data)) {
		return;
	}

	lineno = EG(current_execute_data)->opline->lineno;

	file = (char*) STR_NAME_VAL(op_array->filename);
	file_len = STR_NAME_LEN(op_array->filename);

	xdebug_coverage_count_line_if_active(op_array, file, lineno);
	xdebug_debugger_statement_call(file, file_len, lineno);
}

ZEND_DLEXPORT int xdebug_zend_startup(zend_extension *extension)
{
	/* Override header handler in SAPI */
	if (xdebug_orig_header_handler == NULL) {
		xdebug_orig_header_handler = sapi_module.header_handler;
		sapi_module.header_handler = xdebug_header_handler;
	}

	xdebug_debugger_zend_startup();

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
	/* Restore original header handler in SAPI */
	sapi_module.header_handler = xdebug_orig_header_handler;
	xdebug_orig_header_handler = NULL;

	xdebug_debugger_zend_shutdown();
}

ZEND_DLEXPORT void xdebug_init_oparray(zend_op_array *op_array)
{
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
