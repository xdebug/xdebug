/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2015 Derick Rethans                               |
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

#include "php_xdebug.h"

#include "xdebug_hash.h"
#include "xdebug_monitor.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static void init_function_monitor_hash(xdebug_hash *internal, HashTable *functions_to_monitor)
{
	HashPosition  pos;
	zval        **val;

	zend_hash_internal_pointer_reset_ex(functions_to_monitor, &pos);
	while (zend_hash_get_current_data_ex(functions_to_monitor, (void **) &val, &pos) != FAILURE) {
		if (Z_TYPE_PP(val) == IS_STRING) {
			xdebug_hash_add(internal, Z_STRVAL_PP(val), Z_STRLEN_PP(val), xdstrdup(Z_STRVAL_PP(val)));
		}

		zend_hash_move_forward_ex(functions_to_monitor, &pos);
	}
}

static void xdebug_hash_function_monitor_dtor(char *function)
{
	xdfree(function);
}

static xdebug_monitored_function_entry *xdebug_monitored_function_init(char *func_name, char *filename, int lineno)
{
	xdebug_monitored_function_entry *tmp = xdmalloc(sizeof(xdebug_monitored_function_dtor));

	tmp->func_name = xdstrdup(func_name);
	tmp->filename = xdstrdup(filename);
	tmp->lineno = lineno;

	return tmp;
}

void xdebug_monitored_function_dtor(void *dummy, void *elem)
{
	xdebug_monitored_function_entry *mfe = (xdebug_monitored_function_entry*) elem;

	xdfree(mfe->func_name);
	xdfree(mfe->filename);
	xdfree(mfe);
}

void xdebug_function_monitor_record(char *func_name, char *filename, int lineno)
{
	xdebug_monitored_function_entry *record;

	record = xdebug_monitored_function_init(func_name, filename, lineno);
	xdebug_llist_insert_next(XG(monitored_functions_found), XDEBUG_LLIST_TAIL(XG(monitored_functions_found)), record);
}

PHP_FUNCTION(xdebug_start_function_monitor)
{
	HashTable *functions_to_monitor;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "H", &functions_to_monitor) == FAILURE) {
		return;
	}

	if (XG(do_monitor_functions) == 1) {
		php_error(E_NOTICE, "Function monitoring was already started");
	}

	/* Clean and store list of functions to monitor */
	if (XG(functions_to_monitor)) {
		xdebug_hash_destroy(XG(functions_to_monitor));
	}
	XG(functions_to_monitor) = xdebug_hash_alloc(zend_hash_num_elements(functions_to_monitor), (xdebug_hash_dtor) xdebug_hash_function_monitor_dtor);
	init_function_monitor_hash(XG(functions_to_monitor), functions_to_monitor);

	XG(do_monitor_functions) = 1;
}

PHP_FUNCTION(xdebug_stop_function_monitor)
{
	if (XG(do_monitor_functions) == 0) {
		php_error(E_NOTICE, "Function monitoring was not started");
	}
	XG(do_monitor_functions) = 0;
}

PHP_FUNCTION(xdebug_get_monitored_functions)
{
	xdebug_llist_element *le;
	zend_bool             clear = 0;
	xdebug_monitored_function_entry *mfe;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &clear) == FAILURE) {
		return;
	}

	array_init(return_value);
	for (le = XDEBUG_LLIST_HEAD(XG(monitored_functions_found)); le != NULL; le = XDEBUG_LLIST_NEXT(le))	{
		mfe = XDEBUG_LLIST_VALP(le);
		add_next_index_string(return_value, mfe->func_name, 1);
	}

	if (clear) {
		xdebug_llist_destroy(XG(monitored_functions_found), NULL);
		XG(monitored_functions_found) = xdebug_llist_alloc(xdebug_monitored_function_dtor);
	}
}
/* }}} */
