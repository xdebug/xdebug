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
#include "trace_textual.h"

#include "lib/lib_private.h"
#include "lib/var_export_line.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

void *xdebug_trace_textual_init(char *fname, zend_string *script_filename, long options)
{
	xdebug_trace_textual_context *tmp_textual_context;
	char *used_fname;

	tmp_textual_context = xdmalloc(sizeof(xdebug_trace_textual_context));
	tmp_textual_context->trace_file = xdebug_trace_open_file(fname, script_filename, options, (char**) &used_fname);
	tmp_textual_context->trace_filename = used_fname;

	return tmp_textual_context->trace_file ? tmp_textual_context : NULL;
}

void xdebug_trace_textual_deinit(void *ctxt)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;

	fclose(context->trace_file);
	context->trace_file = NULL;
	xdfree(context->trace_filename);

	xdfree(context);
}

void xdebug_trace_textual_write_header(void *ctxt)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;
	char *str_time;

	str_time = xdebug_nanotime_to_chars(xdebug_get_nanotime(), 6);
	fprintf(context->trace_file, "TRACE START [%s]\n", str_time);
	xdfree(str_time);

	fflush(context->trace_file);
}

void xdebug_trace_textual_write_footer(void *ctxt)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;
	char     *str_time;
	uint64_t  nanotime;
	char     *tmp;

	nanotime = xdebug_get_nanotime();
	tmp = xdebug_sprintf("%10.4F ", XDEBUG_SECONDS_SINCE_START(nanotime));
	fprintf(context->trace_file, "%s", tmp);
	xdfree(tmp);
#if WIN32|WINNT
	fprintf(context->trace_file, "%10Iu", zend_memory_usage(0));
#else
	fprintf(context->trace_file, "%10zu", zend_memory_usage(0));
#endif
	fprintf(context->trace_file, "\n");

	str_time = xdebug_nanotime_to_chars(nanotime, 6);
	fprintf(context->trace_file, "TRACE END   [%s]\n\n", str_time);
	xdfree(str_time);

	fflush(context->trace_file);
}

char *xdebug_trace_textual_get_filename(void *ctxt)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;

	return context->trace_filename;
}

static void add_single_value(xdebug_str *str, zval *zv)
{
	xdebug_str *tmp_value = NULL;

	tmp_value = xdebug_get_zval_value_line(zv, 0, NULL);

	if (tmp_value) {
		xdebug_str_add_str(str, tmp_value);
		xdebug_str_free(tmp_value);
	} else {
		xdebug_str_add_literal(str, "???");
	}
}

static void add_arguments(xdebug_str *line_entry, function_stack_entry *fse)
{
	unsigned int j = 0; /* Counter */
	int c = 0; /* Comma flag */
	int variadic_opened = 0;
	int variadic_count  = 0;
	int sent_variables = fse->varc;

	if (sent_variables > 0 && fse->var[sent_variables-1].is_variadic && Z_ISUNDEF(fse->var[sent_variables-1].data)) {
		sent_variables--;
	}

	for (j = 0; j < sent_variables; j++) {
		if (c) {
			xdebug_str_add_literal(line_entry, ", ");
		} else {
			c = 1;
		}

		if (fse->var[j].is_variadic) {
			xdebug_str_add_literal(line_entry, "...");
			variadic_opened = 1;
			c = 0;
		}

		if (fse->var[j].name) {
			xdebug_str_addc(line_entry, '$');
			xdebug_str_add_zstr(line_entry, fse->var[j].name);
			if (variadic_opened && !fse->var[j].is_variadic) {
				xdebug_str_add_literal(line_entry, " => ");
			} else {
				xdebug_str_add_literal(line_entry, " = ");
			}
		}

		if (fse->var[j].is_variadic) {
			xdebug_str_add_literal(line_entry, "variadic(");
			if (Z_ISUNDEF(fse->var[j].data)) {
				continue;
			}
			c = 1;
		}

		if (variadic_opened && (!fse->var[j].name || fse->var[j].is_variadic)) {
			xdebug_str_add_fmt(line_entry, "%d => ", variadic_count++);
		}

		if (!Z_ISUNDEF(fse->var[j].data)) {
			add_single_value(line_entry, &fse->var[j].data);
		} else {
			xdebug_str_add_literal(line_entry, "???");
		}
	}

	if (variadic_opened) {
		xdebug_str_addc(line_entry, ')');
	}
}

