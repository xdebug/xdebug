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

#ifndef PHP_XDEBUG_H
#define PHP_XDEBUG_H

#define XDEBUG_NAME       "Xdebug"
#define XDEBUG_VERSION    "2.9.8"
#define XDEBUG_AUTHOR     "Derick Rethans"
#define XDEBUG_COPYRIGHT  "Copyright (c) 2002-2020 by Derick Rethans"
#define XDEBUG_COPYRIGHT_SHORT "Copyright (c) 2002-2020"
#define XDEBUG_URL        "https://xdebug.org"
#define XDEBUG_URL_FAQ    "https://xdebug.org/docs/faq#api"

#include "php.h"

#include "coverage/branch_info.h"
#include "coverage/code_coverage.h"
#include "debugger/debugger.h"
#include "lib/lib.h"
#include "gcstats/gc_stats.h"
#include "profiler/profiler.h"
#include "tracing/tracing.h"
#include "lib/compat.h"
#include "lib/hash.h"
#include "lib/llist.h"

extern zend_module_entry xdebug_module_entry;
#define phpext_xdebug_ptr &xdebug_module_entry

#define MICRO_IN_SEC 1000000.00

#define OUTPUT_NOT_CHECKED -1
#define OUTPUT_IS_TTY       1
#define OUTPUT_NOT_TTY      0

#ifdef PHP_WIN32
#define PHP_XDEBUG_API __declspec(dllexport)
#else
#define PHP_XDEBUG_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include "main/SAPI.h"

#define XDEBUG_ALLOWED_HALT_LEVELS (E_WARNING | E_NOTICE | E_USER_WARNING | E_USER_NOTICE )

PHP_MINIT_FUNCTION(xdebug);
PHP_MSHUTDOWN_FUNCTION(xdebug);
PHP_RINIT_FUNCTION(xdebug);
PHP_RSHUTDOWN_FUNCTION(xdebug);
PHP_MINFO_FUNCTION(xdebug);
ZEND_MODULE_POST_ZEND_DEACTIVATE_D(xdebug);

#ifndef PHP_WIN32
int xdebug_is_output_tty();
#endif

/* call stack functions */
PHP_FUNCTION(xdebug_get_stack_depth);
PHP_FUNCTION(xdebug_get_function_stack);
PHP_FUNCTION(xdebug_get_formatted_function_stack);
PHP_FUNCTION(xdebug_print_function_stack);
PHP_FUNCTION(xdebug_get_declared_vars);
PHP_FUNCTION(xdebug_call_class);
PHP_FUNCTION(xdebug_call_function);
PHP_FUNCTION(xdebug_call_file);
PHP_FUNCTION(xdebug_call_line);

PHP_FUNCTION(xdebug_set_time_limit);
PHP_FUNCTION(xdebug_error_reporting);
PHP_FUNCTION(xdebug_pcntl_exec);

PHP_FUNCTION(xdebug_var_dump);
PHP_FUNCTION(xdebug_debug_zval);
PHP_FUNCTION(xdebug_debug_zval_stdout);

/* activation functions */
PHP_FUNCTION(xdebug_enable);
PHP_FUNCTION(xdebug_disable);
PHP_FUNCTION(xdebug_is_enabled);

/* breaking functions */
PHP_FUNCTION(xdebug_is_debugger_active);
PHP_FUNCTION(xdebug_break);

/* tracing functions */
PHP_FUNCTION(xdebug_start_trace);
PHP_FUNCTION(xdebug_stop_trace);
PHP_FUNCTION(xdebug_get_tracefile_name);

/* error collecting functions */
PHP_FUNCTION(xdebug_start_error_collection);
PHP_FUNCTION(xdebug_stop_error_collection);
PHP_FUNCTION(xdebug_get_collected_errors);

/* function monitorin functions */
PHP_FUNCTION(xdebug_start_function_monitor);
PHP_FUNCTION(xdebug_stop_function_monitor);
PHP_FUNCTION(xdebug_get_monitored_functions);

/* profiling functions */
PHP_FUNCTION(xdebug_get_profiler_filename);
PHP_FUNCTION(xdebug_dump_aggr_profiling_data);
PHP_FUNCTION(xdebug_clear_aggr_profiling_data);

