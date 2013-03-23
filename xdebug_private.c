/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
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
#include "xdebug_private.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

function_stack_entry *xdebug_get_stack_head(TSRMLS_D)
{
	xdebug_llist_element *le;

	if (XG(stack)) {
		if ((le = XDEBUG_LLIST_HEAD(XG(stack)))) {
			return XDEBUG_LLIST_VALP(le);
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

function_stack_entry *xdebug_get_stack_frame(int nr TSRMLS_DC)
{
	xdebug_llist_element *le;

	if (!XG(stack)) {
		return NULL;
	}

	if (!(le = XDEBUG_LLIST_TAIL(XG(stack)))) {
		return NULL;
	}

	while (nr) {
		nr--;
		le = XDEBUG_LLIST_PREV(le);
		if (!le) {
			return NULL;
		}
	}
	return XDEBUG_LLIST_VALP(le);
}

function_stack_entry *xdebug_get_stack_tail(TSRMLS_D)
{
	xdebug_llist_element *le;

	if (XG(stack)) {
		if ((le = XDEBUG_LLIST_TAIL(XG(stack)))) {
			return XDEBUG_LLIST_VALP(le);
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

static void xdebug_used_var_hash_from_llist_dtor(void *data)
{
	/* We are not freeing anything as the list creating didn't copy the data */
}

xdebug_hash* xdebug_used_var_hash_from_llist(xdebug_llist *list)
{
	xdebug_hash *tmp;
	xdebug_llist_element *le;
	char *var_name;

	tmp = xdebug_hash_alloc(32, xdebug_used_var_hash_from_llist_dtor);
	for (le = XDEBUG_LLIST_HEAD(list); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
		var_name = (char*) XDEBUG_LLIST_VALP(le);
		xdebug_hash_add(tmp, var_name, strlen(var_name), var_name);
	}

	return tmp;
}
