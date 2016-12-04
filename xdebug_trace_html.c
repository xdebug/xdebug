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
#include "xdebug_trace_html.h"
#include "xdebug_var.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

void *xdebug_trace_html_init(char *fname, long options TSRMLS_DC)
{
	xdebug_trace_html_context *tmp_html_context;
	char *used_fname;

	tmp_html_context = xdmalloc(sizeof(xdebug_trace_html_context));
	tmp_html_context->trace_file = xdebug_trace_open_file(fname, options, (char**) &used_fname TSRMLS_CC);
	tmp_html_context->trace_filename = used_fname;

	return tmp_html_context->trace_file ? tmp_html_context : NULL;
}

void xdebug_trace_html_deinit(void *ctxt TSRMLS_DC)
{
	xdebug_trace_html_context *context = (xdebug_trace_html_context*) ctxt;

	fclose(context->trace_file);
	context->trace_file = NULL;
	xdfree(context->trace_filename);

	xdfree(context);
}

void xdebug_trace_html_write_header(void *ctxt TSRMLS_DC)
{
	xdebug_trace_html_context *context = (xdebug_trace_html_context*) ctxt;

	fprintf(context->trace_file, "<table class='xdebug-trace' dir='ltr' border='1' cellspacing='0'>\n");
	fprintf(context->trace_file, "\t<tr><th>#</th><th>Time</th>");
	fprintf(context->trace_file, "<th>Mem</th>");
	if (XG(show_mem_delta)) {
		fprintf(context->trace_file, "<th>&#948; Mem</th>");
	}
	fprintf(context->trace_file, "<th colspan='2'>Function</th><th>Location</th></tr>\n");
	fflush(context->trace_file);
}

void xdebug_trace_html_write_footer(void *ctxt TSRMLS_DC)
{
	xdebug_trace_html_context *context = (xdebug_trace_html_context*) ctxt;

	fprintf(context->trace_file, "</table>\n");
	fflush(context->trace_file);
}

char *xdebug_trace_html_get_filename(void *ctxt TSRMLS_DC)
{
	xdebug_trace_html_context *context = (xdebug_trace_html_context*) ctxt;

	return context->trace_filename;
}

void xdebug_trace_html_function_entry(void *ctxt, function_stack_entry *fse, int function_nr TSRMLS_DC)
{
	xdebug_trace_html_context *context = (xdebug_trace_html_context*) ctxt;
	char *tmp_name;
	unsigned int j;
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	xdebug_str_add(&str, "\t<tr>", 0);
	xdebug_str_add(&str, xdebug_sprintf("<td>%d</td>", function_nr), 1);
	xdebug_str_add(&str, xdebug_sprintf("<td>%0.6F</td>", fse->time - XG(start_time)), 1);
	xdebug_str_add(&str, xdebug_sprintf("<td align='right'>%lu</td>", fse->memory), 1);
	if (XG(show_mem_delta)) {
		xdebug_str_add(&str, xdebug_sprintf("<td align='right'>%ld</td>", fse->memory - fse->prev_memory), 1);
	}
	xdebug_str_add(&str, "<td align='left'>", 0);
	for (j = 0; j < fse->level - 1; j++) {
		xdebug_str_add(&str, "&nbsp; &nbsp;", 0);
	}
	xdebug_str_add(&str, "-&gt;</td>", 0);

	tmp_name = xdebug_show_fname(fse->function, 0, 0 TSRMLS_CC);
	xdebug_str_add(&str, xdebug_sprintf("<td>%s(", tmp_name), 1);
	xdfree(tmp_name);

	if (fse->include_filename) {
		if (fse->function.type == XFUNC_EVAL) {
			char             *joined;
			xdebug_arg       *parts = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));

			xdebug_arg_init(parts);
			xdebug_explode("\n", fse->include_filename, parts, 99999);
			joined = xdebug_join("<br />", parts, 0, 99999);
			xdebug_arg_dtor(parts);

			xdebug_str_add(&str, xdebug_sprintf("'%s'", joined), 1);
			xdfree(joined);
		} else {
			xdebug_str_add(&str, fse->include_filename, 0);
		}
	}

	xdebug_str_add(&str, xdebug_sprintf(")</td><td>%s:%d</td>", fse->filename, fse->lineno), 1);
	xdebug_str_add(&str, "</tr>\n", 0);

	fprintf(context->trace_file, "%s", str.d);
	fflush(context->trace_file);
	xdfree(str.d);
}

xdebug_trace_handler_t xdebug_trace_handler_html =
{
	xdebug_trace_html_init,
	xdebug_trace_html_deinit,
	xdebug_trace_html_write_header,
	xdebug_trace_html_write_footer,
	xdebug_trace_html_get_filename,
	xdebug_trace_html_function_entry,
	NULL /* xdebug_trace_html_function_exit */,
	NULL /* xdebug_trace_html_function_return_value */,
#if PHP_VERSION_ID >= 50500
	NULL /* xdebug_trace_html_generator_return_value */,
#endif
	NULL /* xdebug_trace_html_assignment */
};
