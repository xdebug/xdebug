/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000, 2001 The PHP Group             |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@vl-srm.net>                         |
   +----------------------------------------------------------------------+
 */

#ifndef PHP_XDEBUG_H
#define PHP_XDEBUG_H

#include "php.h"

#include "xdebug_llist.h"

extern zend_module_entry xdebug_module_entry;
#define phpext_xdebug_ptr &xdebug_module_entry

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


PHP_FUNCTION(xdebug_get_function_stack);
PHP_FUNCTION(xdebug_call_function);
PHP_FUNCTION(xdebug_call_file);
PHP_FUNCTION(xdebug_call_line);

PHP_FUNCTION(xdebug_enable);
PHP_FUNCTION(xdebug_disable);
PHP_FUNCTION(xdebug_is_enabled);

PHP_FUNCTION(xdebug_start_trace);
PHP_FUNCTION(xdebug_stop_trace);
PHP_FUNCTION(xdebug_get_function_trace);
PHP_FUNCTION(xdebug_dump_function_trace);

#if MEMORY_LIMIT
PHP_FUNCTION(xdebug_memory_usage);
#endif

typedef struct xdebug_var {
	char *name;
	char *value;
} xdebug_var;

#define XFUNC_UNKNOWN        0
#define XFUNC_NORMAL         1
#define XFUNC_STATIC_MEMBER  2
#define XFUNC_MEMBER         3
#define XFUNC_NEW            4
#define XFUNC_EVAL           5
#define XFUNC_INCLUDE        6
#define XFUNC_INCLUDE_ONCE   7
#define XFUNC_REQUIRE        8
#define XFUNC_REQUIRE_ONCE   9

#define XFUNC_SET(e,t,c,f)          (e)->function.type = t; (e)->function.class = estrdup (c); (e)->function.function = estrdup (f);
#define XFUNC_SET_DELAYED_F(e,t,c)  (e)->function.type = t; (e)->function.class = estrdup (c); (e)->delayed_fname = 1;
#define XFUNC_SET_DELAYED_C(e,t,f)  (e)->function.type = t; (e)->function.function = estrdup (f); (e)->delayed_cname = 1;

typedef struct xdebug_func {
	char *class;
	char *function;
	int   type;
	int   internal;
} xdebug_func;

typedef struct function_stack_entry {
	xdebug_func  function;

	char *filename;
	int   lineno;

	int   arg_done;
	int   varc;
	xdebug_var vars[20];

	int   delayed_fname;
	int   delayed_cname;
	int   delayed_include;

	unsigned int memory;
	double       time;

	int   level;
	int   refcount;
} function_stack_entry;


ZEND_BEGIN_MODULE_GLOBALS(xdebug)
	int           level;
	xdebug_llist *stack;
	xdebug_llist *trace;
	int           max_nesting_level;
	zend_bool     default_enable;
	zend_bool     do_trace;
	char         *manual_url;
	FILE         *trace_file;
ZEND_END_MODULE_GLOBALS(xdebug)


#ifdef ZTS
#define XG(v) TSRMG(xdebug_globals_id, zend_xdebug_globals *, v)
#else
#define XG(v) (xdebug_globals.v)
#endif

/* Memory allocators */
#if 0
#define xdmalloc    emalloc
#define xdcalloc    ecalloc	
#define xdrealloc   erealloc
#else  
#define xdmalloc    malloc
#define xdcalloc    calloc	
#define xdrealloc   realloc
#endif
	
#endif


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
