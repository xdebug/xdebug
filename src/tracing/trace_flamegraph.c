/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2021 Derick Rethans                               |
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
#include "php.h"
#include "ext/standard/php_string.h"

#include "php_xdebug.h"
#include "tracing_private.h"
#include "trace_flamegraph.h"

#include "lib/lib_private.h"
#include "lib/var_export_line.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

static flamegraph_function *fg_function_ctor()
{
	flamegraph_function *ret;
	ret = xdmalloc(sizeof(flamegraph_function));
	ret->value = 0;
	ret->prefix = NULL;
	return ret;
}

static inline void fg_function_dtor(flamegraph_function *function)
{
	if (NULL != function->prefix) {
		xdfree(function->prefix);
	}
	xdfree(function);
}

static inline xdebug_str *fg_function_key(const int function_nr) {
	xdebug_str *key = xdebug_str_new();

	xdebug_str_add_fmt(key, "fun-%d", function_nr);

	return key;
}

static inline void fg_function_add(const xdebug_trace_flamegraph_context *context, const int function_nr, const flamegraph_function *function) {
	xdebug_str *key = fg_function_key(function_nr);

	/* @todo This call always segfault badly. */
	xdebug_hash_add(context->functions, key->d, key->l, function);

	xdebug_str_free(key);
}

static inline flamegraph_function *fg_function_find(const xdebug_trace_flamegraph_context *context, const int function_nr) {
	flamegraph_function *function;
	xdebug_str          *key = fg_function_key(function_nr);

	xdebug_hash_find(context->functions, key->d, key->l, (void*) function);

	xdebug_str_free(key);

	return function;
}

static inline flamegraph_function *fg_function_delete(const xdebug_trace_flamegraph_context *context, const int function_nr) {
	flamegraph_function *function;
	xdebug_str          *key = fg_function_key(function_nr);

	xdebug_hash_delete(context->functions, key->d, key->l);

	xdebug_str_free(key);

	return function;
}

/* Find parent function in xdebug stack, which is Fiber-safe. */
static inline function_stack_entry *fg_parent_find() {
	function_stack_entry *parent_fse;
	int                   parent_index = XDEBUG_VECTOR_COUNT(XG_BASE(stack)) - 1;

	parent_fse = xdebug_vector_element_get(XG_BASE(stack), parent_index);

	return parent_fse;
}

static inline int compute_inclusive_value(const xdebug_trace_flamegraph_context *context, const function_stack_entry *fse)
{
	int value = 0, current_mem;

	switch (context->mode) {
		case XDEBUG_TRACE_OPTION_FLAMEGRAPH_MEM:
			/* We compare with 'memory' because 'prev_memory' is not memory when
			   starting the function execution, 'memory' is. */
			current_mem = zend_memory_usage(0);
			if (current_mem < fse->memory) {
				/* When memory is below 0, flamegraph generator will error, and
				   you won't have a good visual.
				   This happens when garbage collection happened during this
				   function and freed something it didn't allocate, I guess. */
				value = 0;
			} else {
				value = current_mem - fse->memory;
			}
			break;

		case XDEBUG_TRACE_OPTION_FLAMEGRAPH_COST:
			value = xdebug_get_nanotime() - fse->nanotime;
			break;
	}

	return value;
}

xdebug_trace_flamegraph_context *xdebug_trace_flamegraph_init(char *fname, zend_string *script_filename, long options)
{
	xdebug_trace_flamegraph_context *tmp_flamegraph_context;
	char *used_fname;

	tmp_flamegraph_context = xdmalloc(sizeof(xdebug_trace_flamegraph_context));
	tmp_flamegraph_context->trace_file = xdebug_trace_open_file(fname, script_filename, options, (char**) &used_fname);
	tmp_flamegraph_context->trace_filename = used_fname;

	return tmp_flamegraph_context->trace_file ? tmp_flamegraph_context : NULL;
}

void *xdebug_trace_flamegraph_init_cost(char *fname, zend_string *script_filename, long options)
{
	xdebug_trace_flamegraph_context *tmp_flamegraph_context = xdebug_trace_flamegraph_init(fname, script_filename, options);

	if (tmp_flamegraph_context) {
		tmp_flamegraph_context->mode = XDEBUG_TRACE_OPTION_FLAMEGRAPH_COST;
		tmp_flamegraph_context->functions = xdebug_hash_alloc(64, (xdebug_hash_dtor_t) fg_function_dtor);
	}

	return tmp_flamegraph_context;
}

void *xdebug_trace_flamegraph_init_mem(char *fname, zend_string *script_filename, long options)
{
	xdebug_trace_flamegraph_context *tmp_flamegraph_context = xdebug_trace_flamegraph_init(fname, script_filename, options);

	if (tmp_flamegraph_context) {
		tmp_flamegraph_context->mode = XDEBUG_TRACE_OPTION_FLAMEGRAPH_MEM;
		tmp_flamegraph_context->functions = xdebug_hash_alloc(64, (xdebug_hash_dtor_t) fg_function_dtor);
	}

	return tmp_flamegraph_context;
}

