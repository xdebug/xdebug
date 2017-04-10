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
#include "xdebug_tracing.h"
#include "xdebug_trace_textual.h"
#include "xdebug_var.h"
#include "ext/standard/php_string.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

void *xdebug_trace_textual_init(char *fname, long options TSRMLS_DC)
{
	xdebug_trace_textual_context *tmp_textual_context;
	char *used_fname;

	tmp_textual_context = xdmalloc(sizeof(xdebug_trace_textual_context));
	tmp_textual_context->trace_file = xdebug_trace_open_file(fname, options, (char**) &used_fname TSRMLS_CC);
	tmp_textual_context->trace_filename = used_fname;

	return tmp_textual_context->trace_file ? tmp_textual_context : NULL;
}

void xdebug_trace_textual_deinit(void *ctxt TSRMLS_DC)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;

	fclose(context->trace_file);
	context->trace_file = NULL;
	xdfree(context->trace_filename);

	xdfree(context);
}

void xdebug_trace_textual_write_header(void *ctxt TSRMLS_DC)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;
	char *str_time;

	str_time = xdebug_get_time();
	fprintf(context->trace_file, "TRACE START [%s]\n", str_time);
	fflush(context->trace_file);
	xdfree(str_time);
}

void xdebug_trace_textual_write_footer(void *ctxt TSRMLS_DC)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;
	char   *str_time;
	double  u_time;
	char   *tmp;

	u_time = xdebug_get_utime();
	tmp = xdebug_sprintf("%10.4F ", u_time - XG(start_time));
	fprintf(context->trace_file, "%s", tmp);
	xdfree(tmp);
#if WIN32|WINNT
	fprintf(context->trace_file, "%10Iu", zend_memory_usage(0 TSRMLS_CC));
#else
	fprintf(context->trace_file, "%10zu", zend_memory_usage(0 TSRMLS_CC));
#endif
	fprintf(context->trace_file, "\n");
	str_time = xdebug_get_time();
	fprintf(context->trace_file, "TRACE END   [%s]\n\n", str_time);
	fflush(context->trace_file);
	xdfree(str_time);
}

char *xdebug_trace_textual_get_filename(void *ctxt TSRMLS_DC)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;

	return context->trace_filename;
}

void add_single_value(xdebug_str *str, zval *zv, int collection_level TSRMLS_DC)
{
	char *tmp_value;

	switch (collection_level) {
		case 1: /* synopsis */
		case 2:
			tmp_value = xdebug_get_zval_synopsis(zv, 0, NULL);
			break;
		case 3: /* full */
		case 4: /* full (with var) */
		default:
			tmp_value = xdebug_get_zval_value(zv, 0, NULL);
			break;
		case 5: /* serialized */
			tmp_value = xdebug_get_zval_value_serialized(zv, 0, NULL TSRMLS_CC);
			break;
	}
	if (tmp_value) {
		xdebug_str_add(str, tmp_value, 1);
	} else {
		xdebug_str_add(str, "???", 0);
	}
}

void xdebug_trace_textual_function_entry(void *ctxt, function_stack_entry *fse, int function_nr TSRMLS_DC)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;
	int c = 0; /* Comma flag */
	unsigned int j = 0; /* Counter */
	char *tmp_name;
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	tmp_name = xdebug_show_fname(fse->function, 0, 0 TSRMLS_CC);

	xdebug_str_add(&str, xdebug_sprintf("%10.4F ", fse->time - XG(start_time)), 1);
	xdebug_str_add(&str, xdebug_sprintf("%10lu ", fse->memory), 1);
	if (XG(show_mem_delta)) {
		xdebug_str_add(&str, xdebug_sprintf("%+8ld ", fse->memory - fse->prev_memory), 1);
	}
	for (j = 0; j < fse->level; j++) {
		xdebug_str_addl(&str, "  ", 2, 0);
	}
	xdebug_str_add(&str, xdebug_sprintf("-> %s(", tmp_name), 1);

	xdfree(tmp_name);

	/* Printing vars */
	if (XG(collect_params) > 0) {
		int variadic_opened = 0;
		int variadic_count  = 0;

		for (j = 0; j < fse->varc; j++) {
			if (c) {
				xdebug_str_addl(&str, ", ", 2, 0);
			} else {
				c = 1;
			}

			if (
				(fse->var[j].is_variadic && fse->var[j].addr)
#if PHP_VERSION_ID >= 50600 && PHP_VERSION_ID < 70000
				|| (!fse->var[j].addr && fse->is_variadic && j == fse->varc - 1)
#endif
			) {
				xdebug_str_add(&str, "...", 0);
				variadic_opened = 1;
#if PHP_VERSION_ID >= 70000
				c = 0;
#endif
			}

			if (fse->var[j].name && XG(collect_params) == 4) {
				xdebug_str_add(&str, xdebug_sprintf("$%s = ", fse->var[j].name), 1);
			}

			if (fse->var[j].is_variadic && fse->var[j].addr) {
				xdebug_str_add(&str, "variadic(", 0);
#if PHP_VERSION_ID >= 70000
				continue;
#endif
			}

			if (
				(variadic_opened && XG(collect_params) != 5)
#if PHP_VERSION_ID >= 50600 && PHP_VERSION_ID < 70000
				&& (!(!fse->var[j].addr && fse->is_variadic && j == fse->varc - 1))
#endif
			) {
				xdebug_str_add(&str, xdebug_sprintf("%d => ", variadic_count++), 1);
			}

			if (fse->var[j].addr) {
				add_single_value(&str, fse->var[j].addr, XG(collect_params) TSRMLS_CC);
#if PHP_VERSION_ID >= 50600 && PHP_VERSION_ID < 70000
			} else if (fse->is_variadic && j == fse->varc - 1) {
				xdebug_str_addl(&str, "variadic(", 9, 0);
#endif
			} else {
				xdebug_str_addl(&str, "???", 3, 0);
			}
		}

		if (variadic_opened) {
			xdebug_str_add(&str, ")", 0);
		}
	}

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

	xdebug_str_add(&str, xdebug_sprintf(") %s:%d\n", fse->filename, fse->lineno), 1);

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);

	xdfree(str.d);
}

