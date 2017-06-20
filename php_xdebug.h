/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2016 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   +----------------------------------------------------------------------+
 */

#ifndef PHP_XDEBUG_H
#define PHP_XDEBUG_H

#define XDEBUG_NAME       "Xdebug"
#define XDEBUG_VERSION    "2.5.5"
#define XDEBUG_AUTHOR     "Derick Rethans"
#define XDEBUG_COPYRIGHT  "Copyright (c) 2002-2017 by Derick Rethans"
#define XDEBUG_COPYRIGHT_SHORT "Copyright (c) 2002-2017"
#define XDEBUG_URL        "http://xdebug.org"
#define XDEBUG_URL_FAQ    "http://xdebug.org/docs/faq#api"

#include "php.h"

#include "xdebug_compat.h"
#include "xdebug_handlers.h"
#include "xdebug_hash.h"
#include "xdebug_llist.h"
#include "xdebug_branch_info.h"
#include "xdebug_code_coverage.h"

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
PHP_FUNCTION(xdebug_pcntl_exec);

PHP_FUNCTION(xdebug_var_dump);
PHP_FUNCTION(xdebug_debug_zval);
PHP_FUNCTION(xdebug_debug_zval_stdout);

/* activation functions */
PHP_FUNCTION(xdebug_enable);
PHP_FUNCTION(xdebug_disable);
PHP_FUNCTION(xdebug_is_enabled);

/* breaking functions */
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

/* misc functions */
PHP_FUNCTION(xdebug_dump_superglobals);
PHP_FUNCTION(xdebug_get_headers);
PHP_FUNCTION(xdebug_memory_usage);
PHP_FUNCTION(xdebug_peak_memory_usage);
PHP_FUNCTION(xdebug_time_index);

ZEND_BEGIN_MODULE_GLOBALS(xdebug)
	int           status;
	int           reason;

	unsigned long level;
	xdebug_llist *stack;
	iniLONG       max_nesting_level;
	iniLONG       max_stack_frames;
	zend_bool     default_enable;
	zend_bool     collect_includes;
	iniLONG       collect_params;
	zend_bool     collect_return;
	zend_bool     collect_vars;
	zend_bool     collect_assignments;
	zend_bool     extended_info;
	zend_bool     show_ex_trace;
	zend_bool     show_error_trace;
	zend_bool     show_local_vars;
	zend_bool     show_mem_delta;
	double        start_time;
	HashTable    *active_symbol_table;
	zend_execute_data *active_execute_data;
	zval              *This;
	function_stack_entry *active_fse;
	unsigned int  prev_memory;
	char         *file_link_format;
	zend_bool     force_display_errors;
	iniLONG       force_error_reporting;
	iniLONG       halt_level;

	zend_bool     overload_var_dump;
	void        (*orig_var_dump_func)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_set_time_limit_func)(INTERNAL_FUNCTION_PARAMETERS);
	void        (*orig_pcntl_exec_func)(INTERNAL_FUNCTION_PARAMETERS);

	xdebug_trace_handler_t *trace_handler;
	void         *trace_context;

	zend_bool     do_trace;
	zend_bool     auto_trace;
	zend_bool     trace_enable_trigger;
	char         *trace_enable_trigger_value;
	char         *trace_output_dir;
	char         *trace_output_name;
	iniLONG       trace_options;
	iniLONG       trace_format;
	char         *last_exception_trace;
	char         *last_eval_statement;
	zend_bool     in_debug_info;

	/* variable dumping limitation settings */
	iniLONG       display_max_children;
	iniLONG       display_max_data;
	iniLONG       display_max_depth;

	zend_bool     cli_color;
	int           output_is_tty;

	/* used for code coverage */
	zend_bool     coverage_enable;
	zend_bool     do_code_coverage;
	xdebug_hash  *code_coverage;
	zend_bool     code_coverage_unused;
	zend_bool     code_coverage_dead_code_analysis;
	zend_bool     code_coverage_branch_check;
	unsigned int  function_count;
	int           dead_code_analysis_tracker_offset;
	long          dead_code_last_start_id;
	char                 *previous_filename;
	xdebug_coverage_file *previous_file;
	char                 *previous_mark_filename;
	xdebug_coverage_file *previous_mark_file;
	xdebug_path_info     *paths_stack;
	xdebug_hash          *visited_branches;
	struct {
		unsigned int  size;
		int *last_branch_nr;
	} branches;

	/* used for collection errors */
	zend_bool     do_collect_errors;
	xdebug_llist *collected_errors;

	/* used for function monitoring */
	zend_bool     do_monitor_functions;
	xdebug_hash  *functions_to_monitor;
	xdebug_llist *monitored_functions_found; /* List of functions found */

	/* superglobals */
	zend_bool     dump_globals;
	zend_bool     dump_once;
	zend_bool     dump_undefined;
	zend_bool     dumped;
	xdebug_llist  server;
	xdebug_llist  get;
	xdebug_llist  post;
	xdebug_llist  cookie;
	xdebug_llist  files;
	xdebug_llist  env;
	xdebug_llist  request;
	xdebug_llist  session;

	/* headers */
	xdebug_llist *headers;

	/* remote settings */
	zend_bool     remote_enable;  /* 0 */
	iniLONG       remote_port;    /* 9000 */
	char         *remote_host;    /* localhost */
	long          remote_mode;    /* XDEBUG_NONE, XDEBUG_JIT, XDEBUG_REQ */
	char         *remote_handler; /* php3, gdb, dbgp */
	zend_bool     remote_autostart; /* Disables the requirement for XDEBUG_SESSION_START */
	zend_bool     remote_connect_back;   /* connect back to the HTTP requestor */
	char         *remote_log;       /* Filename to log protocol communication to */
	FILE         *remote_log_file;  /* File handler for protocol log */
	iniLONG       remote_cookie_expire_time; /* Expire time for the remote-session cookie */
	char         *remote_addr_header; /* User configured header to check for forwarded IP address */

	char         *ide_key; /* As Xdebug uses it, from environment, USER, USERNAME or empty */
	char         *ide_key_setting; /* Set through php.ini and friends */

	/* remote debugging globals */
	zend_bool     remote_enabled;
	zend_bool     breakpoints_allowed;
	xdebug_con    context;
	unsigned int  breakpoint_count;
	unsigned int  no_exec;

	/* profiler settings */
	zend_bool     profiler_enable;
	char         *profiler_output_dir;
	char         *profiler_output_name; /* "pid" or "crc32" */
	zend_bool     profiler_enable_trigger;
	char         *profiler_enable_trigger_value;
	zend_bool     profiler_append;

	/* profiler globals */
	zend_bool     profiler_enabled;
	FILE         *profile_file;
	char         *profile_filename;
	xdebug_hash  *profile_filename_refs;
	int           profile_last_filename_ref;
	xdebug_hash  *profile_functionname_refs;
	int           profile_last_functionname_ref;

	/* DBGp globals */
	char         *lastcmd;
	char         *lasttransid;

	/* output redirection */
	int           stdout_mode;

	/* aggregate profiling */
	HashTable  aggr_calls;
	zend_bool  profiler_aggregate;

	/* scream */
	zend_bool  do_scream;
	zend_bool  in_at;

	/* in-execution checking */
	zend_bool  in_execution;
	zend_bool  in_var_serialisation;
ZEND_END_MODULE_GLOBALS(xdebug)

#ifdef ZTS
#define XG(v) TSRMG(xdebug_globals_id, zend_xdebug_globals *, v)
#else
#define XG(v) (xdebug_globals.v)
#endif

#endif


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
