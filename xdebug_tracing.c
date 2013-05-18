/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   +----------------------------------------------------------------------+
 */
#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_str.h"
#include "xdebug_tracing.h"
#include "xdebug_var.h"
#include "ext/standard/php_string.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static char* return_trace_stack_frame_begin(function_stack_entry* i, int fnr TSRMLS_DC);
static char* return_trace_stack_frame_end(function_stack_entry* i, int fnr TSRMLS_DC);

void xdebug_trace_function_begin(function_stack_entry *fse, int function_nr TSRMLS_DC)
{
	if (XG(do_trace) && XG(trace_file)) {
		char *t = return_trace_stack_frame_begin(fse, function_nr TSRMLS_CC);
		if (fprintf(XG(trace_file), "%s", t) < 0) {
			fclose(XG(trace_file));
			XG(trace_file) = NULL;
		} else {
			fflush(XG(trace_file));
		}
		xdfree(t);
	}
}

void xdebug_trace_function_end(function_stack_entry *fse, int function_nr TSRMLS_DC)
{
	if (XG(do_trace) && XG(trace_file)) {
		char *t = return_trace_stack_frame_end(fse, function_nr TSRMLS_CC);
		if (fprintf(XG(trace_file), "%s", t) < 0) {
			fclose(XG(trace_file));
			XG(trace_file) = NULL;
		} else {
			fflush(XG(trace_file));
		}
		xdfree(t);
	}
}


char* xdebug_return_trace_assignment(function_stack_entry *i, char *varname, zval *retval, char *op, char *filename, int lineno TSRMLS_DC)
{
	int        j = 0;
	xdebug_str str = {0, 0, NULL};
	char      *tmp_value;

	if (XG(trace_format) != 0) {
		return xdstrdup("");
	}

	xdebug_str_addl(&str, "                    ", 20, 0);
	if (XG(show_mem_delta)) {
		xdebug_str_addl(&str, "        ", 8, 0);
	}
	for (j = 0; j <= i->level; j++) {
		xdebug_str_addl(&str, "  ", 2, 0);
	}
	xdebug_str_addl(&str, "   => ", 6, 0);

	xdebug_str_add(&str, varname, 0);

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

	return str.d;
}

static void xdebug_return_trace_stack_common(xdebug_str *str, function_stack_entry *i TSRMLS_DC)
{
	int        j = 0; /* Counter */

	xdebug_str_addl(str, "                    ", 20, 0);
	if (XG(show_mem_delta)) {
		xdebug_str_addl(str, "        ", 8, 0);
	}
	for (j = 0; j < i->level; j++) {
		xdebug_str_addl(str, "  ", 2, 0);
	}
	xdebug_str_addl(str, "   >=> ", 7, 0);
}

char* xdebug_return_trace_stack_retval(function_stack_entry* i, zval* retval TSRMLS_DC)
{
	xdebug_str str = {0, 0, NULL};
	char      *tmp_value;

	if (XG(trace_format) != 0) {
		return xdstrdup("");
	}

	xdebug_return_trace_stack_common(&str, i TSRMLS_CC);

	tmp_value = xdebug_get_zval_value(retval, 0, NULL);
	if (tmp_value) {
		xdebug_str_add(&str, tmp_value, 1);
	}
	xdebug_str_addl(&str, "\n", 2, 0);

	return str.d;
}

#if PHP_VERSION_ID >= 50500
char* xdebug_return_trace_stack_generator_retval(function_stack_entry* i, zend_generator* generator TSRMLS_DC)
{
	xdebug_str str = {0, 0, NULL};
	char      *tmp_value = NULL;

	if (XG(trace_format) != 0) {
		return xdstrdup("");
	}

	/* Generator key */
	tmp_value = xdebug_get_zval_value(generator->key, 0, NULL);
	if (tmp_value) {
		xdebug_return_trace_stack_common(&str, i TSRMLS_CC);

		xdebug_str_addl(&str, "(", 1, 0);
		xdebug_str_add(&str, tmp_value, 1);
		xdebug_str_addl(&str, " => ", 4, 0);

		tmp_value = xdebug_get_zval_value(generator->value, 0, NULL);
		if (tmp_value) {
			xdebug_str_add(&str, tmp_value, 1);
		}
		xdebug_str_addl(&str, ")", 1, 0);
		xdebug_str_addl(&str, "\n", 2, 0);
		return str.d;
	} else {
		return xdstrdup("");
	}
}
#endif

