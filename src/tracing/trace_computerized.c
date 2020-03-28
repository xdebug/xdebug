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
#include "ext/standard/php_string.h"

#include "php_xdebug.h"
#include "tracing_private.h"
#include "trace_computerized.h"

#include "lib/var_export_line.h"
#include "lib/var_export_serialized.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

void *xdebug_trace_computerized_init(char *fname, char *script_filename, long options)
{
	xdebug_trace_computerized_context *tmp_computerized_context;
	char *used_fname;

	tmp_computerized_context = xdmalloc(sizeof(xdebug_trace_computerized_context));
	tmp_computerized_context->trace_file = xdebug_trace_open_file(fname, script_filename, options, (char**) &used_fname);
	tmp_computerized_context->trace_filename = used_fname;

	return tmp_computerized_context->trace_file ? tmp_computerized_context : NULL;
}

void xdebug_trace_computerized_deinit(void *ctxt)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;

	fclose(context->trace_file);
	context->trace_file = NULL;
	xdfree(context->trace_filename);

	xdfree(context);
}

void xdebug_trace_computerized_write_header(void *ctxt)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	char *str_time;

	fprintf(context->trace_file, "Version: %s\n", XDEBUG_VERSION);
	fprintf(context->trace_file, "File format: 4\n");

	str_time = xdebug_get_time();
	fprintf(context->trace_file, "TRACE START [%s]\n", str_time);
	fflush(context->trace_file);
	xdfree(str_time);
}

void xdebug_trace_computerized_write_footer(void *ctxt)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	char   *str_time;
	double  u_time;
	char   *tmp;

	u_time = xdebug_get_utime();
	tmp = xdebug_sprintf("\t\t\t%F\t", u_time - XG_BASE(start_time));
	fprintf(context->trace_file, "%s", tmp);
	xdfree(tmp);
#if WIN32|WINNT
	fprintf(context->trace_file, "%Iu", zend_memory_usage(0));
#else
	fprintf(context->trace_file, "%zu", zend_memory_usage(0));
#endif
	fprintf(context->trace_file, "\n");
	str_time = xdebug_get_time();

	fprintf(context->trace_file, "TRACE END   [%s]\n\n", str_time);
	fflush(context->trace_file);
	xdfree(str_time);
}

char *xdebug_trace_computerized_get_filename(void *ctxt)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;

	return context->trace_filename;
}

static void add_single_value(xdebug_str *str, zval *zv, int collection_level)
{
	xdebug_str *tmp_value = NULL;

	switch (collection_level) {
		case 1: /* synopsis */
		case 2:
			tmp_value = xdebug_get_zval_synopsis_line(zv, 0, NULL);
			break;
		case 3: /* full */
		case 4: /* full (with var) */
		default:
			tmp_value = xdebug_get_zval_value_line(zv, 0, NULL);
			break;
		case 5: /* serialized */
			tmp_value = xdebug_get_zval_value_serialized(zv, 0, NULL);
			break;
	}
	if (tmp_value) {
		xdebug_str_add_str(str, tmp_value);
		xdebug_str_free(tmp_value);
	} else {
		xdebug_str_add(str, "???", 0);
	}
}


