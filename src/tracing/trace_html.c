/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2022 Derick Rethans                               |
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
#include "lib/php-header.h"

#include "php_xdebug.h"
#include "tracing_private.h"
#include "trace_html.h"

#include "lib/var.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

void *xdebug_trace_html_init(char *fname, zend_string *script_filename, long options)
{
	xdebug_trace_html_context *tmp_html_context;

	tmp_html_context = xdmalloc(sizeof(xdebug_trace_html_context));
	tmp_html_context->trace_file = xdebug_trace_open_file(fname, script_filename, options);

	if (!tmp_html_context->trace_file) {
		xdfree(tmp_html_context);
		return NULL;
	}

	return tmp_html_context;
}

void xdebug_trace_html_deinit(void *ctxt)
{
	xdebug_trace_html_context *context = (xdebug_trace_html_context*) ctxt;

	xdebug_file_close(context->trace_file);
	xdebug_file_dtor(context->trace_file);
	context->trace_file = NULL;

	xdfree(context);
}

void xdebug_trace_html_write_header(void *ctxt)
{
	xdebug_trace_html_context *context = (xdebug_trace_html_context*) ctxt;

	xdebug_file_printf(context->trace_file, "<table style='hyphens: auto; -webkit-hyphens: auto; -ms-hyphens: auto;' class='xdebug-trace' dir='ltr' border='1' cellspacing='0'>\n");
	xdebug_file_printf(context->trace_file, "\t<tr><th>#</th><th>Time</th>");
	xdebug_file_printf(context->trace_file, "<th>Mem</th>");
	xdebug_file_printf(context->trace_file, "<th colspan='2'>Function</th><th>Location</th></tr>\n");
	xdebug_file_flush(context->trace_file);
}

void xdebug_trace_html_write_footer(void *ctxt)
{
	xdebug_trace_html_context *context = (xdebug_trace_html_context*) ctxt;

	xdebug_file_printf(context->trace_file, "</table>\n");
	xdebug_file_flush(context->trace_file);
}

char *xdebug_trace_html_get_filename(void *ctxt)
{
	xdebug_trace_html_context *context = (xdebug_trace_html_context*) ctxt;

	return context->trace_file->name;
}

void xdebug_trace_html_function_entry(void *ctxt, function_stack_entry *fse, int function_nr)
{
	xdebug_trace_html_context *context = (xdebug_trace_html_context*) ctxt;
	char *tmp_name;
	unsigned int j;
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	xdebug_str_add_literal(&str, "\t<tr>");
	xdebug_str_add_fmt(&str, "<td>%d</td>", function_nr);
	xdebug_str_add_fmt(&str, "<td>%0.6F</td>", XDEBUG_SECONDS_SINCE_START(fse->nanotime));
	xdebug_str_add_fmt(&str, "<td align='right'>%lu</td>", fse->memory);
	xdebug_str_add_literal(&str, "<td align='left'>");
	for (j = 0; j < fse->level - 1; j++) {
		xdebug_str_add_literal(&str, "&nbsp; &nbsp;");
	}
	xdebug_str_add_literal(&str, "-&gt;</td>");

	tmp_name = xdebug_show_fname(fse->function, XDEBUG_SHOW_FNAME_DEFAULT);
	xdebug_str_add_fmt(&str, "<td>%s(", tmp_name);
	xdfree(tmp_name);

	if (fse->include_filename) {
		if (fse->function.type == XFUNC_EVAL) {
			xdebug_str       *joined;
			xdebug_arg       *parts;

			parts = xdebug_arg_ctor();
			xdebug_explode("\n", ZSTR_VAL(fse->include_filename), parts, 99999);
			joined = xdebug_join("<br />", parts, 0, 99999);
			xdebug_arg_dtor(parts);

			xdebug_str_add_fmt(&str, "'%s'", joined->d);
			xdebug_str_free(joined);
		} else {
			xdebug_str_add_zstr(&str, fse->include_filename);
		}
	}

	xdebug_str_add_fmt(&str, ")</td><td>%s:%d</td>", ZSTR_VAL(fse->filename), fse->lineno);
	xdebug_str_add_literal(&str, "</tr>\n");

	xdebug_file_printf(context->trace_file, "%s", str.d);
	xdebug_file_flush(context->trace_file);
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
	NULL /* xdebug_trace_html_generator_return_value */,
	NULL /* xdebug_trace_html_assignment */
};