static char* return_trace_stack_frame_begin_normal(function_stack_entry* i TSRMLS_DC)
{
	int c = 0; /* Comma flag */
	int j = 0; /* Counter */
	char *tmp_name;
	xdebug_str str = {0, 0, NULL};

	tmp_name = xdebug_show_fname(i->function, 0, 0 TSRMLS_CC);

	xdebug_str_add(&str, xdebug_sprintf("%10.4f ", i->time - XG(start_time)), 1);
	xdebug_str_add(&str, xdebug_sprintf("%10lu ", i->memory), 1);
	if (XG(show_mem_delta)) {
		xdebug_str_add(&str, xdebug_sprintf("%+8ld ", i->memory - i->prev_memory), 1);
	}
	for (j = 0; j < i->level; j++) {
		xdebug_str_addl(&str, "  ", 2, 0);
	}
	xdebug_str_add(&str, xdebug_sprintf("-> %s(", tmp_name), 1);

	xdfree(tmp_name);

	/* Printing vars */
	if (XG(collect_params) > 0) {
		for (j = 0; j < i->varc; j++) {
			char *tmp_value;

			if (c) {
				xdebug_str_addl(&str, ", ", 2, 0);
			} else {
				c = 1;
			}

			if (i->var[j].name && XG(collect_params) >= 4) {
				xdebug_str_add(&str, xdebug_sprintf("$%s = ", i->var[j].name), 1);
			}

			switch (XG(collect_params)) {
				case 1: /* synopsis */
				case 2:
					tmp_value = xdebug_get_zval_synopsis(i->var[j].addr, 0, NULL);
					break;
				case 3:
				default:
					tmp_value = xdebug_get_zval_value(i->var[j].addr, 0, NULL);
					break;
			}
			if (tmp_value) {
				xdebug_str_add(&str, tmp_value, 1);
			} else {
				xdebug_str_add(&str, "???", 0);
			}
		}
	}

	if (i->include_filename) {
		if (i->function.type == XFUNC_EVAL) {
			int tmp_len;

			char *escaped;
			escaped = php_addcslashes(i->include_filename, strlen(i->include_filename), &tmp_len, 0, "'\\\0..\37", 6 TSRMLS_CC);
			xdebug_str_add(&str, xdebug_sprintf("'%s'", escaped), 1);
			efree(escaped);
		} else {
			xdebug_str_add(&str, i->include_filename, 0);
		}
	}

	xdebug_str_add(&str, xdebug_sprintf(") %s:%d\n", i->filename, i->lineno), 1);

	return str.d;
}

#define return_trace_stack_frame_begin_computerized(i,f)  return_trace_stack_frame_computerized((i), (f), 0 TSRMLS_CC)
#define return_trace_stack_frame_end_computerized(i,f)    return_trace_stack_frame_computerized((i), (f), 1 TSRMLS_CC)

