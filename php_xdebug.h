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

/* breaking functions */
PHP_FUNCTION(xdebug_break);

/* tracing functions */
PHP_FUNCTION(xdebug_start_trace);
PHP_FUNCTION(xdebug_stop_trace);

/* misc functions */
PHP_FUNCTION(xdebug_dump_superglobals);
PHP_FUNCTION(xdebug_set_error_handler);
#if MEMORY_LIMIT
PHP_FUNCTION(xdebug_memory_usage);
PHP_FUNCTION(xdebug_peak_memory_usage);
#endif
PHP_FUNCTION(xdebug_time_index);

ZEND_BEGIN_MODULE_GLOBALS(xdebug)
	int           status;
	int           reason;

	int           level;
	xdebug_llist *stack;
	int           max_nesting_level;
	zend_bool     default_enable;
	zend_bool     collect_params;
	zend_bool     extended_info;
	zend_bool     show_local_vars;
	char         *manual_url;
	char         *error_handler;
	double        start_time;
	HashTable    *active_symbol_table;

	FILE         *trace_file;
	zend_bool     do_trace;
	zend_bool     auto_trace;
	char         *auto_trace_file;

	/* used for code coverage */
	zend_bool     do_code_coverage;
	xdebug_hash  *code_coverage;

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

	/* remote settings */
	zend_bool     remote_enable;  /* 0 */
	int           remote_port;    /* 9000 */
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