/* Used for normal return values, and generator return values */
static void xdebug_return_trace_stack_common(xdebug_str *str, function_stack_entry *fse TSRMLS_DC)
{
	unsigned int j = 0; /* Counter */

	xdebug_str_add(str, xdebug_sprintf("%10.4F ", xdebug_get_utime() - XG(start_time)), 1);
	xdebug_str_add(str, xdebug_sprintf("%10lu ", zend_memory_usage(0 TSRMLS_CC)), 1);

	if (XG(show_mem_delta)) {
		xdebug_str_addl(str, "        ", 8, 0);
	}
	for (j = 0; j < fse->level; j++) {
		xdebug_str_addl(str, "  ", 2, 0);
	}
	xdebug_str_addl(str, " >=> ", 5, 0);
}


void xdebug_trace_textual_function_return_value(void *ctxt, function_stack_entry *fse, int function_nr, zval *return_value TSRMLS_DC)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;
	xdebug_str str = XDEBUG_STR_INITIALIZER;
	char      *tmp_value;

	xdebug_return_trace_stack_common(&str, fse TSRMLS_CC);

	tmp_value = xdebug_get_zval_value(return_value, 0, NULL);
	if (tmp_value) {
		xdebug_str_add(&str, tmp_value, 1);
	}
	xdebug_str_addl(&str, "\n", 2, 0);

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);

	xdfree(str.d);
}

#if PHP_VERSION_ID >= 50500
void xdebug_trace_textual_generator_return_value(void *ctxt, function_stack_entry *fse, int function_nr, zend_generator *generator TSRMLS_DC)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;
	xdebug_str str = XDEBUG_STR_INITIALIZER;
	char      *tmp_value = NULL;

	if (! (generator->flags & ZEND_GENERATOR_CURRENTLY_RUNNING)) {
		return;
	}

#if PHP_VERSION_ID >= 70000
	if (generator->node.ptr.root->execute_data == NULL) {
		return;
	}
#endif

	/* Generator key */
#if PHP_VERSION_ID >= 70000
	tmp_value = xdebug_get_zval_value(&generator->key, 0, NULL);
#else
	tmp_value = xdebug_get_zval_value(generator->key, 0, NULL);
#endif
	if (tmp_value) {
		xdebug_return_trace_stack_common(&str, fse TSRMLS_CC);

		xdebug_str_addl(&str, "(", 1, 0);
		xdebug_str_add(&str, tmp_value, 1);
		xdebug_str_addl(&str, " => ", 4, 0);

#if PHP_VERSION_ID >= 70000
		tmp_value = xdebug_get_zval_value(&generator->value, 0, NULL);
#else
		tmp_value = xdebug_get_zval_value(generator->value, 0, NULL);
#endif
		if (tmp_value) {
			xdebug_str_add(&str, tmp_value, 1);
		}
		xdebug_str_addl(&str, ")", 1, 0);
		xdebug_str_addl(&str, "\n", 2, 0);

		fprintf(context->trace_file, "%s", str.d);
		fflush(context->trace_file);

		xdfree(str.d);
	}
}
#endif

void xdebug_trace_textual_assignment(void *ctxt, function_stack_entry *fse, char *full_varname, zval *retval, char *op, char *filename, int lineno TSRMLS_DC)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;
	unsigned int j = 0;
	xdebug_str str = XDEBUG_STR_INITIALIZER;
	char      *tmp_value;

	xdebug_str_addl(&str, "                    ", 20, 0);
	if (XG(show_mem_delta)) {
		xdebug_str_addl(&str, "        ", 8, 0);
	}
	for (j = 0; j <= fse->level; j++) {
		xdebug_str_addl(&str, "  ", 2, 0);
	}
	xdebug_str_addl(&str, "   => ", 6, 0);

	xdebug_str_add(&str, full_varname, 0);

	if (op[0] != '\0' ) { /* pre/post inc/dec ops are special */
		xdebug_str_add(&str, xdebug_sprintf(" %s ", op), 1);

		tmp_value = xdebug_get_zval_value(retval, 0, NULL);

		if (tmp_value) {
			xdebug_str_add(&str, tmp_value, 1);
		} else {
			xdebug_str_addl(&str, "NULL", 4, 0);
		}
	}
	xdebug_str_add(&str, xdebug_sprintf(" %s:%d\n", filename, lineno), 1);

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);

	xdfree(str.d);
}

xdebug_trace_handler_t xdebug_trace_handler_textual =
{
	xdebug_trace_textual_init,
	xdebug_trace_textual_deinit,
	xdebug_trace_textual_write_header,
	xdebug_trace_textual_write_footer,
	xdebug_trace_textual_get_filename,
	xdebug_trace_textual_function_entry,
	NULL /*xdebug_trace_textual_function_exit */,
	xdebug_trace_textual_function_return_value,
#if PHP_VERSION_ID >= 50500
	xdebug_trace_textual_generator_return_value,
#endif
	xdebug_trace_textual_assignment
};
