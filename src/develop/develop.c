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
#include "php.h"
#include "zend_exceptions.h"

#include "php_xdebug.h"
#include "monitor.h"
#include "stack.h"
#include "superglobals.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static int xdebug_silence_handler(XDEBUG_OPCODE_HANDLER_ARGS);

static void xdebug_develop_overloaded_functions_setup(void)
{
	zend_function *orig;

	/* Override var_dump with our own function */
	orig = zend_hash_str_find_ptr(EG(function_table), "var_dump", sizeof("var_dump") - 1);
	XG_DEV(orig_var_dump_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_xdebug_var_dump;
}

static void xdebug_develop_overloaded_functions_restore(void)
{
	zend_function *orig;

	orig = zend_hash_str_find_ptr(EG(function_table), "var_dump", sizeof("var_dump") - 1);
	orig->internal_function.handler = XG_DEV(orig_var_dump_func);
}


void xdebug_init_develop_globals(xdebug_develop_globals_t *xg)
{
	xg->do_monitor_functions = 0;

	xg->in_at                = 0; /* scream */

	xdebug_llist_init(&xg->server, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->get, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->post, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->cookie, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->files, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->env, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->request, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->session, xdebug_superglobals_dump_dtor);
}

void xdebug_deinit_develop_globals(xdebug_develop_globals_t *xg)
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

void xdebug_develop_minit(INIT_FUNC_ARGS)
{
	/* Overload opcodes for 'scream' */
	xdebug_set_opcode_handler(ZEND_BEGIN_SILENCE, xdebug_silence_handler);
	xdebug_set_opcode_handler(ZEND_END_SILENCE, xdebug_silence_handler);

	REGISTER_LONG_CONSTANT("XDEBUG_STACK_NO_DESC", XDEBUG_STACK_NO_DESC, CONST_CS | CONST_PERSISTENT);
}

void xdebug_develop_mshutdown()
{
}

void xdebug_develop_rinit()
{
	XG_DEV(collected_errors)  = xdebug_llist_alloc(xdebug_llist_string_dtor);

	/* Function monitoring */
	XG_DEV(do_monitor_functions) = 0;
	XG_DEV(functions_to_monitor) = NULL;
	XG_DEV(monitored_functions_found) = xdebug_llist_alloc(xdebug_monitored_function_dtor);

	xdebug_develop_overloaded_functions_setup();
}

void xdebug_develop_post_deactivate()
{
	xdebug_llist_destroy(XG_DEV(collected_errors), NULL);
	XG_DEV(collected_errors) = NULL;

	xdebug_llist_destroy(XG_DEV(monitored_functions_found), NULL);
	XG_DEV(monitored_functions_found) = NULL;

	if (XG_DEV(functions_to_monitor)) {
		xdebug_hash_destroy(XG_DEV(functions_to_monitor));
		XG_DEV(functions_to_monitor) = NULL;
	}

	xdebug_develop_overloaded_functions_restore();
}


static int xdebug_silence_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	const zend_op *cur_opcode = execute_data->opline;

	xdebug_coverage_record_if_active(execute_data, op_array);

	if (XINI_DEV(do_scream)) {
		execute_data->opline++;
		if (cur_opcode->opcode == ZEND_BEGIN_SILENCE) {
			XG_DEV(in_at) = 1;
		} else {
			XG_DEV(in_at) = 0;
		}
		return ZEND_USER_OPCODE_CONTINUE;
	}

	return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}
