/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 Derick Rethans                              |
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
#define XDEBUG_VERSION    "2.0.0dev"
#define XDEBUG_AUTHOR     "Derick Rethans"
#define XDEBUG_COPYRIGHT  "Copyright (c) 2002, 2003 by Derick Rethans"
#define XDEBUG_URL        "http://xdebug.org"

#include "php.h"

#include "xdebug_handlers.h"
#include "xdebug_hash.h"
#include "xdebug_llist.h"

extern zend_module_entry xdebug_module_entry;
#define phpext_xdebug_ptr &xdebug_module_entry

#define MICRO_IN_SEC 1000000.00

#ifdef PHP_WIN32
#define PHP_XDEBUG_API __declspec(dllexport)
#else
#define PHP_XDEBUG_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(xdebug);
PHP_MSHUTDOWN_FUNCTION(xdebug);
PHP_RINIT_FUNCTION(xdebug);
PHP_RSHUTDOWN_FUNCTION(xdebug);
PHP_MINFO_FUNCTION(xdebug);
#ifdef ZEND_ENGINE_2
ZEND_MODULE_EXEC_FINISHED_D(xdebug);
#endif

/* call stack functions */
PHP_FUNCTION(xdebug_get_function_stack);
PHP_FUNCTION(xdebug_call_class);
PHP_FUNCTION(xdebug_call_function);
PHP_FUNCTION(xdebug_call_file);
PHP_FUNCTION(xdebug_call_line);

PHP_FUNCTION(xdebug_var_dump);

/* activation functions */
PHP_FUNCTION(xdebug_enable);
PHP_FUNCTION(xdebug_disable);
PHP_FUNCTION(xdebug_is_enabled);

/* tracing functions */
PHP_FUNCTION(xdebug_start_trace);
PHP_FUNCTION(xdebug_stop_trace);
PHP_FUNCTION(xdebug_get_function_trace);
PHP_FUNCTION(xdebug_dump_function_trace);

/* misc functions */
PHP_FUNCTION(xdebug_dump_superglobals);
PHP_FUNCTION(xdebug_set_error_handler);
#if MEMORY_LIMIT
PHP_FUNCTION(xdebug_memory_usage);
#endif
PHP_FUNCTION(xdebug_time_index);


void xdebug_start_trace();
void xdebug_stop_trace();

typedef struct xdebug_var {
	char *name;
	char *value;
	void *addr;
} xdebug_var;

#define XFUNC_UNKNOWN        0x00
#define XFUNC_NORMAL         0x01
#define XFUNC_STATIC_MEMBER  0x02
#define XFUNC_MEMBER         0x03
#define XFUNC_NEW            0x04

#define XFUNC_INCLUDES       0x10
#define XFUNC_EVAL           0x10
#define XFUNC_INCLUDE        0x11
#define XFUNC_INCLUDE_ONCE   0x12
#define XFUNC_REQUIRE        0x13
#define XFUNC_REQUIRE_ONCE   0x14

#define XDEBUG_IS_FUNCTION(f) (f == XFUNC_NORMAL || f == XFUNC_STATIC_MEMBER || f == XFUNC_MEMBER)

#define XDEBUG_REGISTER_LONG_CONSTANT(__c) REGISTER_LONG_CONSTANT(#__c, __c, CONST_CS|CONST_PERSISTENT)

#define XDEBUG_NONE          0
#define XDEBUG_JIT           1
#define XDEBUG_REQ           2

#define XDEBUG_BREAK         1
#define XDEBUG_STEP          2

#define XDEBUG_INTERNAL      1
#define XDEBUG_EXTERNAL      2

#define XDEBUG_MAX_FUNCTION_LEN 1024

#define STATUS_STARTING   0
#define STATUS_STOPPING   1
#define STATUS_STOPPED    2
#define STATUS_RUNNING    3
#define STATUS_BREAK      4

#define REASON_OK         0
#define REASON_ERROR      1
#define REASON_ABORTED    2
#define REASON_EXCEPTION  3

#define XDEBUG_ERROR_OK                              0
#define XDEBUG_ERROR_PARSE                           1
#define XDEBUG_ERROR_DUP_ARG                         2
#define XDEBUG_ERROR_INVALID_ARGS                    3

#define XDEBUG_ERROR_CANT_OPEN_FILE                100

#define XDEBUG_ERROR_BREAKPOINT_NOT_SET            200
#define XDEBUG_ERROR_BREAKPOINT_TYPE_NOT_SUPPORTED 201
#define XDEBUG_ERROR_BREAKPOINT_INVALID            202
#define XDEBUG_ERROR_BREAKPOINT_NO_CODE            203
#define XDEBUG_ERROR_NO_SUCH_BREAKPOINT            204
#define XDEBUG_ERROR_EVALUATING_CODE               205

#define XDEBUG_ERROR_PROPERTY_NON_EXISTANT         300
#define XDEBUG_ERROR_STACK_DEPTH_TOO_HIGH          301
#define XDEBUG_ERROR_STACK_DEPTH_INVALID           302

#define XDEBUG_ERROR_ENCODING_NOT_SUPPORTED        900
#define XDEBUG_ERROR_UNIMPLEMENTED                 999

typedef struct xdebug_func {
	char *class;
	char *function;
	int   type;
	int   internal;
} xdebug_func;

typedef struct function_stack_entry {
	/* function properties */
	xdebug_func  function;
	int          user_defined;

	/* location properties */
	int          level;
	char        *filename;
	int          lineno;

	/* argument properties */
	int          arg_done;
	int          varc;
	xdebug_var   vars[20];
	xdebug_hash *used_vars;
	HashTable   *symbol_table;

	/* profiling properties */
	unsigned int memory;
	double       time;
	double       time_taken;	
	unsigned int f_calls;

	/* misc properties */
	int          refcount;
} function_stack_entry;

ZEND_BEGIN_MODULE_GLOBALS(xdebug)
	int           status;
	int           reason;

	int           level;
	xdebug_llist *stack;
	xdebug_llist *trace;
	int           max_nesting_level;
	zend_bool     default_enable;
	zend_bool     collect_params;
	zend_bool     auto_trace;
	zend_bool     do_trace;
	char         *manual_url;
	FILE         *trace_file;
	char         *error_handler;
	double        start_time;
	HashTable    *active_symbol_table;

	/* used for code coverage */
	zend_bool     do_code_coverage;
	xdebug_hash  *code_coverage;

	/* used for profiling */
	double 	      total_execution_time;
	double 	      total_compiling_time;
	zend_bool     do_profile;
	zend_bool     profiler_trace;
	FILE         *profile_file;
	zend_bool     auto_profile;
	char         *output_dir;
	int           auto_profile_mode;

	/* superglobals */
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

	/* remote settings */
	zend_bool     remote_enable;  /* 0 */
	int           remote_port;    /* 17869 */
	char         *remote_host;    /* localhost */
	int           remote_mode;    /* XDEBUG_NONE, XDEBUG_JIT, XDEBUG_REQ */
	char         *remote_handler; /* php3, gdb, dbgp */

	/* remote debugging globals */
	zend_bool     remote_enabled;
	zend_bool     breakpoints_allowed;
	xdebug_con    context;
	unsigned int  breakpoint_count;
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
