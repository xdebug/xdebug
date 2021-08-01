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
#include "trace_computerized.h"

#include "lib/lib_private.h"
#include "lib/var_export_line.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

void *xdebug_trace_computerized_init(char *fname, zend_string *script_filename, long options)
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

	str_time = xdebug_nanotime_to_chars(xdebug_get_nanotime(), 6);
	fprintf(context->trace_file, "TRACE START [%s]\n", str_time);
	xdfree(str_time);

	fflush(context->trace_file);
}

void xdebug_trace_computerized_write_footer(void *ctxt)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	char    *str_time;
	uint64_t nanotime;
	char    *tmp;

	nanotime = xdebug_get_nanotime();

	tmp = xdebug_sprintf("\t\t\t%F\t", XDEBUG_SECONDS_SINCE_START(nanotime));
	fprintf(context->trace_file, "%s", tmp);
	xdfree(tmp);
#if WIN32|WINNT
	fprintf(context->trace_file, "%Iu", zend_memory_usage(0));
#else
	fprintf(context->trace_file, "%zu", zend_memory_usage(0));
#endif
	fprintf(context->trace_file, "\n");

	str_time = xdebug_nanotime_to_chars(xdebug_get_nanotime(), 6);
	fprintf(context->trace_file, "TRACE END   [%s]\n\n", str_time);
	xdfree(str_time);

	fflush(context->trace_file);
}

char *xdebug_trace_computerized_get_filename(void *ctxt)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;

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
	int sent_variables = fse->varc;

	if (sent_variables > 0 && fse->var[sent_variables-1].is_variadic && Z_ISUNDEF(fse->var[sent_variables-1].data)) {
		sent_variables--;
	}

	/* Nr of arguments (11) */
	xdebug_str_add_fmt(line_entry, "\t%d", sent_variables);

	/* Arguments (12-...) */
	for (j = 0; j < sent_variables; j++) {
		xdebug_str_addc(line_entry, '\t');

		if (!Z_ISUNDEF(fse->var[j].data)) {
			add_single_value(line_entry, &(fse->var[j].data));
		} else {
			xdebug_str_add_literal(line_entry, "???");
		}
	}
}

void xdebug_trace_computerized_function_entry(void *ctxt, function_stack_entry *fse, int function_nr)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	char *tmp_name;
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	xdebug_str_add_fmt(&str, "%d\t", fse->level);
	xdebug_str_add_fmt(&str, "%d\t", function_nr);

	tmp_name = xdebug_show_fname(fse->function, XDEBUG_SHOW_FNAME_TODO);

	xdebug_str_add_literal(&str, "0\t");
	xdebug_str_add_fmt(&str, "%F\t", XDEBUG_SECONDS_SINCE_START(fse->nanotime));
	xdebug_str_add_fmt(&str, "%lu\t", fse->memory);
	xdebug_str_add_fmt(&str, "%s\t", tmp_name);
	if (fse->user_defined == XDEBUG_USER_DEFINED) {
		xdebug_str_add_literal(&str, "1\t");
	} else {
		xdebug_str_add_literal(&str, "0\t");
	}
	xdfree(tmp_name);

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

	/* Filename and Lineno (9, 10) */
	xdebug_str_add_fmt(&str, "\t%s\t%d", ZSTR_VAL(fse->filename), fse->lineno);

	add_arguments(&str, fse);

	/* Trailing \n */
	xdebug_str_addc(&str, '\n');

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);
	xdfree(str.d);
}

void xdebug_trace_computerized_function_exit(void *ctxt, function_stack_entry *fse, int function_nr)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	xdebug_str_add_fmt(&str, "%d\t", fse->level);
	xdebug_str_add_fmt(&str, "%d\t", function_nr);

	xdebug_str_add_literal(&str, "1\t");
	xdebug_str_add_fmt(&str, "%F\t", XDEBUG_SECONDS_SINCE_START(xdebug_get_nanotime()));
	xdebug_str_add_fmt(&str, "%lu\n", zend_memory_usage(0));

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);
	xdfree(str.d);
}

void xdebug_trace_computerized_function_return_value(void *ctxt, function_stack_entry *fse, int function_nr, zval *return_value)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	xdebug_str_add_fmt(&str, "%d\t", fse->level);
	xdebug_str_add_fmt(&str, "%d\t", function_nr);
	xdebug_str_add_literal(&str, "R\t\t\t");

	add_single_value(&str, return_value);

	xdebug_str_add_literal(&str, "\n");

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);
	xdfree(str.d);
}

void xdebug_trace_computerized_assignment(void *ctxt, function_stack_entry *fse, char *full_varname, zval *retval, char *right_full_varname, const char *op, char *filename, int lineno)
{
	xdebug_trace_computerized_context *context = (xdebug_trace_computerized_context*) ctxt;
	xdebug_str                         str = XDEBUG_STR_INITIALIZER;
	xdebug_str                        *tmp_value;

	xdebug_str_add_fmt(&str, "%d\t", fse->level);
	/* no function_nr */
	xdebug_str_add_literal(&str, "\t");

	xdebug_str_add_literal(&str, "A\t");
	/* skip time index, memory usage, function name, user defined */
	xdebug_str_add_literal(&str, "\t\t\t\t");

	/* Filename and Lineno (9, 10) */
	xdebug_str_add_fmt(&str, "\t%s\t%d", filename, lineno);
	xdebug_str_add_fmt(&str, "\t%s", full_varname);

	if (op[0] != '\0' ) { /* pre/post inc/dec ops are special */
		xdebug_str_addc(&str, ' ');
		xdebug_str_add(&str, op, 0);
		xdebug_str_addc(&str, ' ');

		tmp_value = xdebug_get_zval_value_line(retval, 0, NULL);

		if (tmp_value) {
			xdebug_str_add_str(&str, tmp_value);
			xdebug_str_free(tmp_value);
		} else {
			xdebug_str_add_literal(&str, "NULL");
		}
	}

	/* Trailing \n */
	xdebug_str_add_literal(&str, "\n");

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
	xdebug_trace_computerized_assignment
};