void xdebug_trace_textual_function_entry(void *ctxt, function_stack_entry *fse, int function_nr)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;
	unsigned int j = 0; /* Counter */
	char *tmp_name;
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	tmp_name = xdebug_show_fname(fse->function, 0, 0);

	xdebug_str_add_fmt(&str, "%10.4F ", XDEBUG_SECONDS_SINCE_START(fse->nanotime));
	xdebug_str_add_fmt(&str, "%10lu ", fse->memory);
	for (j = 0; j < fse->level; j++) {
		xdebug_str_add_literal(&str, "  ");
	}
	xdebug_str_add_fmt(&str, "-> %s(", tmp_name);

	xdfree(tmp_name);

	add_arguments(&str, fse);

	if (fse->include_filename) {
		if (fse->function.type == XFUNC_EVAL) {
			zend_string *escaped;
#if PHP_VERSION_ID >= 70300
			escaped = php_addcslashes(fse->include_filename, (char*) "'\\\0..\37", 6);
#else
			escaped = php_addcslashes(fse->include_filename, 0, (char*) "'\\\0..\37", 6);
#endif
			xdebug_str_addc(&str, '\'');
			xdebug_str_add_zstr(&str, escaped);
			xdebug_str_addc(&str, '\'');
			zend_string_release(escaped);
		} else {
			xdebug_str_add_zstr(&str, fse->include_filename);
		}
	}

	xdebug_str_add_fmt(&str, ") %s:%d\n", ZSTR_VAL(fse->filename), fse->lineno);

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);

	xdfree(str.d);
}

/* Used for normal return values, and generator return values */
static void xdebug_return_trace_stack_common(xdebug_str *str, function_stack_entry *fse)
{
	unsigned int j = 0; /* Counter */

	xdebug_str_add_fmt(str, "%10.4F ", XDEBUG_SECONDS_SINCE_START(xdebug_get_nanotime()));
	xdebug_str_add_fmt(str, "%10lu ", zend_memory_usage(0));

	for (j = 0; j < fse->level; j++) {
		xdebug_str_add_literal(str, "  ");
	}
	xdebug_str_add_literal(str, " >=> ");
}


void xdebug_trace_textual_function_return_value(void *ctxt, function_stack_entry *fse, int function_nr, zval *return_value)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;
	xdebug_str                    str = XDEBUG_STR_INITIALIZER;
	xdebug_str                   *tmp_value;

	xdebug_return_trace_stack_common(&str, fse);

	tmp_value = xdebug_get_zval_value_line(return_value, 0, NULL);
	if (tmp_value) {
		xdebug_str_add_str(&str, tmp_value);
		xdebug_str_free(tmp_value);
	}
	xdebug_str_addc(&str, '\n');

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);

	xdebug_str_destroy(&str);
}

void xdebug_trace_textual_generator_return_value(void *ctxt, function_stack_entry *fse, int function_nr, zend_generator *generator)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;
	xdebug_str                    str = XDEBUG_STR_INITIALIZER;
	xdebug_str                   *tmp_value = NULL;

	if (! (generator->flags & ZEND_GENERATOR_CURRENTLY_RUNNING)) {
		return;
	}

	if (generator->execute_data == NULL) {
		return;
	}

	/* Generator key */
	tmp_value = xdebug_get_zval_value_line(&generator->key, 0, NULL);
	if (!tmp_value) {
		return;
	}

	xdebug_return_trace_stack_common(&str, fse);

	xdebug_str_addc(&str, '(');
	xdebug_str_add_str(&str, tmp_value);
	xdebug_str_add_literal(&str, " => ");

	tmp_value = xdebug_get_zval_value_line(&generator->value, 0, NULL);
	if (tmp_value) {
		xdebug_str_add_str(&str, tmp_value);
		xdebug_str_free(tmp_value);
	}

	xdebug_str_add_literal(&str, ")\n");

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);

	xdebug_str_destroy(&str);
}

void xdebug_trace_textual_assignment(void *ctxt, function_stack_entry *fse, char *full_varname, zval *retval, char *right_full_varname, const char *op, char *filename, int lineno)
{
	xdebug_trace_textual_context *context = (xdebug_trace_textual_context*) ctxt;
	unsigned int                  j = 0;
	xdebug_str                    str = XDEBUG_STR_INITIALIZER;
	xdebug_str                   *tmp_value;

	xdebug_str_add_literal(&str, "                    ");
	for (j = 0; j <= fse->level; j++) {
		xdebug_str_add_literal(&str, "  ");
	}
	xdebug_str_add_literal(&str, "   => ");

	xdebug_str_add(&str, full_varname, 0);

	if (op[0] != '\0' ) { /* pre/post inc/dec ops are special */
		xdebug_str_addc(&str, ' ');
		xdebug_str_add(&str, op, 0);
		xdebug_str_addc(&str, ' ');

		if (right_full_varname) {
			xdebug_str_add(&str, right_full_varname, 0);
		} else {
			tmp_value = xdebug_get_zval_value_line(retval, 0, NULL);

			if (tmp_value) {
				xdebug_str_add_str(&str, tmp_value);
				xdebug_str_free(tmp_value);
			} else {
				xdebug_str_add_literal(&str, "NULL");
			}
		}

	}
	xdebug_str_add_fmt(&str, " %s:%d\n", filename, lineno);

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
	xdebug_trace_textual_generator_return_value,
	xdebug_trace_textual_assignment
};