/* gc stats functions */
PHP_FUNCTION(xdebug_start_gcstats);
PHP_FUNCTION(xdebug_stop_gcstats);
PHP_FUNCTION(xdebug_get_gcstats_filename);
PHP_FUNCTION(xdebug_get_gc_run_count);
PHP_FUNCTION(xdebug_get_gc_total_collected_roots);

/* misc functions */
PHP_FUNCTION(xdebug_dump_superglobals);
PHP_FUNCTION(xdebug_get_headers);
PHP_FUNCTION(xdebug_memory_usage);
PHP_FUNCTION(xdebug_peak_memory_usage);
PHP_FUNCTION(xdebug_time_index);

/* filter functions */
PHP_FUNCTION(xdebug_set_filter);

struct xdebug_base_info {
	unsigned long level;
	xdebug_llist *stack;
	double        start_time;
	unsigned int  prev_memory;
	zif_handler   orig_var_dump_func;
	zif_handler   orig_set_time_limit_func;
	zif_handler   orig_error_reporting_func;
	zif_handler   orig_pcntl_exec_func;
	int           output_is_tty;
	zend_bool     in_debug_info;
	char         *last_exception_trace;
	zend_long     error_reporting_override;
	zend_bool     error_reporting_overridden;
	unsigned int  function_count;
	char         *last_eval_statement;

	/* headers */
	xdebug_llist *headers;

	/* used for collection errors */
	zend_bool     do_collect_errors;
	xdebug_llist *collected_errors;

	/* used for function monitoring */
	zend_bool     do_monitor_functions;
	xdebug_hash  *functions_to_monitor;
	xdebug_llist *monitored_functions_found; /* List of functions found */

	/* superglobals */
	zend_bool     dumped;
	xdebug_llist  server;
	xdebug_llist  get;
	xdebug_llist  post;
	xdebug_llist  cookie;
	xdebug_llist  files;
	xdebug_llist  env;
	xdebug_llist  request;
	xdebug_llist  session;

	/* scream */
	zend_bool  in_at;

	/* in-execution checking */
	zend_bool  in_execution;
	zend_bool  in_var_serialisation;

	/* filters */
	zend_long     filter_type_tracing;
	zend_long     filter_type_profiler;
	zend_long     filter_type_code_coverage;
	xdebug_llist *filters_tracing;
	xdebug_llist *filters_code_coverage;

	struct {
		zend_long     max_nesting_level;
		zend_long     max_stack_frames;
		zend_bool     default_enable;
		zend_bool     collect_includes;
		zend_long     collect_params;
		zend_bool     collect_return;
		zend_bool     collect_vars;
		zend_bool     collect_assignments;
		zend_bool     show_ex_trace;
		zend_bool     show_error_trace;
		zend_bool     show_local_vars;
		zend_bool     show_mem_delta;
		char         *file_link_format;
		char         *filename_format;
		zend_bool     force_display_errors;
		zend_long     force_error_reporting;
		zend_long     halt_level;

		zend_long     overload_var_dump;

		/* variable dumping limitation settings */
		zend_long     display_max_children;
		zend_long     display_max_data;
		zend_long     display_max_depth;
		zend_long     cli_color;

		/* superglobals */
		zend_bool     dump_globals;
		zend_bool     dump_once;
		zend_bool     dump_undefined;

		/* scream */
		zend_bool  do_scream;
	} settings;
};

ZEND_BEGIN_MODULE_GLOBALS(xdebug)
	struct xdebug_base_info     base;
	struct {
		xdebug_coverage_globals_t coverage;
		xdebug_debugger_globals_t debugger;
		xdebug_gc_stats_globals_t gc_stats;
		xdebug_library_globals_t  library;
		xdebug_profiler_globals_t profiler;
		xdebug_tracing_globals_t  tracing;
	} globals;
	struct {
		xdebug_coverage_settings_t coverage;
		xdebug_debugger_settings_t debugger;
		xdebug_gc_stats_settings_t gc_stats;
		xdebug_profiler_settings_t profiler;
		xdebug_tracing_settings_t  tracing;
	} settings;
ZEND_END_MODULE_GLOBALS(xdebug)

#ifdef ZTS
#define XG(v) TSRMG(xdebug_globals_id, zend_xdebug_globals *, v)
#else
#define XG(v) (xdebug_globals.v)
#endif

#define XG_BASE(v)     (XG(base.v))
#define XINI_BASE(v)     (XG(base.settings.v))

#endif