void xdebug_trace_computerized_function_entry(void *ctxt, function_stack_entry *fse, int function_nr)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	char *tmp_name;
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	xdebug_str_add(&str, xdebug_sprintf("%d\t", fse->level), 1);
	xdebug_str_add(&str, xdebug_sprintf("%d\t", function_nr), 1);

	tmp_name = xdebug_show_fname(fse->function, 0, 0);

	xdebug_str_add(&str, "0\t", 0);
	xdebug_str_add(&str, xdebug_sprintf("%F\t", fse->time - XG_BASE(start_time)), 1);
	xdebug_str_add(&str, xdebug_sprintf("%lu\t", fse->memory), 1);
	xdebug_str_add(&str, xdebug_sprintf("%s\t", tmp_name), 1);
	xdebug_str_add(&str, xdebug_sprintf("%d\t", fse->user_defined == XDEBUG_USER_DEFINED ? 1 : 0), 1);
	xdfree(tmp_name);

	if (fse->include_filename) {
		if (fse->function.type == XFUNC_EVAL) {
			zend_string *i_filename = zend_string_init(fse->include_filename, strlen(fse->include_filename), 0);
			zend_string *escaped;
#if PHP_VERSION_ID >= 70300
			escaped = php_addcslashes(i_filename, (char*) "'\\\0..\37", 6);
#else
			escaped = php_addcslashes(i_filename, 0, (char*) "'\\\0..\37", 6);
#endif
			xdebug_str_add(&str, xdebug_sprintf("'%s'", escaped->val), 1);
			zend_string_release(escaped);
			zend_string_release(i_filename);
		} else {
			xdebug_str_add(&str, fse->include_filename, 0);
		}
	}

	/* Filename and Lineno (9, 10) */
	xdebug_str_add(&str, xdebug_sprintf("\t%s\t%d", fse->filename, fse->lineno), 1);


	if (XINI_BASE(collect_params) > 0) {
		unsigned int j = 0; /* Counter */

		/* Nr of arguments (11) */
		xdebug_str_add(&str, xdebug_sprintf("\t%d", fse->varc), 1);

		/* Arguments (12-...) */
		for (j = 0; j < fse->varc; j++) {
			xdebug_str_addl(&str, "\t", 1, 0);

			if (fse->var[j].is_variadic) {
				xdebug_str_addl(&str, "...\t", 4, 0);
			}

			if (fse->var[j].name && XINI_BASE(collect_params) == 4) {
				xdebug_str_add(&str, xdebug_sprintf("$%s = ", fse->var[j].name), 1);
			}

			if (!Z_ISUNDEF(fse->var[j].data)) {
				add_single_value(&str, &(fse->var[j].data), XINI_BASE(collect_params));
			} else {
				xdebug_str_add(&str, "???", 0);
			}
		}
	}

	/* Trailing \n */
	xdebug_str_add(&str, "\n", 0);

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);
	xdfree(str.d);
}

void xdebug_trace_computerized_function_exit(void *ctxt, function_stack_entry *fse, int function_nr)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	xdebug_str_add(&str, xdebug_sprintf("%d\t", fse->level), 1);
	xdebug_str_add(&str, xdebug_sprintf("%d\t", function_nr), 1);

	xdebug_str_add(&str, "1\t", 0);
	xdebug_str_add(&str, xdebug_sprintf("%F\t", xdebug_get_utime() - XG_BASE(start_time)), 1);
	xdebug_str_add(&str, xdebug_sprintf("%lu\n", zend_memory_usage(0)), 1);

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);
	xdfree(str.d);
}

void xdebug_trace_computerized_function_return_value(void *ctxt, function_stack_entry *fse, int function_nr, zval *return_value)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	xdebug_str_add(&str, xdebug_sprintf("%d\t", fse->level), 1);
	xdebug_str_add(&str, xdebug_sprintf("%d\t", function_nr), 1);
	xdebug_str_add(&str, "R\t\t\t", 0);

	add_single_value(&str, return_value, XINI_BASE(collect_params));

	xdebug_str_addl(&str, "\n", 2, 0);

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);
	xdfree(str.d);
}

xdebug_trace_handler_t xdebug_trace_handler_computerized =
{
	xdebug_trace_computerized_init,
	xdebug_trace_computerized_deinit,
	xdebug_trace_computerized_write_header,
	xdebug_trace_computerized_write_footer,
	xdebug_trace_computerized_get_filename,
	xdebug_trace_computerized_function_entry,
	xdebug_trace_computerized_function_exit,
	xdebug_trace_computerized_function_return_value,
	NULL /* xdebug_trace_computerized_generator_return_value */,
	NULL /* xdebug_trace_computerized_assignment */
};