static char* return_trace_stack_frame_computerized(function_stack_entry* i, int fnr, int whence TSRMLS_DC)
{
	char *tmp_name;
	xdebug_str str = {0, 0, NULL};

	xdebug_str_add(&str, xdebug_sprintf("%d\t", i->level), 1);
	xdebug_str_add(&str, xdebug_sprintf("%d\t", fnr), 1);
	if (whence == 0) { /* start */
		tmp_name = xdebug_show_fname(i->function, 0, 0 TSRMLS_CC);

		xdebug_str_add(&str, "0\t", 0);
		xdebug_str_add(&str, xdebug_sprintf("%f\t", i->time - XG(start_time)), 1);
#if HAVE_PHP_MEMORY_USAGE
		xdebug_str_add(&str, xdebug_sprintf("%lu\t", i->memory), 1);
#else
		xdebug_str_add(&str, "\t", 0);
#endif
		xdebug_str_add(&str, xdebug_sprintf("%s\t", tmp_name), 1);
		xdebug_str_add(&str, xdebug_sprintf("%d\t", i->user_defined == XDEBUG_EXTERNAL ? 1 : 0), 1);
		xdfree(tmp_name);

		if (i->include_filename) {
			if (i->function.type == XFUNC_EVAL) {
				int tmp_len;

				char *escaped;
				escaped = php_addcslashes(i->include_filename, strlen(i->include_filename), &tmp_len, 0, "'\\\0..\37", 6 TSRMLS_CC);
				xdebug_str_add(&str, xdebug_sprintf("'%s'", escaped), 1);
				efree(escaped);
			} else {
				xdebug_str_add(&str, i->include_filename, 0);
			}
		}

		/* Filename and Lineno (9, 10) */
		xdebug_str_add(&str, xdebug_sprintf("\t%s\t%d", i->filename, i->lineno), 1);


		if (XG(collect_params) > 0) {
			int j = 0; /* Counter */

			/* Nr of arguments (11) */
			xdebug_str_add(&str, xdebug_sprintf("\t%d", i->varc), 1);

			/* Arguments (12-...) */
			for (j = 0; j < i->varc; j++) {
				char *tmp_value;

				xdebug_str_addl(&str, "\t", 1, 0);

				if (i->var[j].name && XG(collect_params) >= 4) {
					xdebug_str_add(&str, xdebug_sprintf("$%s = ", i->var[j].name), 1);
				}

				switch (XG(collect_params)) {
					case 1: /* synopsis */
					case 2:
						tmp_value = xdebug_get_zval_synopsis(i->var[j].addr, 0, NULL);
						break;
					case 3:
					default:
						tmp_value = xdebug_get_zval_value(i->var[j].addr, 0, NULL);
						break;
				}
				if (tmp_value) {
					xdebug_str_add(&str, tmp_value, 1);
				} else {
					xdebug_str_add(&str, "???", 0);
				}
			}
		}

		/* Trailing \n */
		xdebug_str_add(&str, "\n", 0);

	} else if (whence == 1) { /* end */
		xdebug_str_add(&str, "1\t", 0);
		xdebug_str_add(&str, xdebug_sprintf("%f\t", xdebug_get_utime() - XG(start_time)), 1);
#if HAVE_PHP_MEMORY_USAGE
		xdebug_str_add(&str, xdebug_sprintf("%lu\n", XG_MEMORY_USAGE()), 1);
#else
		xdebug_str_add(&str, "\n", 0);
#endif
	}

	return str.d;
}

static char* return_trace_stack_frame_begin_html(function_stack_entry* i, int fnr TSRMLS_DC)
{
	char *tmp_name;
	int   j;
	xdebug_str str = {0, 0, NULL};

	xdebug_str_add(&str, "\t<tr>", 0);
	xdebug_str_add(&str, xdebug_sprintf("<td>%d</td>", fnr), 1);
	xdebug_str_add(&str, xdebug_sprintf("<td>%0.6f</td>", i->time - XG(start_time)), 1);
#if HAVE_PHP_MEMORY_USAGE
	xdebug_str_add(&str, xdebug_sprintf("<td align='right'>%lu</td>", i->memory), 1);
#endif
	xdebug_str_add(&str, "<td align='left'>", 0);
	for (j = 0; j < i->level - 1; j++) {
		xdebug_str_add(&str, "&nbsp; &nbsp;", 0);
	}
	xdebug_str_add(&str, "-&gt;</td>", 0);

	tmp_name = xdebug_show_fname(i->function, 0, 0 TSRMLS_CC);
	xdebug_str_add(&str, xdebug_sprintf("<td>%s(", tmp_name), 1);
	xdfree(tmp_name);

	if (i->include_filename) {
		if (i->function.type == XFUNC_EVAL) {
			char             *joined;
			xdebug_arg       *parts = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));

			xdebug_arg_init(parts);
			xdebug_explode("\n", i->include_filename, parts, 99999);
			joined = xdebug_join("<br />", parts, 0, 99999);
			xdebug_arg_dtor(parts);

			xdebug_str_add(&str, xdebug_sprintf("'%s'", joined), 1);
			xdfree(joined);
		} else {
			xdebug_str_add(&str, i->include_filename, 0);
		}
	}

	xdebug_str_add(&str, xdebug_sprintf(")</td><td>%s:%d</td>", i->filename, i->lineno), 1);
	xdebug_str_add(&str, "</tr>\n", 0);

	return str.d;
}


static char* return_trace_stack_frame_begin(function_stack_entry* i, int fnr TSRMLS_DC)
{
	switch (XG(trace_format)) {
		case 0:
			return return_trace_stack_frame_begin_normal(i TSRMLS_CC);
		case 1:
			return return_trace_stack_frame_begin_computerized(i, fnr);
		case 2:
			return return_trace_stack_frame_begin_html(i, fnr TSRMLS_CC);
		default:
			return xdstrdup("");
	}
}


static char* return_trace_stack_frame_end(function_stack_entry* i, int fnr TSRMLS_DC)
{
	switch (XG(trace_format)) {
		case 1:
			return return_trace_stack_frame_end_computerized(i, fnr);
		default:
			return xdstrdup("");
	}
}

