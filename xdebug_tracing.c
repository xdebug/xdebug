/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2018 Derick Rethans                               |
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
#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_str.h"
#include "xdebug_tracing.h"
#include "xdebug_var.h"
#include "ext/standard/php_string.h"

#include "xdebug_compat.h"
#include "xdebug_tracing.h"
#include "xdebug_trace_textual.h"
#include "xdebug_trace_computerized.h"
#include "xdebug_trace_html.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

xdebug_trace_handler_t *xdebug_select_trace_handler(int options TSRMLS_DC)
{
	xdebug_trace_handler_t *tmp;

	switch (XG(trace_format)) {
		case 0: tmp = &xdebug_trace_handler_textual; break;
		case 1: tmp = &xdebug_trace_handler_computerized; break;
		case 2: tmp = &xdebug_trace_handler_html; break;
		default:
			php_error(E_NOTICE, "A wrong value for xdebug.trace_format was selected (%d), defaulting to the textual format", (int) XG(trace_format));
			tmp = &xdebug_trace_handler_textual; break;
	}

	if (options & XDEBUG_TRACE_OPTION_COMPUTERIZED) {
		tmp = &xdebug_trace_handler_computerized;
	}
	if (options & XDEBUG_TRACE_OPTION_HTML) {
		tmp = &xdebug_trace_handler_html;
	}

	return tmp;
}

FILE *xdebug_trace_open_file(char *fname, char *script_filename, long options, char **used_fname TSRMLS_DC)
{
	FILE *file;
	char *filename;

	if (fname && strlen(fname)) {
		filename = xdstrdup(fname);
	} else {
		if (!strlen(XG(trace_output_name)) ||
			xdebug_format_output_filename(&fname, XG(trace_output_name), script_filename) <= 0
		) {
			/* Invalid or empty xdebug.trace_output_name */
			return NULL;
		}
		if (IS_SLASH(XG(trace_output_dir)[strlen(XG(trace_output_dir)) - 1])) {
			filename = xdebug_sprintf("%s%s", XG(trace_output_dir), fname);
		} else {
			filename = xdebug_sprintf("%s%c%s", XG(trace_output_dir), DEFAULT_SLASH, fname);
		}
		xdfree(fname);
	}
	if (options & XDEBUG_TRACE_OPTION_APPEND) {
		file = xdebug_fopen(filename, "a", (options & XDEBUG_TRACE_OPTION_NAKED_FILENAME) ? NULL : "xt", used_fname);
	} else {
		file = xdebug_fopen(filename, "w", (options & XDEBUG_TRACE_OPTION_NAKED_FILENAME) ? NULL : "xt", used_fname);
	}
	xdfree(filename);

	return file;
}

char* xdebug_start_trace(char* fname, char *script_filename, long options TSRMLS_DC)
{
	XG(trace_handler) = xdebug_select_trace_handler(options TSRMLS_CC);
	XG(trace_context) = (void*) XG(trace_handler)->init(fname, script_filename, options TSRMLS_CC);

	if (XG(trace_context)) {
		XG(do_trace) = 1;
		XG(trace_handler)->write_header(XG(trace_context) TSRMLS_CC);
		return xdstrdup(XG(trace_handler)->get_filename(XG(trace_context) TSRMLS_CC));
	}

	return NULL;
}

void xdebug_stop_trace(TSRMLS_D)
{
	XG(do_trace) = 0;
	if (XG(trace_context)) {
		XG(trace_handler)->write_footer(XG(trace_context) TSRMLS_CC);
		XG(trace_handler)->deinit(XG(trace_context) TSRMLS_CC);
		XG(trace_context) = NULL;
	}
}

PHP_FUNCTION(xdebug_start_trace)
{
	char *fname = NULL;
	size_t fname_len = 0;
	char *trace_fname;
	zend_long options = XG(trace_options);

	if (XG(do_trace) == 0) {
		function_stack_entry *fse;

		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl", &fname, &fname_len, &options) == FAILURE) {
			return;
		}

		fse = xdebug_get_stack_frame(0 TSRMLS_CC);

		if ((trace_fname = xdebug_start_trace(fname, fse->filename, options TSRMLS_CC)) != NULL) {
			XG(do_trace) = 1;
			RETVAL_STRING(trace_fname);
			xdfree(trace_fname);
			return;
		} else {
			php_error(E_NOTICE, "Trace could not be started");
		}

		XG(do_trace) = 0;
		RETURN_FALSE;
	} else {
		php_error(E_NOTICE, "Function trace already started");
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_stop_trace)
{
	if (XG(do_trace) == 1) {
		RETVAL_STRING(XG(trace_handler)->get_filename(XG(trace_context) TSRMLS_CC));
		xdebug_stop_trace(TSRMLS_C);
	} else {
		RETVAL_FALSE;
		php_error(E_NOTICE, "Function trace was not started");
	}
}

PHP_FUNCTION(xdebug_get_tracefile_name)
{
	if (XG(do_trace) == 1 && XG(trace_handler) && XG(trace_handler)->get_filename) {
		RETVAL_STRING(XG(trace_handler)->get_filename(XG(trace_context) TSRMLS_CC));
	} else {
		RETURN_FALSE;
	}
}
