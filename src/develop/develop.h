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

#ifndef __XDEBUG_DEVELOP_H__
#define __XDEBUG_DEVELOP_H__

#define XDEBUG_LAST_EXCEPTION_TRACE_SLOTS 8

typedef struct _xdebug_develop_globals_t {
	/* used for function monitoring */
	zend_bool     do_monitor_functions;
	xdebug_hash  *functions_to_monitor;
	xdebug_llist *monitored_functions_found; /* List of functions found */

	/* superglobals */
	xdebug_llist  server;
	xdebug_llist  get;
	xdebug_llist  post;
	xdebug_llist  cookie;
	xdebug_llist  files;
	xdebug_llist  env;
	xdebug_llist  request;
	xdebug_llist  session;

	/* used for collection errors */
	xdebug_llist *collected_errors;

	/* scream */
	zend_bool  in_at;

	/* overloaded var_dump */
	zif_handler   orig_var_dump_func;

	/* last exception stack trace */
	struct {
		int          next_slot;
		zend_object *obj_ptr[XDEBUG_LAST_EXCEPTION_TRACE_SLOTS];
		zval         stack_trace[XDEBUG_LAST_EXCEPTION_TRACE_SLOTS];
	} last_exception_trace;
} xdebug_develop_globals_t;

typedef struct _xdebug_develop_settings_t {

	zend_long     max_stack_frames;
	zend_bool     show_ex_trace;
	zend_bool     show_error_trace;
	zend_bool     show_local_vars;
	zend_bool     force_display_errors;
	zend_long     force_error_reporting;
	zend_long     halt_level;

	zend_long     cli_color;

	/* superglobals */
	zend_bool     dump_globals;
	zend_bool     dump_once;
	zend_bool     dump_undefined;

	/* scream */
	zend_bool  do_scream;
} xdebug_develop_settings_t;

void xdebug_init_develop_globals(xdebug_develop_globals_t *xg);
void xdebug_deinit_develop_globals(xdebug_develop_globals_t *xg);

void xdebug_develop_minit();
void xdebug_develop_mshutdown();
void xdebug_develop_rinit();
void xdebug_develop_post_deactivate();
void xdebug_develop_rshutdown();

void xdebug_develop_throw_exception_hook(zend_object *exception, zval *file, zval *line, zval *code, char *code_str, zval *message);
void xdebug_monitor_handler(function_stack_entry *fse);

#endif
