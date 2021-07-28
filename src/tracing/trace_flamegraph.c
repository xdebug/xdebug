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

flamegraph_stack *fg_stack_create()
{
	flamegraph_stack *ret;
	ret = xdmalloc(sizeof(flamegraph_stack));
	ret->head = NULL;
	return ret;
}

void fg_stack_free(flamegraph_stack *stack)
{
	flamegraph_stack_item *cur, *prev;
	cur = stack->head;
	while (cur) {
		prev = cur->next;
		xdfree(cur->prefix);
		xdfree(cur);
		cur = prev;
	}
	xdfree(stack);
}

static inline void fg_stack_inc(flamegraph_stack *stack, int inc)
{
	if (NULL == stack->head) {
		return;
	}
	stack->head->value += inc;
}

static inline void fg_stack_push(flamegraph_stack *stack, char *prefix)
{
	flamegraph_stack_item *item;
	item = xdmalloc(sizeof(flamegraph_stack_item));
	item->prefix = xdstrdup(prefix);
	item->value = 0;
	item->next = stack->head;
	stack->head = item;
}

static inline flamegraph_stack_item *fg_stack_get_head(flamegraph_stack *stack)
{
	return stack->head;
}

static inline flamegraph_stack_item *fg_stack_pop(flamegraph_stack *stack)
{
	flamegraph_stack_item *item;
	if (NULL == stack->head) {
		return 0;
	}
	item = stack->head;
	stack->head = item->next;
	return item;
}

static inline int compute_inclusive_value(xdebug_trace_flamegraph_context *context, function_stack_entry *fse)
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
		tmp_flamegraph_context->stack = fg_stack_create();
	}

	return tmp_flamegraph_context;
}

void *xdebug_trace_flamegraph_init_mem(char *fname, zend_string *script_filename, long options)
{
	xdebug_trace_flamegraph_context *tmp_flamegraph_context = xdebug_trace_flamegraph_init(fname, script_filename, options);

	if (tmp_flamegraph_context) {
		tmp_flamegraph_context->mode = XDEBUG_TRACE_OPTION_FLAMEGRAPH_MEM;
		tmp_flamegraph_context->stack = fg_stack_create();
	}

	return tmp_flamegraph_context;
}

void xdebug_trace_flamegraph_deinit(void *ctxt)
{
	xdebug_trace_flamegraph_context *context = (xdebug_trace_flamegraph_context*) ctxt;

	if (NULL != context->stack) {
		fg_stack_free(context->stack);
	}

	fclose(context->trace_file);
	context->trace_file = NULL;
	xdfree(context->trace_filename);

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
	flamegraph_stack_item           *head;
	char                            *tmp_name;
	char                            *prefix;

	head = fg_stack_get_head(context->stack);
	tmp_name = xdebug_show_fname(fse->function, 0, 0);

	/* Insert an item into the custom stack which yields path prefix and
	   a 0 value for the current function, if this function calls children
	   they will append their costs to it. Once we reach out this function
	   we will remove the element from the stack. */
	if (NULL == head) {
		/* Root function. */
		fg_stack_push(context->stack, tmp_name);
	} else {
		prefix = xdmalloc(strlen(head->prefix) + 1 + strlen(tmp_name));
		sprintf(prefix, "%s;%s", head->prefix, tmp_name);
		fg_stack_push(context->stack, prefix);
		xdfree(prefix);
	}

	xdfree(tmp_name);
}

void xdebug_trace_flamegraph_function_exit(void *ctxt, function_stack_entry *fse, int function_nr)
{
	xdebug_trace_flamegraph_context *context = (xdebug_trace_flamegraph_context*) ctxt;
	xdebug_str                       str = XDEBUG_STR_INITIALIZER;
	int                              inclusive;
	int                              self;
	flamegraph_stack_item           *stack_item;

	stack_item = fg_stack_pop(context->stack);

	if (NULL == stack_item) {
		/* This should never happen, better be safe than sorry. */
		/* @todo log error? */
		return;
	}

	inclusive = compute_inclusive_value(context, fse);
	self = inclusive - stack_item->value;
	xdebug_str_add_fmt(&str, "%s %d\n", stack_item->prefix, self);
	xdfree(stack_item->prefix);
	xdfree(stack_item);

	/* Increment head value (which is now parent) by inclusive cost. */
	fg_stack_inc(context->stack, inclusive);

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