PHP_FUNCTION(xdebug_start_trace)
{
	char *fname = NULL;
	int   fname_len = 0;
	char *trace_fname;
	long  options = XG(trace_options);

	if (XG(do_trace) == 0) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl", &fname, &fname_len, &options) == FAILURE) {
			return;
		}

		if ((trace_fname = xdebug_start_trace(fname, options TSRMLS_CC)) != NULL) {
			XG(do_trace) = 1;
			RETVAL_STRING(trace_fname, 1);
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

char* xdebug_start_trace(char* fname, long options TSRMLS_DC)
{
	char *str_time;
	char *filename;
	char *tmp_fname = NULL;

	if (fname && strlen(fname)) {
		filename = xdstrdup(fname);
	} else {
		if (!strlen(XG(trace_output_name)) ||
			xdebug_format_output_filename(&fname, XG(trace_output_name), NULL) <= 0
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
		XG(trace_file) = xdebug_fopen(filename, "a", "xt", (char**) &tmp_fname);
	} else {
		XG(trace_file) = xdebug_fopen(filename, "w", "xt", (char**) &tmp_fname);
	}
	xdfree(filename);
	if (options & XDEBUG_TRACE_OPTION_COMPUTERIZED) {
		XG(trace_format) = 1;
	}
	if (options & XDEBUG_TRACE_OPTION_HTML) {
		XG(trace_format) = 2;
	}
	if (XG(trace_file)) {
		if (XG(trace_format) == 1) {
			fprintf(XG(trace_file), "Version: %s\n", XDEBUG_VERSION);
			fprintf(XG(trace_file), "File format: 2\n");
		}
		if (XG(trace_format) == 0 || XG(trace_format) == 1) {
			str_time = xdebug_get_time();
			fprintf(XG(trace_file), "TRACE START [%s]\n", str_time);
			xdfree(str_time);
		}
		if (XG(trace_format) == 2) {
			fprintf(XG(trace_file), "<table class='xdebug-trace' dir='ltr' border='1' cellspacing='0'>\n");
			fprintf(XG(trace_file), "\t<tr><th>#</th><th>Time</th>");
#if HAVE_PHP_MEMORY_USAGE
			fprintf(XG(trace_file), "<th>Mem</th>");
#endif
			fprintf(XG(trace_file), "<th colspan='2'>Function</th><th>Location</th></tr>\n");
		}
		XG(do_trace) = 1;
		XG(tracefile_name) = tmp_fname;
		return xdstrdup(XG(tracefile_name));
	}
	return NULL;
}

void xdebug_stop_trace(TSRMLS_D)
{
	char   *str_time;
	double  u_time;
	char   *tmp;

	XG(do_trace) = 0;
	if (XG(trace_file)) {
		if (XG(trace_format) == 0 || XG(trace_format) == 1) {
			u_time = xdebug_get_utime();
			tmp = xdebug_sprintf(XG(trace_format) == 0 ? "%10.4f " : "\t\t\t%f\t", u_time - XG(start_time));
			fprintf(XG(trace_file), "%s", tmp);
			xdfree(tmp);
#if HAVE_PHP_MEMORY_USAGE
			fprintf(XG(trace_file), XG(trace_format) == 0 ? "%10zu" : "%lu", XG_MEMORY_USAGE());
#else
			fprintf(XG(trace_file), XG(trace_format) == 0 ? "%10u" : "", 0);
#endif
			fprintf(XG(trace_file), "\n");
			str_time = xdebug_get_time();
			fprintf(XG(trace_file), "TRACE END   [%s]\n\n", str_time);
			xdfree(str_time);
		}
		if (XG(trace_format) == 2) {
			fprintf(XG(trace_file), "</table>\n");
		}

		fclose(XG(trace_file));
		XG(trace_file) = NULL;
	}
	if (XG(tracefile_name)) {
		xdfree(XG(tracefile_name));
		XG(tracefile_name) = NULL;
	}
}

PHP_FUNCTION(xdebug_stop_trace)
{
	if (XG(do_trace) == 1) {
		RETVAL_STRING(XG(tracefile_name), 1);
		xdebug_stop_trace(TSRMLS_C);
	} else {
		RETVAL_FALSE;
		php_error(E_NOTICE, "Function trace was not started");
	}
}

PHP_FUNCTION(xdebug_get_tracefile_name)
{
	if (XG(tracefile_name)) {
		RETURN_STRING(XG(tracefile_name), 1);
	} else {
		RETURN_FALSE;
	}
}
