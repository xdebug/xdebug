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
 */
#include "php.h"
#include "ext/standard/php_string.h"

#include "php_xdebug.h"
#include "tracing_private.h"
#include "trace_flamegraph.h"

#include "lib/lib_private.h"
#include "lib/var_export_line.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

int compute_inclusive_value(xdebug_trace_flamegraph_context *context, function_stack_entry *fse)
{
	int value = 0;

	switch (context->mode) {
		case XDEBUG_TRACE_OPTION_FLAMEGRAPH_MEM:
			if (fse->memory < fse->prev_memory) {
				/*
				   @todo When memory is below 0, flamegraph generator will
				     error, and you won't have a good visual.
				     This happens when garbage collection happened during this
				     function and freed something it didn't allocate, I guess.
				 */
				value = 0;
			} else {
				value = fse->memory - fse->prev_memory;
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
		/* tmp_flamegraph_context->values = new hashmap */
	}

	return tmp_flamegraph_context;
}

void *xdebug_trace_flamegraph_init_mem(char *fname, zend_string *script_filename, long options)
{
	xdebug_trace_flamegraph_context *tmp_flamegraph_context = xdebug_trace_flamegraph_init(fname, script_filename, options);

	if (tmp_flamegraph_context) {
		tmp_flamegraph_context->mode = XDEBUG_TRACE_OPTION_FLAMEGRAPH_MEM;
		/* tmp_flamegraph_context->values = new hashmap */
	}

	return tmp_flamegraph_context;
}

void xdebug_trace_flamegraph_deinit(void *ctxt)
{
	xdebug_trace_flamegraph_context *context = (xdebug_trace_flamegraph_context*) ctxt;

	/* free(context->values) */

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
}

void xdebug_trace_flamegraph_function_exit(void *ctxt, function_stack_entry *fse, int function_nr)
{
	xdebug_trace_flamegraph_context *context = (xdebug_trace_flamegraph_context*) ctxt;
	xdebug_str str = XDEBUG_STR_INITIALIZER;
	char *tmp_name;
	int value, child_delta, parent_delta;
	function_stack_entry *parent_fse;
	size_t i;
	size_t stack_size = XDEBUG_VECTOR_COUNT(XG_BASE(stack)) - 1;

	value = compute_inclusive_value(context, fse);

	/* We need to compute self-timings, not inclusive, otherwise timings
	   will not reveal anything and your flamegraph will just look like
	   a nice pine forest without any meaning */
	/* context->values->set(function_nr, 0) */

	/*
	   @todo For now this will always include the full stack instead of
	     starting where xdebug_start_trace() started, but I don't think
	     there's any solution because xdebug_stop_trace() could be
	     called lower in the stack, and this would yield a very wrong
	     trace file.
	 */
	for (i = 0; i < stack_size; ++i) {
		parent_fse = xdebug_vector_element_get(XG_BASE(stack), i);
		tmp_name = xdebug_show_fname(parent_fse->function, 0, 0);
		xdebug_str_add_fmt(&str, "%s;", tmp_name);

		/* Catch direct parent and add child timing */
		if (i == stack_size) {
			/* parent_delta = hashmap_get(context->values, parent_fse.nr, 0) */
			/* hashmap_set(context->values, parent_fse->nr, parent_delta + value) */
		}
	}

	tmp_name = xdebug_show_fname(fse->function, 0, 0);

	/* child_delta = hashmap_get(context->values, function_nr, 0) */
	/* hashmap_remove(context->values, function_nr) */

	xdebug_str_add_fmt(&str, "%s %d\n", tmp_name, value /* + child_delta */);

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);
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
