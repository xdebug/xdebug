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

#include "lib_private.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

const char *xdebug_log_prefix[11] = {
	"", "E: ", "", "W: ", "", "", "", "I: ", "", "", "D: "
};


void xdebug_init_library_globals(xdebug_library_globals_t *xg)
{
	xg->active_execute_data  = NULL;
	xg->opcode_handlers_set = xdebug_set_create(256);
	memset(xg->original_opcode_handlers, 0, sizeof(xg->original_opcode_handlers));
}

void xdebug_library_minit(void)
{
}

void xdebug_library_mshutdown(void)
{
	int i;

	/* Restore all opcode handlers that we have set */
	for (i = 0; i < 256; i++) {
		xdebug_unset_opcode_handler(i);
	}

	xdebug_set_free(XG_LIB(opcode_handlers_set));
}


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


void xdebug_lib_set_active_data(zend_execute_data *execute_data)
{
	XG_LIB(active_execute_data) = execute_data;
}

void xdebug_lib_set_active_object(zval *object)
{
	XG_LIB(active_object) = object;
}

void xdebug_lib_set_active_stack_entry(function_stack_entry *fse)
{
	XG_LIB(active_stack_entry) = fse;
}

void xdebug_lib_set_active_symbol_table(HashTable *symbol_table)
{
	XG_LIB(active_symbol_table) = symbol_table;
}

int xdebug_lib_has_active_data(void)
{
	return !!XG_LIB(active_execute_data);
}

int xdebug_lib_has_active_function(void)
{
	return !!XG_LIB(active_execute_data)->func;
}

int xdebug_lib_has_active_object(void)
{
	return !!XG_LIB(active_object);
}

int xdebug_lib_has_active_symbol_table(void)
{
	return !!XG_LIB(active_symbol_table);
}

zend_execute_data *xdebug_lib_get_active_data(void)
{
	return XG_LIB(active_execute_data);
}

zend_op_array *xdebug_lib_get_active_func_oparray(void)
{
	return &XG_LIB(active_execute_data)->func->op_array;
}

function_stack_entry *xdebug_lib_get_active_stack_entry(void)
{
	return XG_LIB(active_stack_entry);
}

HashTable *xdebug_lib_get_active_symbol_table(void)
{
	return XG_LIB(active_symbol_table);
}

zval *xdebug_lib_get_active_object(void)
{
	return XG_LIB(active_object);
}

int xdebug_isset_opcode_handler(int opcode)
{
	return xdebug_set_in(XG_LIB(opcode_handlers_set), opcode);
}

void xdebug_set_opcode_handler(int opcode, user_opcode_handler_t handler)
{
	if (xdebug_isset_opcode_handler(opcode)) {
		abort();
	}
	XG_LIB(original_opcode_handlers[opcode]) = zend_get_user_opcode_handler(opcode);
	xdebug_set_add(XG_LIB(opcode_handlers_set), opcode);
	zend_set_user_opcode_handler(opcode, handler);
}

void xdebug_unset_opcode_handler(int opcode)
{
	if (xdebug_set_in(XG_LIB(opcode_handlers_set), opcode)) {
		zend_set_user_opcode_handler(opcode, XG_LIB(original_opcode_handlers[opcode]));
	}
}

int xdebug_call_original_opcode_handler_if_set(int opcode, XDEBUG_OPCODE_HANDLER_ARGS)
{
	if (xdebug_isset_opcode_handler(opcode)) {
		user_opcode_handler_t handler = XG_LIB(original_opcode_handlers[opcode]);

		if (handler) {
			return handler(XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
		}
	}

	return ZEND_USER_OPCODE_DISPATCH;
}
