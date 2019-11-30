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

#include "php_xdebug.h"
#include "private.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

const char *xdebug_log_prefix[11] = {
	"", "E: ", "", "W: ", "", "", "", "I: ", "", "", "D: "
};

function_stack_entry *xdebug_get_stack_head(void)
{
	xdebug_llist_element *le;

	if (XG_BASE(stack)) {
		if ((le = XDEBUG_LLIST_HEAD(XG_BASE(stack)))) {
			return XDEBUG_LLIST_VALP(le);
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

function_stack_entry *xdebug_get_stack_frame(int nr)
{
	xdebug_llist_element *le;

	if (!XG_BASE(stack)) {
		return NULL;
	}

	if (!(le = XDEBUG_LLIST_TAIL(XG_BASE(stack)))) {
		return NULL;
	}

	if (nr < 0) {
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

function_stack_entry *xdebug_get_stack_tail(void)
{
	xdebug_llist_element *le;

	if (XG_BASE(stack)) {
		if ((le = XDEBUG_LLIST_TAIL(XG_BASE(stack)))) {
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
	xdebug_str *var_name = (xdebug_str*) data;

	xdebug_str_free(var_name);
}

static int xdebug_compare_le_xdebug_str(const void *le1, const void *le2)
{
	return strcmp(
		((xdebug_str *) XDEBUG_LLIST_VALP(*(xdebug_llist_element **) le1))->d,
		((xdebug_str *) XDEBUG_LLIST_VALP(*(xdebug_llist_element **) le2))->d
	);
}

xdebug_hash* xdebug_declared_var_hash_from_llist(xdebug_llist *list)
{
	xdebug_hash          *tmp;
	xdebug_llist_element *le;
	xdebug_str           *var_name;

	tmp = xdebug_hash_alloc_with_sort(32, xdebug_used_var_hash_from_llist_dtor, xdebug_compare_le_xdebug_str);
	for (le = XDEBUG_LLIST_HEAD(list); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
		var_name = (xdebug_str*) XDEBUG_LLIST_VALP(le);
		xdebug_hash_add(tmp, var_name->d, var_name->l, xdebug_str_copy(var_name));
	}

	return tmp;
}

int xdebug_trigger_enabled(int setting, const char *var_name, char *var_value)
{
	zval *trigger_val;

	if (!setting) {
		return 0;
	}

	if (
		(
			(
				(trigger_val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_GET]), var_name, strlen(var_name))) != NULL
			) || (
				(trigger_val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_POST]), var_name, strlen(var_name))) != NULL
			) || (
				(trigger_val = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_COOKIE]), var_name, strlen(var_name))) != NULL
			)
		) && (
			(var_value == NULL) || (var_value[0] == '\0') ||
			(strcmp(var_value, Z_STRVAL_P(trigger_val)) == 0)
		)
	) {
		return 1;
	}

	return 0;
}