void xdebug_trace_flamegraph_deinit(void *ctxt)
{
	xdebug_trace_flamegraph_context *context = (xdebug_trace_flamegraph_context*) ctxt;

	if (NULL == context) {
		return;
	}

	if (NULL != context->functions) {
		xdebug_hash_destroy(context->functions);
		context->functions = NULL;
	}

	if (NULL != context->trace_file) {
		fclose(context->trace_file);
		context->trace_file = NULL;
		xdfree(context->trace_filename);
		context->trace_filename = NULL;
	}

	xdfree(context);
}

void xdebug_trace_flamegraph_write_header(void *ctxt)
{
}

void xdebug_trace_flamegraph_write_footer(void *ctxt)
{
}

char *xdebug_trace_flamegraph_get_filename(void *ctxt)
{
	xdebug_trace_flamegraph_context *context = (xdebug_trace_flamegraph_context*) ctxt;

	return context->trace_filename;
}

void xdebug_trace_flamegraph_function_entry(void *ctxt, function_stack_entry *fse, int function_nr)
{
	xdebug_trace_flamegraph_context *context = (xdebug_trace_flamegraph_context*) ctxt;
	function_stack_entry            *parent_fse;
	flamegraph_function             *function;
	flamegraph_function             *parent_function;
	xdebug_str                      *key;
	xdebug_str                      *prefix = xdebug_str_new();
	char                            *tmp_name;

	tmp_name = xdebug_show_fname(fse->function, 0, 0);
	function = fg_function_ctor();

	parent_fse = fg_parent_find();

	if (NULL == parent_fse) {
		/* No parent means we are top-level, prefix is function name. */
		xdebug_str_add_fmt(prefix, tmp_name);
	} else {
		/* Find value in our custom hashmap in order to compute prefix. */
		parent_function = fg_function_find(context, fse->function_nr);
		if (NULL == parent_function) {
			/* No function found is a bug, we should have one.
			   treat it as it was a top-level function. */
			xdebug_str_add_fmt(prefix, tmp_name);
		} else {
			xdebug_str_add_fmt(prefix, "%s;%s", parent_function->prefix->d, tmp_name);
		}
	}

	function->prefix = prefix;

	fg_function_add(context, fse->function_nr, function);

	xdfree(tmp_name);
}

void xdebug_trace_flamegraph_function_exit(void *ctxt, function_stack_entry *fse, int function_nr)
{
	xdebug_trace_flamegraph_context *context = (xdebug_trace_flamegraph_context*) ctxt;
	flamegraph_function             *function;
	flamegraph_function             *parent_function;
	function_stack_entry			*parent_fse;
	xdebug_str                       str = XDEBUG_STR_INITIALIZER;
	int                              inclusive;
	int                              self;

	function = fg_function_find(context, fse->function_nr);

	if (NULL == function) {
		/* This should never happen, better be safe than sorry. */
		return;
	}

	inclusive = compute_inclusive_value(context, fse);
	self = inclusive - function->value;
	xdebug_str_add_fmt(&str, "%s %d\n", function->prefix, self);

	/* xdebug_hash_delete() will free the function. */
	fg_function_delete(context, fse->function_nr);

	/* Increment head value (which is now parent) by inclusive cost. */
	parent_fse = fg_parent_find();
	if (NULL != parent_fse) {
		parent_function = fg_function_find(context, parent_fse->function_nr);
		if (NULL != parent_function) {
			parent_function->value += inclusive;
		}
	}

	fprintf(context->trace_file, "%s", str.d);
	xdfree(str.d);
}

void xdebug_trace_flamegraph_function_return_value(void *ctxt, function_stack_entry *fse, int function_nr, zval *return_value)
{
}

void xdebug_trace_flamegraph_assignment(void *ctxt, function_stack_entry *fse, char *full_varname, zval *retval, char *right_full_varname, const char *op, char *filename, int lineno)
{
}

xdebug_trace_handler_t xdebug_trace_handler_flamegraph_cost =
{
	xdebug_trace_flamegraph_init_cost,
	xdebug_trace_flamegraph_deinit,
	xdebug_trace_flamegraph_write_header,
	xdebug_trace_flamegraph_write_footer,
	xdebug_trace_flamegraph_get_filename,
	xdebug_trace_flamegraph_function_entry,
	xdebug_trace_flamegraph_function_exit,
	xdebug_trace_flamegraph_function_return_value,
	NULL /* xdebug_trace_flamegraph_generator_return_value */,
	xdebug_trace_flamegraph_assignment
};

xdebug_trace_handler_t xdebug_trace_handler_flamegraph_mem =
{
	xdebug_trace_flamegraph_init_mem,
	xdebug_trace_flamegraph_deinit,
	xdebug_trace_flamegraph_write_header,
	xdebug_trace_flamegraph_write_footer,
	xdebug_trace_flamegraph_get_filename,
	xdebug_trace_flamegraph_function_entry,
	xdebug_trace_flamegraph_function_exit,
	xdebug_trace_flamegraph_function_return_value,
	NULL /* xdebug_trace_flamegraph_generator_return_value */,
	xdebug_trace_flamegraph_assignment
};
