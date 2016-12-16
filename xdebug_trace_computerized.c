/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2016 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <derick@xdebug.org>                          |
   +----------------------------------------------------------------------+
 */
#include "xdebug_trace_computerized.h"
#include "xdebug_var.h"
#include "ext/standard/php_string.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

void *xdebug_trace_computerized_init(char *fname, long options TSRMLS_DC)
{
	xdebug_trace_computerized_context *tmp_computerized_context;
	char *used_fname;

	tmp_computerized_context = xdmalloc(sizeof(xdebug_trace_computerized_context));
	tmp_computerized_context->trace_file = xdebug_trace_open_file(fname, options, (char**) &used_fname TSRMLS_CC);
	tmp_computerized_context->trace_filename = used_fname;

	return tmp_computerized_context->trace_file ? tmp_computerized_context : NULL;
}

void xdebug_trace_computerized_deinit(void *ctxt TSRMLS_DC)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;

	fclose(context->trace_file);
	context->trace_file = NULL;
	xdfree(context->trace_filename);

	xdfree(context);
}

void xdebug_trace_computerized_write_header(void *ctxt TSRMLS_DC)
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

void xdebug_trace_computerized_write_footer(void *ctxt TSRMLS_DC)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	char   *str_time;
	double  u_time;
	char   *tmp;

	u_time = xdebug_get_utime();
	tmp = xdebug_sprintf("\t\t\t%F\t", u_time - XG(start_time));
	fprintf(context->trace_file, "%s", tmp);
	xdfree(tmp);
#if WIN32|WINNT
	fprintf(context->trace_file, "%Iu", zend_memory_usage(0 TSRMLS_CC));
#else
	fprintf(context->trace_file, "%zu", zend_memory_usage(0 TSRMLS_CC));
#endif
	fprintf(context->trace_file, "\n");
	str_time = xdebug_get_time();

	fprintf(context->trace_file, "TRACE END   [%s]\n\n", str_time);
	fflush(context->trace_file);
	xdfree(str_time);
}

char *xdebug_trace_computerized_get_filename(void *ctxt TSRMLS_DC)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;

	return context->trace_filename;
}

static char *render_variable(zval *var, int type TSRMLS_DC)
{
	char *tmp_value = NULL;

	switch (XG(collect_params)) {
		case 1: /* synopsis */
		case 2:
			tmp_value = xdebug_get_zval_synopsis(var, 0, NULL);
			break;
		case 3: /* full */
		case 4: /* full (with var) */
		default:
			tmp_value = xdebug_get_zval_value(var, 0, NULL);
			break;
		case 5: /* serialized */
			tmp_value = xdebug_get_zval_value_serialized(var, 0, NULL TSRMLS_CC);
			break;
	}

	return tmp_value;
}


void xdebug_trace_computerized_function_entry(void *ctxt, function_stack_entry *fse, int function_nr TSRMLS_DC)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	char *tmp_name;
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	xdebug_str_add(&str, xdebug_sprintf("%d\t", fse->level), 1);
	xdebug_str_add(&str, xdebug_sprintf("%d\t", function_nr), 1);

	tmp_name = xdebug_show_fname(fse->function, 0, 0 TSRMLS_CC);

	xdebug_str_add(&str, "0\t", 0);
	xdebug_str_add(&str, xdebug_sprintf("%F\t", fse->time - XG(start_time)), 1);
	xdebug_str_add(&str, xdebug_sprintf("%lu\t", fse->memory), 1);
	xdebug_str_add(&str, xdebug_sprintf("%s\t", tmp_name), 1);
	xdebug_str_add(&str, xdebug_sprintf("%d\t", fse->user_defined == XDEBUG_EXTERNAL ? 1 : 0), 1);
	xdfree(tmp_name);

	if (fse->include_filename) {
		if (fse->function.type == XFUNC_EVAL) {
#if PHP_VERSION_ID >= 70000
			zend_string *i_filename = zend_string_init(fse->include_filename, strlen(fse->include_filename), 0);
			zend_string *escaped;
			escaped = php_addcslashes(i_filename, 0, "'\\\0..\37", 6);
			xdebug_str_add(&str, xdebug_sprintf("'%s'", escaped->val), 1);
			zend_string_release(escaped);
			zend_string_release(i_filename);
#else
			int tmp_len;

			char *escaped;
			escaped = php_addcslashes(fse->include_filename, strlen(fse->include_filename), &tmp_len, 0, "'\\\0..\37", 6 TSRMLS_CC);
			xdebug_str_add(&str, xdebug_sprintf("'%s'", escaped), 1);
			efree(escaped);
#endif
		} else {
			xdebug_str_add(&str, fse->include_filename, 0);
		}
	}

	/* Filename and Lineno (9, 10) */
	xdebug_str_add(&str, xdebug_sprintf("\t%s\t%d", fse->filename, fse->lineno), 1);


	if (XG(collect_params) > 0) {
		unsigned int j = 0; /* Counter */

		/* Nr of arguments (11) */
		xdebug_str_add(&str, xdebug_sprintf("\t%d", fse->varc), 1);

		/* Arguments (12-...) */
		for (j = 0; j < fse->varc; j++) {
			char *tmp_value;

			xdebug_str_addl(&str, "\t", 1, 0);

			if (fse->var[j].is_variadic) {
				xdebug_str_addl(&str, "...\t", 4, 0);
			}

			if (fse->var[j].name && XG(collect_params) == 4) {
				xdebug_str_add(&str, xdebug_sprintf("$%s = ", fse->var[j].name), 1);
			}

			tmp_value = render_variable(fse->var[j].addr, XG(collect_params) TSRMLS_CC);

			if (tmp_value) {
				xdebug_str_add(&str, tmp_value, 1);
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

void xdebug_trace_computerized_function_exit(void *ctxt, function_stack_entry *fse, int function_nr TSRMLS_DC)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	xdebug_str_add(&str, xdebug_sprintf("%d\t", fse->level), 1);
	xdebug_str_add(&str, xdebug_sprintf("%d\t", function_nr), 1);

	xdebug_str_add(&str, "1\t", 0);
	xdebug_str_add(&str, xdebug_sprintf("%F\t", xdebug_get_utime() - XG(start_time)), 1);
	xdebug_str_add(&str, xdebug_sprintf("%lu\n", zend_memory_usage(0 TSRMLS_CC)), 1);

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);
	xdfree(str.d);
}

void xdebug_trace_computerized_function_return_value(void *ctxt, function_stack_entry *fse, int function_nr, zval *return_value TSRMLS_DC)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	xdebug_str str = XDEBUG_STR_INITIALIZER;
	char      *tmp_value = NULL;

	xdebug_str_add(&str, xdebug_sprintf("%d\t", fse->level), 1);
	xdebug_str_add(&str, xdebug_sprintf("%d\t", function_nr), 1);
	xdebug_str_add(&str, "R\t\t\t", 0);

	tmp_value = render_variable(return_value, XG(collect_params) TSRMLS_CC);

	if (tmp_value) {
		xdebug_str_add(&str, tmp_value, 1);
	} else {
		xdebug_str_add(&str, "???", 0);
	}
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
#if PHP_VERSION_ID >= 50500
	NULL /* xdebug_trace_computerized_generator_return_value */,
#endif
	NULL /* xdebug_trace_computerized_assignment */
};
