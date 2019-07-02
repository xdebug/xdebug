#include "php_xdebug.h"

void function_stack_entry_dtor(void *dummy, void *elem);

extern HashTable sw_xdebug_globals;
static zend_function *get_cid_function = NULL;

void sw_xdebug_init()
{
	if (zend_hash_str_find_ptr(&module_registry, ZEND_STRL("swoole"))) {
		zend_string      *classname    = zend_string_init(ZEND_STRL("Swoole\\Coroutine"), 0);
		zend_class_entry *class_handle = zend_lookup_class(classname);
		zend_string_release(classname);

		get_cid_function = zend_hash_str_find_ptr(&(class_handle->function_table), ZEND_STRL("getcid"));
	}
}

long get_cid()
{
	zval retval;

	if (UNEXPECTED(!get_cid_function)) {
		return 0;
	}

	get_cid_function->internal_function.handler(NULL, &retval);

	return Z_LVAL(retval) == -1 ? 0 : Z_LVAL(retval);
}

int add_current_context()
{
	long cid = get_cid();
	zval pData;
	sw_zend_xdebug_globals *new_context;

	if (zend_hash_index_exists(&sw_xdebug_globals, cid)) {
		return 0;
	}

	new_context                = emalloc(sizeof(sw_zend_xdebug_globals));
	new_context->cid           = cid;
	new_context->level         = 0;
	new_context->stack         = xdebug_llist_alloc(function_stack_entry_dtor);
	new_context->prev_memory   = 0;
	new_context->paths_stack   = xdebug_path_info_ctor();
	new_context->branches.size = 0;
	new_context->branches.last_branch_nr = NULL;

	ZVAL_PTR(&pData, new_context);
	zend_hash_index_add(&sw_xdebug_globals, cid, &pData);

	return 1;
}

sw_zend_xdebug_globals *get_current_context()
{
	zval *val = zend_hash_index_find(&sw_xdebug_globals, get_cid());

	return (sw_zend_xdebug_globals *)Z_PTR_P(val);
}

void remove_context(long cid)
{
	sw_zend_xdebug_globals *context;
	zval *pData = zend_hash_index_find(&sw_xdebug_globals, cid);

	context = (sw_zend_xdebug_globals *)Z_PTR_P(pData);

	xdebug_llist_destroy(context->stack, NULL);

	if (context->paths_stack) {
		xdebug_path_info_dtor(context->paths_stack);
	}
	if (context->branches.last_branch_nr) {
		free(context->branches.last_branch_nr);
	}

	efree(context);

	zend_hash_index_del(&sw_xdebug_globals, cid);
}
