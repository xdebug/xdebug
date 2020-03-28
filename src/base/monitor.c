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

#include "php_xdebug.h"

#include "monitor.h"

#include "lib/compat.h"
#include "lib/hash.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static void init_function_monitor_hash(xdebug_hash *internal, HashTable *functions_to_monitor)
{
	zval *val;

	ZEND_HASH_FOREACH_VAL(functions_to_monitor, val) {
		if (Z_TYPE_P(val) == IS_STRING) {
			xdebug_hash_add(internal, Z_STRVAL_P(val), Z_STRLEN_P(val), xdstrdup(Z_STRVAL_P(val)));
		}
	} ZEND_HASH_FOREACH_END();
}

static void xdebug_hash_function_monitor_dtor(char *function)
{
	xdfree(function);
}

static xdebug_monitored_function_entry *xdebug_monitored_function_init(char *func_name, char *filename, int lineno)
{
	xdebug_monitored_function_entry *tmp = xdmalloc(sizeof(xdebug_monitored_function_entry));

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
	xdebug_llist_insert_next(XG_BASE(monitored_functions_found), XDEBUG_LLIST_TAIL(XG_BASE(monitored_functions_found)), record);
}

PHP_FUNCTION(xdebug_start_function_monitor)
{
	HashTable *functions_to_monitor;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "H", &functions_to_monitor) == FAILURE) {
		return;
	}

	if (XG_BASE(do_monitor_functions) == 1) {
		php_error(E_NOTICE, "Function monitoring was already started");
	}

	/* Clean and store list of functions to monitor */
	if (XG_BASE(functions_to_monitor)) {
		xdebug_hash_destroy(XG_BASE(functions_to_monitor));
	}

	/* We add "1" here so that we don't alloc a 0-slot hash table */
	XG_BASE(functions_to_monitor) = xdebug_hash_alloc(zend_hash_num_elements(functions_to_monitor) + 1, (xdebug_hash_dtor_t) xdebug_hash_function_monitor_dtor);
	init_function_monitor_hash(XG_BASE(functions_to_monitor), functions_to_monitor);

	XG_BASE(do_monitor_functions) = 1;
}

PHP_FUNCTION(xdebug_stop_function_monitor)
{
	if (XG_BASE(do_monitor_functions) == 0) {
		php_error(E_NOTICE, "Function monitoring was not started");
	}
	XG_BASE(do_monitor_functions) = 0;
}

PHP_FUNCTION(xdebug_get_monitored_functions)
{
	xdebug_llist_element *le;
	zend_bool             clear = 0;
	xdebug_monitored_function_entry *mfe;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &clear) == FAILURE) {
		return;
	}

	array_init(return_value);
	for (le = XDEBUG_LLIST_HEAD(XG_BASE(monitored_functions_found)); le != NULL; le = XDEBUG_LLIST_NEXT(le))	{
		zval *entry;

		mfe = XDEBUG_LLIST_VALP(le);

		XDEBUG_MAKE_STD_ZVAL(entry);
		array_init(entry);

		add_assoc_string_ex(entry, "function", HASH_KEY_SIZEOF("function"), mfe->func_name);
		add_assoc_string_ex(entry, "filename", HASH_KEY_SIZEOF("filename"), mfe->filename);
		add_assoc_long(entry, "lineno", mfe->lineno);

		add_next_index_zval(return_value, entry);
		efree(entry);
	}

	if (clear) {
		xdebug_llist_destroy(XG_BASE(monitored_functions_found), NULL);
		XG_BASE(monitored_functions_found) = xdebug_llist_alloc(xdebug_monitored_function_dtor);
	}
}
/* }}} */
