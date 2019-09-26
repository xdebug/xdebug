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
   +----------------------------------------------------------------------+
 */

#ifndef PHP_XDEBUG_H
#define PHP_XDEBUG_H

#define XDEBUG_NAME       "Xdebug"
#define XDEBUG_VERSION    "3.0.0-dev"
#define XDEBUG_AUTHOR     "Derick Rethans"
#define XDEBUG_COPYRIGHT  "Copyright (c) 2002-2019 by Derick Rethans"
#define XDEBUG_COPYRIGHT_SHORT "Copyright (c) 2002-2019"
#define XDEBUG_URL        "https://xdebug.org"
#define XDEBUG_URL_FAQ    "https://xdebug.org/docs/faq#api"

#include "php.h"

#include "coverage/xdebug_branch_info.h"
#include "coverage/xdebug_code_coverage.h"
#include "debugger/xdebug_handlers.h"
#include "lib/xdebug_compat.h"
#include "lib/xdebug_hash.h"
#include "lib/xdebug_llist.h"

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

struct xdebug_stepdbg_info {
	int           status;
	int           reason;
	const char   *lastcmd;
	char         *lasttransid;

	zend_bool     remote_connection_enabled;
	zend_ulong    remote_connection_pid;
	zend_bool     breakpoints_allowed;
	xdebug_con    context;
	unsigned int  breakpoint_count;
	unsigned int  no_exec;
	char         *ide_key; /* As Xdebug uses it, from environment, USER, USERNAME or empty */
	FILE         *remote_log_file;  /* File handler for protocol log */

	HashTable    *active_symbol_table;
	zend_execute_data *active_execute_data;
	zval              *This;
	function_stack_entry *active_fse;

	/* output redirection */
	int           stdout_mode;

	struct {
		zend_bool     remote_enable;  /* 0 */
		zend_long     remote_port;    /* 9000 */
		char         *remote_host;    /* localhost */
		long          remote_mode;    /* XDEBUG_NONE, XDEBUG_JIT, XDEBUG_REQ */
		char         *remote_handler; /* php3, gdb, dbgp */
		zend_bool     remote_autostart; /* Disables the requirement for XDEBUG_SESSION_START */
		zend_bool     remote_connect_back;   /* connect back to the HTTP requestor */
		char         *remote_log;       /* Filename to log protocol communication to */
		int           remote_log_level; /* Log level XDEBUG_LOG_{ERR,WARN,INFO,DEBUG} */
		zend_long     remote_cookie_expire_time; /* Expire time for the remote-session cookie */
		char         *remote_addr_header; /* User configured header to check for forwarded IP address */
		zend_long     remote_connect_timeout; /* Timeout in MS for remote connections */

		char         *ide_key_setting; /* Set through php.ini and friends */
	} settings;
};

struct xdebug_trace_info {
	xdebug_trace_handler_t *trace_handler;
	void         *trace_context;

	struct {
		zend_bool     auto_trace;
		zend_bool     trace_enable_trigger;
		char         *trace_enable_trigger_value;
		char         *trace_output_dir;
		char         *trace_output_name;
		zend_long     trace_options;
		zend_long     trace_format;
	} settings;
};

struct xdebug_coverage_info {
	zend_bool     code_coverage_active; /* Whether code coverage is currently running */
	xdebug_hash  *code_coverage_info;   /* Stores code coverage information */
	zend_bool     code_coverage_unused;
	zend_bool     code_coverage_dead_code_analysis;
	zend_bool     code_coverage_branch_check;
	int           dead_code_analysis_tracker_offset;
	long          dead_code_last_start_id;
	long          code_coverage_filter_offset;
	char                 *previous_filename;
	xdebug_coverage_file *previous_file;
	char                 *previous_mark_filename;
	xdebug_coverage_file *previous_mark_file;
	xdebug_path_info     *paths_stack;
	xdebug_hash          *visited_classes;
	xdebug_hash          *visited_branches;
	struct {
		unsigned int  size;
		int *last_branch_nr;
	} branches;
	struct {
		zend_bool     code_coverage_enable; /* Flag to enable code coverage (and opcode overloading) */
	} settings;
};

struct xdebug_profiler_info {
	/* profiler globals */
	double        profiler_start_time;
	zend_bool     profiler_enabled;
	FILE         *profile_file;
	char         *profile_filename;
	xdebug_hash  *profile_filename_refs;
	int           profile_last_filename_ref;
	xdebug_hash  *profile_functionname_refs;
	int           profile_last_functionname_ref;

	/* aggregate profiling */
	HashTable  aggr_calls;

	struct {
		/* profiler settings */
		zend_bool     profiler_enable;
		char         *profiler_output_dir;
		char         *profiler_output_name; /* "pid" or "crc32" */
		zend_bool     profiler_enable_trigger;
		char         *profiler_enable_trigger_value;
		zend_bool     profiler_append;
		zend_bool     profiler_aggregate;
	} settings;
};

struct xdebug_gc_stats_info {
	/* garbage stats */
	zend_bool  gc_stats_enabled;
	FILE      *gc_stats_file;
	char      *gc_stats_filename;

	struct {
		zend_bool  gc_stats_enable;
		char      *gc_stats_output_dir;
		char      *gc_stats_output_name;
	} settings;
};

ZEND_BEGIN_MODULE_GLOBALS(xdebug)
	struct xdebug_base_info     base;
	struct xdebug_stepdbg_info  stepdbg;
	struct xdebug_trace_info    trace;
	struct xdebug_coverage_info coverage;
	struct xdebug_profiler_info profiler;
	struct xdebug_gc_stats_info gc_stats;
ZEND_END_MODULE_GLOBALS(xdebug)

#ifdef ZTS
#define XG(v) TSRMG(xdebug_globals_id, zend_xdebug_globals *, v)
#else
#define XG(v) (xdebug_globals.v)
#endif

#define XG_BASE(v)     (XG(base.v))
#define XG_COV(v)      (XG(coverage.v))
#define XG_DBG(v)      (XG(stepdbg.v))
#define XG_GCSTATS(v)  (XG(gc_stats.v))
#define XG_PROF(v)     (XG(profiler.v))
#define XG_TRACE(v)    (XG(trace.v))

#define XINI_BASE(v)     (XG(base.settings.v))
#define XINI_COV(v)      (XG(coverage.settings.v))
#define XINI_DBG(v)      (XG(stepdbg.settings.v))
#define XINI_GCSTATS(v)  (XG(gc_stats.settings.v))
#define XINI_PROF(v)     (XG(profiler.settings.v))
#define XINI_TRACE(v)    (XG(trace.settings.v))

#endif


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
