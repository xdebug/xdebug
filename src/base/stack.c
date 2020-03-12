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
#include "php_xdebug.h"

#include "main/php_ini.h"

#include "ext/standard/html.h"
#include "ext/standard/php_smart_string.h"
#include "zend_exceptions.h"
#include "zend_generators.h"

#include "stack.h"

#include "base/filter.h"
#include "base/monitor.h"
#include "base/superglobals.h"
#include "coverage/code_coverage.h"
#include "lib/compat.h"
#include "lib/lib.h"
#include "lib/str.h"
#include "lib/var_export_html.h"
#include "lib/var_export_line.h"
#include "lib/var_export_serialized.h"
#include "profiler/profiler.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static const char* text_formats[11] = {
	"\n",
	"%s: %s in %s on line %d\n",
	"\nCall Stack:\n",
	"%10.4F %10ld %3d. %s(",
	"'%s'",
	") %s:%d\n",
	"\n\nVariables in local scope (#%d):\n",
	"\n",
	"  $%s = %s\n",
	"  $%s = *uninitialized*\n",
	"SCREAM:  Error suppression ignored for\n"
};

static const char* ansi_formats[11] = {
	"\n",
	"[1m[31m%s[0m: %s[22m in [31m%s[0m on line [32m%d[0m[22m\n",
	"\n[1mCall Stack:[22m\n",
	"%10.4F %10ld %3d. %s(",
	"'%s'",
	") %s:%d\n",
	"\n\nVariables in local scope (#%d):\n",
	"\n",
	"  $%s = %s\n",
	"  $%s = *uninitialized*\n",
	"[1m[31mSCREAM[0m:  Error suppression ignored for\n"
};

static const char* html_formats[13] = {
	"<br />\n<font size='1'><table class='xdebug-error xe-%s%s' dir='ltr' border='1' cellspacing='0' cellpadding='1'>\n",
	"<tr><th align='left' bgcolor='#f57900' colspan=\"5\"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> %s: %s in %s on line <i>%d</i></th></tr>\n",
	"<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>\n<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>\n",
	"<tr><td bgcolor='#eeeeec' align='center'>%d</td><td bgcolor='#eeeeec' align='center'>%.4F</td><td bgcolor='#eeeeec' align='right'>%ld</td><td bgcolor='#eeeeec'>%s( ",
	"<font color='#00bb00'>'%s'</font>",
	" )</td><td title='%s' bgcolor='#eeeeec'>%s<b>:</b>%d</td></tr>\n",
	"<tr><th align='left' colspan='5' bgcolor='#e9b96e'>Variables in local scope (#%d)</th></tr>\n",
	"</table></font>\n",
	"<tr><td colspan='2' align='right' bgcolor='#eeeeec' valign='top'><pre>$%s&nbsp;=</pre></td><td colspan='3' bgcolor='#eeeeec'>%s</td></tr>\n",
	"<tr><td colspan='2' align='right' bgcolor='#eeeeec' valign='top'><pre>$%s&nbsp;=</pre></td><td colspan='3' bgcolor='#eeeeec' valign='top'><i>Undefined</i></td></tr>\n",
	" )</td><td title='%s' bgcolor='#eeeeec'><a style='color: black' href='%s'>%s<b>:</b>%d</a></td></tr>\n",
	"<tr><th align='left' bgcolor='#f57900' colspan=\"5\"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> %s: %s in <a style='color: black' href='%s'>%s</a> on line <i>%d</i></th></tr>\n",
	"<tr><th align='left' bgcolor='#f57900' colspan=\"5\"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> SCREAM: Error suppression ignored for</th></tr>\n"
};

static const char** select_formats(int html) {
	if (html) {
		return html_formats;
	}
	else if ((XINI_BASE(cli_color) == 1 && xdebug_is_output_tty()) || (XINI_BASE(cli_color) == 2)) {
		return ansi_formats;
	}
	else {
		return text_formats;
	}
}

void xdebug_log_stack(const char *error_type_str, char *buffer, const char *error_filename, const int error_lineno)
{
	xdebug_llist_element *le;
	function_stack_entry *i;
	char                 *tmp_log_message;

	tmp_log_message = xdebug_sprintf( "PHP %s:  %s in %s on line %d", error_type_str, buffer, error_filename, error_lineno);
	php_log_err(tmp_log_message);
	xdfree(tmp_log_message);

	if (XG_BASE(stack) && XG_BASE(stack)->size) {
		php_log_err((char*) "PHP Stack trace:");

		for (le = XDEBUG_LLIST_HEAD(XG_BASE(stack)); le != NULL; le = XDEBUG_LLIST_NEXT(le))
		{
			int c = 0; /* Comma flag */
			unsigned int j = 0; /* Counter */
			char *tmp_name;
			xdebug_str log_buffer = XDEBUG_STR_INITIALIZER;
			int variadic_opened = 0;

			i = XDEBUG_LLIST_VALP(le);
			tmp_name = xdebug_show_fname(i->function, 0, 0);
			xdebug_str_add(&log_buffer, xdebug_sprintf("PHP %3d. %s(", i->level, tmp_name), 1);
			xdfree(tmp_name);

			/* Printing vars */
			for (j = 0; j < i->varc; j++) {
				char       *tmp_varname;
				xdebug_str *tmp_value;

				if (c) {
					xdebug_str_addl(&log_buffer, ", ", 2, 0);
				} else {
					c = 1;
				}

				if (
					(i->var[j].is_variadic && XINI_BASE(collect_params) != 5)
				) {
					xdebug_str_add(&log_buffer, "...", 0);
					variadic_opened = 1;
				}

				tmp_varname = i->var[j].name ? xdebug_sprintf("$%s = ", i->var[j].name) : xdstrdup("");
				xdebug_str_add(&log_buffer, tmp_varname, 0);
				xdfree(tmp_varname);

				if (i->var[j].is_variadic) {
					xdebug_str_add(&log_buffer, "variadic(", 0);
					c = 0;
					continue;
				}

				if (!Z_ISUNDEF(i->var[j].data)) {
					tmp_value = xdebug_get_zval_value_line(&i->var[j].data, 0, NULL);
					xdebug_str_add_str(&log_buffer, tmp_value);
					xdebug_str_free(tmp_value);
				} else {
					xdebug_str_addl(&log_buffer, "*uninitialized*", 15, 0);
				}
			}

			if (variadic_opened) {
				xdebug_str_add(&log_buffer, ")", 0);
			}

			xdebug_str_add(&log_buffer, xdebug_sprintf(") %s:%d", i->filename, i->lineno), 1);
			php_log_err(log_buffer.d);
			xdebug_str_destroy(&log_buffer);
		}
	}
}

void xdebug_append_error_head(xdebug_str *str, int html, const char *error_type_str)
{
	const char **formats = select_formats(html);

	if (html) {
		xdebug_str_add(str, xdebug_sprintf(formats[0], error_type_str, XG_BASE(in_at) ? " xe-scream" : ""), 1);
		if (XG_BASE(in_at)) {
			xdebug_str_add(str, formats[12], 0);
		}
	} else {
		xdebug_str_add(str, formats[0], 0);
		if (XG_BASE(in_at)) {
			xdebug_str_add(str, formats[10], 0);
		}
	}
}

void xdebug_append_error_description(xdebug_str *str, int html, const char *error_type_str, const char *buffer, const char *error_filename, const int error_lineno)
{
	const char **formats = select_formats(html);
	char *escaped;

	if (html) {
		zend_string *tmp;
		char *first_closing = strchr(buffer, ']');

		/* We do need to escape HTML entities here, as HTML chars could be in
		 * the error message. However, PHP in some circumstances also adds an
		 * HTML link to a manual page. That bit, we don't need to escape. So
		 * this bit of code finds the portion that doesn't need escaping, adds
		 * it to a tmp string, and then adds an HTML escaped string for the
		 * rest of the original buffer. */
		if (first_closing && strstr(buffer, "() [<a href=") != NULL) {
			smart_string special_escaped = { 0, 0, 0 };

			*first_closing = '\0';
			first_closing++;

			smart_string_appends(&special_escaped, buffer);
			tmp = php_escape_html_entities((unsigned char *) first_closing, strlen(first_closing), 0, 0, NULL);
			smart_string_appends(&special_escaped, tmp->val);
			zend_string_free(tmp);

			smart_string_0(&special_escaped);
			escaped = estrdup(special_escaped.c);
			smart_string_free(&special_escaped);
		} else if (strncmp(buffer, "assert()", 8) == 0) {
			/* Also don't escape if we're in an assert, as things are already
			 * escaped. It's all nice and consistent ey? */
			escaped = estrdup(buffer);
		} else {
			tmp = php_escape_html_entities((unsigned char *) buffer, strlen(buffer), 0, 0, NULL);
			escaped = estrdup(tmp->val);
			zend_string_free(tmp);
		}
	} else {
		escaped = estrdup(buffer);
	}

	if (strlen(XINI_BASE(file_link_format)) > 0 && html) {
		char *file_link;

		xdebug_format_file_link(&file_link, error_filename, error_lineno);
		xdebug_str_add(str, xdebug_sprintf(formats[11], error_type_str, escaped, file_link, error_filename, error_lineno), 1);
		xdfree(file_link);
	} else {
		xdebug_str_add(str, xdebug_sprintf(formats[1], error_type_str, escaped, error_filename, error_lineno), 1);
	}

	efree(escaped);
}

static void add_single_value(xdebug_str *str, zval *zv, int html, int collecton_level)
{
	xdebug_str *tmp_value = NULL, *tmp_html_synop_value = NULL;
	char       *tmp_html_value = NULL;
	size_t      newlen;

	if (html) {
		switch (collecton_level) {
			case 1: /* synopsis */
				tmp_html_synop_value = xdebug_get_zval_synopsis_html("", zv, 0, NULL);

				xdebug_str_addl(str, "<span>", 6, 0);
				xdebug_str_add_str(str, tmp_html_synop_value);
				xdebug_str_addl(str, "</span>", 7, 0);

				xdfree(tmp_html_synop_value);
				break;
			case 2: /* synopsis + full in tooltip */
				tmp_value = xdebug_get_zval_value_line(zv, 0, NULL);
				tmp_html_value = xdebug_xmlize(tmp_value->d, tmp_value->l, &newlen);
				tmp_html_synop_value = xdebug_get_zval_synopsis_html("", zv, 0, NULL);

				xdebug_str_addl(str, "<span title='", 13, 0);
				xdebug_str_add(str, tmp_html_value, 0);
				xdebug_str_addl(str, "'>", 2, 0);
				xdebug_str_add_str(str, tmp_html_synop_value);
				xdebug_str_addl(str, "</span>", 7, 0);

				xdebug_str_free(tmp_value);
				efree(tmp_html_value);
				xdebug_str_free(tmp_html_synop_value);
				break;
			case 3: /* full */
			case 4: /* full (with var_name) */
			default:
				tmp_value = xdebug_get_zval_value_line(zv, 0, NULL);
				tmp_html_value = xdebug_xmlize(tmp_value->d, tmp_value->l, &newlen);

				xdebug_str_addl(str, "<span>", 6, 0);
				xdebug_str_add(str, tmp_html_value, 0);
				xdebug_str_addl(str, "</span>", 7, 0);

				xdebug_str_free(tmp_value);
				efree(tmp_html_value);
				break;
			case 5: { /* serialized */
				tmp_value = xdebug_get_zval_value_serialized(zv, 0, NULL);

				xdebug_str_addl(str, "<span>", 6, 0);
				xdebug_str_add_str(str, tmp_value);
				xdebug_str_addl(str, "</span>", 7, 0);

				xdebug_str_free(tmp_value);
			} break;
		}
	} else {
		switch (collecton_level) {
			case 1: /* synopsis */
			case 2:
				tmp_value = xdebug_get_zval_synopsis_line(zv, 0, NULL);
				break;
			case 3: /* full */
			case 4: /* full (with var_name) */
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
			xdebug_str_addl(str, "???", 3, 0);
		}
	}
}

void xdebug_append_printable_stack(xdebug_str *str, int html)
{
	xdebug_llist_element *le;
	function_stack_entry *i;
	int                   printed_frames = 0;
	const char          **formats = select_formats(html);

	if (XG_BASE(stack) && XG_BASE(stack)->size) {
		i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_HEAD(XG_BASE(stack)));

		xdebug_str_add(str, formats[2], 0);

		for (le = XDEBUG_LLIST_HEAD(XG_BASE(stack)); le != NULL; le = XDEBUG_LLIST_NEXT(le))
		{
			int c = 0; /* Comma flag */
			unsigned int j = 0; /* Counter */
			char *tmp_name;
			int variadic_opened = 0;

			i = XDEBUG_LLIST_VALP(le);
			if (xdebug_is_stack_frame_filtered(XDEBUG_FILTER_TRACING, i)) {
				continue;
			}
			tmp_name = xdebug_show_fname(i->function, html, 0);
			if (html) {
				xdebug_str_add(str, xdebug_sprintf(formats[3], i->level, i->time - XG_BASE(start_time), i->memory, tmp_name), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf(formats[3], i->time - XG_BASE(start_time), i->memory, i->level, tmp_name), 1);
			}
			xdfree(tmp_name);

			/* Printing vars */
			for (j = 0; j < i->varc; j++) {
				if (c) {
					xdebug_str_addl(str, ", ", 2, 0);
				} else {
					c = 1;
				}

				if (
					(i->var[j].is_variadic && Z_ISUNDEF(i->var[j].data))
				) {
					xdebug_str_add(str, "...", 0);
				}

				if (i->var[j].name && XINI_BASE(collect_params) == 4) {
					if (html) {
						xdebug_str_add(str, xdebug_sprintf("<span>$%s = </span>", i->var[j].name), 1);
					} else {
						xdebug_str_add(str, xdebug_sprintf("$%s = ", i->var[j].name), 1);
					}
				}

				if (!variadic_opened && i->var[j].is_variadic && Z_ISUNDEF(i->var[j].data)) {
					if (html) {
						xdebug_str_add(str, "<i>variadic</i>(", 0);
					} else {
						xdebug_str_add(str, "variadic(", 0);
					}
					c = 0;
					variadic_opened = 1;
					continue;
				}

				if (!Z_ISUNDEF(i->var[j].data)) {
					add_single_value(str, &i->var[j].data, html, XINI_BASE(collect_params));
				} else {
					xdebug_str_addl(str, "???", 3, 0);
				}
			}

			if (variadic_opened) {
				xdebug_str_add(str, ")", 0);
			}

			if (i->include_filename) {
				xdebug_str_add(str, xdebug_sprintf(formats[4], i->include_filename), 1);
			}

			if (html) {
				char *formatted_filename;
				xdebug_format_filename(&formatted_filename, XINI_BASE(filename_format), "...%s%n", i->filename);

				if (strlen(XINI_BASE(file_link_format)) > 0) {
					char *file_link;

					xdebug_format_file_link(&file_link, i->filename, i->lineno);
					xdebug_str_add(str, xdebug_sprintf(formats[10], i->filename, file_link, formatted_filename, i->lineno), 1);
					xdfree(file_link);
				} else {
					xdebug_str_add(str, xdebug_sprintf(formats[5], i->filename, formatted_filename, i->lineno), 1);
				}

				xdfree(formatted_filename);
			} else {
				xdebug_str_add(str, xdebug_sprintf(formats[5], i->filename, i->lineno), 1);
			}

			printed_frames++;
			if (XINI_BASE(max_stack_frames) > 0 && printed_frames >= XINI_BASE(max_stack_frames)) {
				break;
			}
		}

		if (XINI_BASE(dump_globals) && !(XINI_BASE(dump_once) && XG_BASE(dumped))) {
			char *tmp = xdebug_get_printable_superglobals(html);

			if (tmp) {
				xdebug_str_add(str, tmp, 1);
			}
			XG_BASE(dumped) = 1;
		}

		if (XINI_BASE(show_local_vars) && XG_BASE(stack) && XDEBUG_LLIST_TAIL(XG_BASE(stack))) {
			int scope_nr = XG_BASE(stack)->size;

			i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack)));
			if (i->user_defined == XDEBUG_BUILT_IN && XDEBUG_LLIST_PREV(XDEBUG_LLIST_TAIL(XG_BASE(stack))) && XDEBUG_LLIST_VALP(XDEBUG_LLIST_PREV(XDEBUG_LLIST_TAIL(XG_BASE(stack))))) {
				i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_PREV(XDEBUG_LLIST_TAIL(XG_BASE(stack))));
				scope_nr--;
			}
			if (i->declared_vars && i->declared_vars->size) {
				xdebug_hash *tmp_hash;

				xdebug_str_add(str, xdebug_sprintf(formats[6], scope_nr), 1);
				tmp_hash = xdebug_declared_var_hash_from_llist(i->declared_vars);
				xdebug_hash_apply_with_argument(tmp_hash, (void*) &html, xdebug_dump_used_var_with_contents, (void *) str);
				xdebug_hash_destroy(tmp_hash);
			}
		}
	}
}

void xdebug_append_error_footer(xdebug_str *str, int html)
{
	const char **formats = select_formats(html);

	xdebug_str_add(str, formats[7], 0);
}

static char *get_printable_stack(int html, int error_type, const char *buffer, const char *error_filename, const int error_lineno, int include_decription)
{
	char *prepend_string;
	char *append_string;
	char *error_type_str = xdebug_error_type(error_type);
	char *error_type_str_simple = xdebug_error_type_simple(error_type);
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	prepend_string = INI_STR((char*) "error_prepend_string");
	append_string = INI_STR((char*) "error_append_string");

	xdebug_str_add(&str, prepend_string ? prepend_string : "", 0);
	xdebug_append_error_head(&str, html, error_type_str_simple);
	if (include_decription) {
		xdebug_append_error_description(&str, html, error_type_str, buffer, error_filename, error_lineno);
	}
	xdebug_append_printable_stack(&str, html);
	xdebug_append_error_footer(&str, html);
	xdebug_str_add(&str, append_string ? append_string : "", 0);

	xdfree(error_type_str);
	xdfree(error_type_str_simple);

	return str.d;
}

static void php_output_error(const char *error)
{
#ifdef PHP_DISPLAY_ERRORS_STDERR
	if (PG(display_errors) == PHP_DISPLAY_ERRORS_STDERR) {
		fputs(error, stderr);
		fflush(stderr);
		return;
	}
#endif
	php_printf("%s", error);
}

char *xdebug_strip_php_stack_trace(char *buffer)
{
	char *tmp_buf, *p;

	if (strncmp(buffer, "Uncaught ", 9) == 0) {
		/* find first new line */
		p = strchr(buffer, '\n');
		if (!p) {
			p = buffer + strlen(buffer);
		} else {
			/* find the last " in ", which isn't great and might not work... but in most cases it will */
			p = xdebug_strrstr(buffer, " in ");
			if (!p) {
				p = buffer + strlen(buffer);
			}
		}
		/* Create new buffer */
		tmp_buf = calloc(p - buffer + 1, 1);
		strncpy(tmp_buf, buffer, p - buffer);

		return tmp_buf;
	}
	return NULL;
}

static char *xdebug_handle_stack_trace(int type, char *error_type_str, const char *error_filename, const unsigned int error_lineno, char *buffer)
{
	char *printable_stack;
	char *tmp_buf;

	/* We need to see if we have an uncaught exception fatal error now */
	if (type == E_ERROR && ((tmp_buf = xdebug_strip_php_stack_trace(buffer)) != NULL)) {
		xdebug_str str = XDEBUG_STR_INITIALIZER;

		/* Append error */
		xdebug_append_error_head(&str, PG(html_errors), "uncaught-exception");
		xdebug_append_error_description(&str, PG(html_errors), error_type_str, tmp_buf, error_filename, error_lineno);
		xdebug_append_printable_stack(&str, PG(html_errors));
		if (XG_BASE(last_exception_trace)) {
			xdebug_str_add(&str, XG_BASE(last_exception_trace), 0);
		}
		xdebug_append_error_footer(&str, PG(html_errors));

		free(tmp_buf);
		printable_stack = str.d;
	} else {
		printable_stack = get_printable_stack(PG(html_errors), type, buffer, error_filename, error_lineno, 1);
	}

	return printable_stack;
}

/* Error callback for formatting stack traces */
void xdebug_error_cb(int orig_type, const char *error_filename, const unsigned int error_lineno, const char *format, va_list args)
{
	char *buffer, *error_type_str;
	int buffer_len, display;
	error_handling_t  error_handling;
	zend_class_entry *exception_class;
#if PHP_VERSION_ID >= 80000
	int type = orig_type & E_ALL;
#else
	int type = orig_type;
#endif

	buffer_len = vspprintf(&buffer, PG(log_errors_max_len), format, args);

	error_type_str = xdebug_error_type(type);

	/* check for repeated errors to be ignored */
	if (PG(ignore_repeated_errors) && PG(last_error_message)) {
			/* no check for PG(last_error_file) is needed since it cannot
			 * be NULL if PG(last_error_message) is not NULL */
			if (strcmp(PG(last_error_message), buffer)
					|| (!PG(ignore_repeated_source)
							&& ((PG(last_error_lineno) != (int)error_lineno)
									|| strcmp(PG(last_error_file), error_filename)))) {
					display = 1;
			} else {
					display = 0;
			}
	} else {
			display = 1;
	}

#if PHP_VERSION_ID < 70300
	/* Store last error message for error_get_last() */
	if (display) {
		if (PG(last_error_message)) {
			char *s = PG(last_error_message);
			PG(last_error_message) = NULL;
			free(s);
		}
		if (PG(last_error_file)) {
			char *s = PG(last_error_file);
			PG(last_error_file) = NULL;
			free(s);
		}
		if (!error_filename) {
			error_filename = "Unknown";
		}
		PG(last_error_type) = type;
		PG(last_error_message) = strdup(buffer);
		PG(last_error_file) = strdup(error_filename);
		PG(last_error_lineno) = error_lineno;
	}
#endif
	error_handling  = EG(error_handling);
	exception_class = EG(exception_class);
#if PHP_VERSION_ID >= 70300
	/* according to error handling mode, throw exception or show it */
	if (error_handling == EH_THROW) {
#else
	/* according to error handling mode, suppress error, throw exception or show it */
	if (error_handling != EH_NORMAL) {
#endif
		switch (type) {
			case E_ERROR:
			case E_CORE_ERROR:
			case E_COMPILE_ERROR:
			case E_USER_ERROR:
			case E_PARSE:
				/* fatal errors are real errors and cannot be made exceptions */
				break;
			case E_STRICT:
			case E_DEPRECATED:
			case E_USER_DEPRECATED:
				/* for the sake of BC to old damaged code */
				break;
			case E_NOTICE:
			case E_USER_NOTICE:
				/* notices are no errors and are not treated as such like E_WARNINGS */
				break;
			default:
				/* throw an exception if we are in EH_THROW mode
				 * but DO NOT overwrite a pending exception
				 */
#if PHP_VERSION_ID >= 70300
				if (!EG(exception)) {
#else
				if (error_handling == EH_THROW && !EG(exception)) {
#endif
					zend_throw_error_exception(exception_class, buffer, 0, type);
				}
				efree(buffer);
				xdfree(error_type_str);
				return;
		}
	}

#if PHP_VERSION_ID >= 70300
	/* Store last error message for error_get_last() */
	if (display) {
		if (PG(last_error_message)) {
			char *s = PG(last_error_message);
			PG(last_error_message) = NULL;
			free(s);
		}
		if (PG(last_error_file)) {
			char *s = PG(last_error_file);
			PG(last_error_file) = NULL;
			free(s);
		}
		if (!error_filename) {
			error_filename = "Unknown";
		}
		PG(last_error_type) = type;
		PG(last_error_message) = strdup(buffer);
		PG(last_error_file) = strdup(error_filename);
		PG(last_error_lineno) = error_lineno;
	}
#endif

	if ((EG(error_reporting | XINI_BASE(force_error_reporting))) & type) {
		/* Log to logger */
		if (PG(log_errors)) {

#ifdef PHP_WIN32
			if (type==E_CORE_ERROR || type==E_CORE_WARNING) {
				MessageBox(NULL, buffer, error_type_str, MB_OK);
			}
#endif
			xdebug_log_stack(error_type_str, buffer, error_filename, error_lineno);
			if (XINI_BASE(dump_globals) && !(XINI_BASE(dump_once) && XG_BASE(dumped))) {
				char *printable_stack = xdebug_get_printable_superglobals(0);

				if (printable_stack) {
					int pc;

					xdebug_arg *parts = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));

					xdebug_arg_init(parts);
					xdebug_explode("\n", printable_stack, parts, -1);

					for (pc = 0; pc < parts->c; pc++) {
						char *tmp_line = xdebug_sprintf("PHP %s", parts->args[pc]);
						php_log_err(tmp_line);
						xdfree(tmp_line);
					}

					xdebug_arg_dtor(parts);
					xdfree(printable_stack);
					php_log_err((char*) "PHP ");
				}
			}
		}

		/* Display errors */
		if ((PG(display_errors) || XINI_BASE(force_display_errors)) && !PG(during_request_startup)) {
			char *printable_stack;

			printable_stack = xdebug_handle_stack_trace(type, error_type_str, error_filename, error_lineno, buffer);

			if (XG_BASE(do_collect_errors) && (type != E_ERROR) && (type != E_COMPILE_ERROR) && (type != E_USER_ERROR)) {
				xdebug_llist_insert_next(XG_BASE(collected_errors), XDEBUG_LLIST_TAIL(XG_BASE(collected_errors)), printable_stack);
			} else {
				php_output_error(printable_stack);
				xdfree(printable_stack);
			}
		} else if (XG_BASE(do_collect_errors)) {
			char *printable_stack;
			printable_stack = get_printable_stack(PG(html_errors), type, buffer, error_filename, error_lineno, 1);
			xdebug_llist_insert_next(XG_BASE(collected_errors), XDEBUG_LLIST_TAIL(XG_BASE(collected_errors)), printable_stack);
		}
	}

	xdebug_debugger_error_cb(error_filename, error_lineno, type, error_type_str, buffer);

	xdfree(error_type_str);

	if (type & XINI_BASE(halt_level) & XDEBUG_ALLOWED_HALT_LEVELS) {
		type = E_USER_ERROR;
	}

	/* Bail out if we can't recover */
	switch (type) {
		case E_CORE_ERROR:
			if (!php_get_module_initialized()) {
				/* bad error in module startup - no way we can live with this */
				exit(-2);
			}
			XDEBUG_BREAK_INTENTIONALLY_MISSING

		case E_ERROR:
		case E_RECOVERABLE_ERROR:
		case E_PARSE:
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			EG(exit_status) = 255;
			if (php_get_module_initialized()) {
				if (!PG(display_errors) &&
				    !SG(headers_sent) &&
					SG(sapi_headers).http_response_code == 200
				) {
					sapi_header_line ctr = { 0, 0, 0 };

					ctr.line = (char*) "HTTP/1.0 500 Internal Server Error";
					ctr.line_len = sizeof("HTTP/1.0 500 Internal Server Error") - 1;
					sapi_header_op(SAPI_HEADER_REPLACE, &ctr);
				}
				/* the parser would return 1 (failure), we can bail out nicely */
#if PHP_VERSION_ID >= 80000
				if (!(orig_type & E_DONT_BAIL)) {
#else
				if (type != E_PARSE) {
#endif
					/* restore memory limit */
					zend_set_memory_limit(PG(memory_limit));
					efree(buffer);
					zend_objects_store_mark_destructed(&EG(objects_store));
					_zend_bailout((char*) __FILE__, __LINE__);
					return;
				}
			}
			break;
	}

#if PHP_VERSION_ID < 70400
	/* Log if necessary */
	if (!display) {
		efree(buffer);
		return;
	}

#if PHP_VERSION_ID >= 70200
	if (PG(track_errors) && EG(active)) {
#else
	if (PG(track_errors) && EG(valid_symbol_table)) {
#endif
		zval tmp;
		ZVAL_STRINGL(&tmp, buffer, buffer_len);

		if (EG(current_execute_data)) {
			if (zend_set_local_var_str("php_errormsg", sizeof("php_errormsg")-1, &tmp, 0) == FAILURE) {
				zval_ptr_dtor(&tmp);
			}
		} else {
			zend_hash_str_update(&EG(symbol_table), "php_errormsg", sizeof("php_errormsg"), &tmp);
		}
	}
#endif

	efree(buffer);
}

/* {{{ proto void xdebug_print_function_stack([string message [, int options])
   Displays a stack trace */
PHP_FUNCTION(xdebug_print_function_stack)
{
	char *message = NULL;
	size_t message_len;
	function_stack_entry *i;
	char *tmp;
	zend_long options = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|sl", &message, &message_len, &options) == FAILURE) {
		return;
	}

	i = xdebug_get_stack_frame(0);
	if (message) {
		tmp = get_printable_stack(PG(html_errors), 0, message, i->filename, i->lineno, !(options & XDEBUG_STACK_NO_DESC));
	} else {
		tmp = get_printable_stack(PG(html_errors), 0, "user triggered", i->filename, i->lineno, !(options & XDEBUG_STACK_NO_DESC));
	}
	php_printf("%s", tmp);
	xdfree(tmp);
}
/* }}} */

/* {{{ proto array xdebug_get_formatted_function_stack()
   Displays a stack trace */
PHP_FUNCTION(xdebug_get_formatted_function_stack)
{
	function_stack_entry *i;
	char *tmp;

	i = xdebug_get_stack_frame(0);
	tmp = get_printable_stack(PG(html_errors), 0, "user triggered", i->filename, i->lineno, 1);
	RETVAL_STRING(tmp);
	xdfree(tmp);
}
/* }}} */

/* {{{ proto string xdebug_call_class()
   Returns the name of the calling class */
PHP_FUNCTION(xdebug_call_class)
{
	function_stack_entry *i;
	zend_long depth = 2;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &depth) == FAILURE) {
		return;
	}
	i = xdebug_get_stack_frame(depth);
	if (i) {
		if (i->function.class) {
			RETURN_STRING(i->function.class);
		} else {
			RETURN_FALSE;
		}
	} else {
		return;
	}
}
/* }}} */

/* {{{ proto string xdebug_call_function()
   Returns the function name from which the current function was called from. */
PHP_FUNCTION(xdebug_call_function)
{
	function_stack_entry *i;
	zend_long depth = 2;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &depth) == FAILURE) {
		return;
	}
	i = xdebug_get_stack_frame(depth);
	if (i) {
		if (i->function.function) {
			RETURN_STRING(i->function.function);
		} else {
			RETURN_FALSE;
		}
	} else {
		return;
	}
}
/* }}} */

/* {{{ proto int xdebug_call_line()
   Returns the line number where the current function was called from. */
PHP_FUNCTION(xdebug_call_line)
{
	function_stack_entry *i;
	zend_long depth = 2;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &depth) == FAILURE) {
		return;
	}
	i = xdebug_get_stack_frame(depth);
	if (i) {
		RETURN_LONG(i->lineno);
	} else {
		return;
	}
}
/* }}} */

/* {{{ proto string xdebug_call_file()
   Returns the filename where the current function was called from. */
PHP_FUNCTION(xdebug_call_file)
{
	function_stack_entry *i;
	zend_long depth = 2;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &depth) == FAILURE) {
		return;
	}
	i = xdebug_get_stack_frame(depth);
	if (i) {
		RETURN_STRING(i->filename);
	} else {
		return;
	}
}
/* }}} */

static int find_line_number_for_current_execute_point(zend_execute_data *edata)
{
	zend_execute_data *ptr = edata;

	while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
		ptr = ptr->prev_execute_data;
	}

	if (ptr && ptr->opline) {
		return ptr->opline->lineno;
	}

	return 0;
}

int xdebug_function_name_is_closure(char *fname)
{
	int length = strlen(fname);
	int closure_length = strlen("{closure}");

	if (length < closure_length) {
		return 0;
	}

	if (strcmp(fname + length - closure_length, "{closure}") == 0) {
		return 1;
	}

	return 0;
}

char* xdebug_wrap_closure_location_around_function_name(zend_op_array *opa, char *fname)
{
	xdebug_str tmp = XDEBUG_STR_INITIALIZER;
	char *tmp_loc_info;

	xdebug_str_addl(&tmp, fname, strlen(fname) - 1, 0);

	tmp_loc_info = xdebug_sprintf(
		":%s:%d-%d}",
		opa->filename->val,
		opa->line_start,
		opa->line_end
	);
	xdebug_str_add(&tmp, tmp_loc_info, 1);

	return tmp.d;
}

/* I don't like this API, but the function_stack_entry does not keep this as a
 * pointer, and hence we need two APIs for freeing :-S */
void xdebug_func_dtor_by_ref(xdebug_func *elem)
{
	if (elem->function) {
		xdfree(elem->function);
	}
	if (elem->class) {
		xdfree(elem->class);
	}
}

void xdebug_func_dtor(xdebug_func *elem)
{
	xdebug_func_dtor_by_ref(elem);
	xdfree(elem);
}

void xdebug_build_fname_from_oparray(xdebug_func *tmp, zend_op_array *opa)
{
	int closure = 0;

	memset(tmp, 0, sizeof(xdebug_func));

	if (opa->function_name) {
		if (xdebug_function_name_is_closure(STR_NAME_VAL(opa->function_name))) {
			tmp->function = xdebug_wrap_closure_location_around_function_name(opa, STR_NAME_VAL(opa->function_name));
			closure = 1;
		} else {
			tmp->function = xdstrdup(STR_NAME_VAL(opa->function_name));
		}
	} else {
		tmp->function = xdstrdup("{main}");
		tmp->type = XFUNC_MAIN;
	}

	if (opa->scope && !closure) {
		tmp->type = XFUNC_MEMBER;
		tmp->class = xdstrdup(STR_NAME_VAL(opa->scope->name));
	} else {
		tmp->type = XFUNC_NORMAL;
	}
}

void xdebug_build_fname(xdebug_func *tmp, zend_execute_data *edata)
{
	memset(tmp, 0, sizeof(xdebug_func));

	if (edata && edata->func && edata->func == (zend_function*) &zend_pass_function) {
		tmp->type     = XFUNC_ZEND_PASS;
		tmp->function = xdstrdup("{zend_pass}");
	} else if (edata && edata->func) {
		tmp->type = XFUNC_NORMAL;
		if ((Z_TYPE(edata->This)) == IS_OBJECT) {
			tmp->type = XFUNC_MEMBER;
			if (edata->func->common.scope && strcmp(edata->func->common.scope->name->val, "class@anonymous") == 0) {
				tmp->class = xdebug_sprintf(
					"{anonymous-class:%s:%d-%d}",
					edata->func->common.scope->info.user.filename->val,
					edata->func->common.scope->info.user.line_start,
					edata->func->common.scope->info.user.line_end
				);
			} else {
				tmp->class = xdstrdup(edata->This.value.obj->ce->name->val);
			}
		} else {
			if (edata->func->common.scope) {
				tmp->type = XFUNC_STATIC_MEMBER;
				tmp->class = xdstrdup(edata->func->common.scope->name->val);
			}
		}
		if (edata->func->common.function_name) {
			if (xdebug_function_name_is_closure(edata->func->common.function_name->val)) {
				tmp->function = xdebug_wrap_closure_location_around_function_name(&edata->func->op_array, edata->func->common.function_name->val);
			} else if (strncmp(edata->func->common.function_name->val, "call_user_func", 14) == 0) {
				const char *fname = NULL;
				int         lineno = 0;

				if (edata->prev_execute_data && edata->prev_execute_data->func && edata->prev_execute_data->func->type == ZEND_USER_FUNCTION) {
					fname = edata->prev_execute_data->func->op_array.filename->val;
				}

				if (
					!fname &&
					XDEBUG_LLIST_TAIL(XG_BASE(stack)) &&
					XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack))) &&
					((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack))))->filename
				) {
					fname = ((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack))))->filename;
				}

				if (!fname) {
					/* It wasn't a special call_user_func after all */
					goto normal_after_all;
				}

				lineno = find_line_number_for_current_execute_point(edata);

				tmp->function = xdebug_sprintf(
					"%s:{%s:%d}",
					edata->func->common.function_name->val,
					fname,
					lineno
				);
			} else {
normal_after_all:
				tmp->function = xdstrdup(edata->func->common.function_name->val);
			}
		} else if (
			edata &&
			edata->func &&
			edata->func->type == ZEND_EVAL_CODE &&
			edata->prev_execute_data &&
			edata->prev_execute_data->func &&
			edata->prev_execute_data->func->common.function_name &&
			(
				(strncmp(edata->prev_execute_data->func->common.function_name->val, "assert", 6) == 0) ||
				(strncmp(edata->prev_execute_data->func->common.function_name->val, "create_function", 15) == 0)
			)
		) {
			tmp->type = XFUNC_NORMAL;
			tmp->function = xdstrdup("{internal eval}");
		} else if (
			edata &&
			edata->prev_execute_data &&
			edata->prev_execute_data->func->type == ZEND_USER_FUNCTION &&
			edata->prev_execute_data->opline &&
			edata->prev_execute_data->opline->opcode == ZEND_INCLUDE_OR_EVAL
		) {
			switch (edata->prev_execute_data->opline->extended_value) {
				case ZEND_EVAL:
					tmp->type = XFUNC_EVAL;
					break;
				case ZEND_INCLUDE:
					tmp->type = XFUNC_INCLUDE;
					break;
				case ZEND_REQUIRE:
					tmp->type = XFUNC_REQUIRE;
					break;
				case ZEND_INCLUDE_ONCE:
					tmp->type = XFUNC_INCLUDE_ONCE;
					break;
				case ZEND_REQUIRE_ONCE:
					tmp->type = XFUNC_REQUIRE_ONCE;
					break;
				default:
					tmp->type = XFUNC_UNKNOWN;
					break;
			}
		} else if (
			edata &&
			edata->prev_execute_data
		) {
			xdebug_build_fname(tmp, edata->prev_execute_data);
		} else {
			tmp->type = XFUNC_UNKNOWN;
		}
	}
}

function_stack_entry *xdebug_add_stack_frame(zend_execute_data *zdata, zend_op_array *op_array, int type)
{
	zend_execute_data    *edata;
	zend_op             **opline_ptr = NULL;
	function_stack_entry *tmp;
	zend_op              *cur_opcode;
	int                   i = 0;
	int                   hit_variadic = 0;

	if (type == XDEBUG_USER_DEFINED) {
		edata = EG(current_execute_data)->prev_execute_data;
		if (edata) {
			opline_ptr = (zend_op**) &edata->opline;
		}
	} else {
		edata = EG(current_execute_data);
		opline_ptr = (zend_op**) &EG(current_execute_data)->opline;
	}
	zdata = EG(current_execute_data);

	tmp = xdmalloc (sizeof (function_stack_entry));
	tmp->var           = NULL;
	tmp->varc          = 0;
	tmp->refcount      = 1;
	tmp->level         = XG_BASE(level);
	tmp->arg_done      = 0;
	tmp->declared_vars = NULL;
	tmp->user_defined  = type;
	tmp->filename      = NULL;
	tmp->include_filename  = NULL;
	tmp->profile.call_list = NULL;
	tmp->op_array      = op_array;
	tmp->symbol_table  = NULL;
	tmp->execute_data  = NULL;
	tmp->is_variadic   = 0;
	tmp->filtered_tracing       = 0;
	tmp->filtered_code_coverage = 0;

	XG_BASE(function_count)++;
	tmp->function_nr = XG_BASE(function_count);
	{
		zend_execute_data *ptr = edata;
		while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
			ptr = ptr->prev_execute_data;
		}
		if (ptr) {
			tmp->filename = xdstrdup(ptr->func->op_array.filename->val);
		}
	}

	if (!tmp->filename) {
		/* Includes/main script etc */
		tmp->filename  = (type == XDEBUG_USER_DEFINED && op_array && op_array->filename) ? xdstrdup(op_array->filename->val): NULL;
	}
	/* Call user function locations */
	if (
		!tmp->filename &&
		XG_BASE(stack) &&
		XDEBUG_LLIST_TAIL(XG_BASE(stack)) &&
		XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack))) &&
		((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack))))->filename
	) {
		tmp->filename = xdstrdup(((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack))))->filename);
	}

	if (!tmp->filename) {
		tmp->filename = xdstrdup("UNKNOWN?");
	}
	tmp->prev_memory = XG_BASE(prev_memory);
	tmp->memory = zend_memory_usage(0);
	XG_BASE(prev_memory) = tmp->memory;
	tmp->time   = xdebug_get_utime();
	tmp->lineno = 0;
	tmp->prev   = 0;

	xdebug_build_fname(&(tmp->function), zdata);
	if (!tmp->function.type) {
		tmp->function.function = xdstrdup("{main}");
		tmp->function.class    = NULL;
		tmp->function.type     = XFUNC_MAIN;

	} else if (tmp->function.type & XFUNC_INCLUDES) {
		tmp->lineno = 0;
		if (opline_ptr) {
			cur_opcode = *opline_ptr;
			if (cur_opcode) {
				tmp->lineno = cur_opcode->lineno;
			}
		}

		if (tmp->function.type == XFUNC_EVAL) {
			tmp->include_filename = xdebug_sprintf("%s", XG_BASE(last_eval_statement));
		} else if (XINI_BASE(collect_includes)) {
			tmp->include_filename = xdstrdup(zend_get_executed_filename());
		}
	} else  {
		tmp->lineno = find_line_number_for_current_execute_point(edata);
		tmp->is_variadic = !!(zdata->func->common.fn_flags & ZEND_ACC_VARIADIC);

		if (XINI_BASE(collect_params) || XINI_BASE(collect_vars) || xdebug_is_debug_connection_active_for_current_pid()) {
			int    arguments_sent = 0, arguments_wanted = 0, arguments_storage = 0;

			/* This calculates how many arguments where sent to a function. It
			 * works for both internal and user defined functions.
			 * op_array->num_args works only for user defined functions so
			 * we're not using that here. */
			arguments_sent = ZEND_CALL_NUM_ARGS(zdata);
			arguments_wanted = arguments_sent;

			if (ZEND_USER_CODE(zdata->func->type)) {
				arguments_wanted = op_array->num_args;
			}

			if (ZEND_USER_CODE(zdata->func->type) && zdata->func->common.fn_flags & ZEND_ACC_VARIADIC) {
				arguments_wanted++;
				arguments_sent++;
			}

			if (arguments_wanted > arguments_sent) {
				arguments_storage = arguments_wanted;
			} else {
				arguments_storage = arguments_sent;
			}
			tmp->var = xdmalloc(arguments_storage * sizeof (xdebug_var_name));

			for (i = 0; i < arguments_sent; i++) {
				tmp->var[tmp->varc].name = NULL;
				ZVAL_UNDEF(&tmp->var[tmp->varc].data);
				tmp->var[tmp->varc].length = 0;
				tmp->var[tmp->varc].is_variadic = 0;

				/* Because it is possible that more parameters are sent, then
				 * actually wanted  we can only access the name in case there
				 * is an associated variable to receive the variable here. */
				if (tmp->user_defined == XDEBUG_USER_DEFINED && i < arguments_wanted) {
					if (op_array->arg_info[i].name) {
						tmp->var[tmp->varc].name = xdstrdup(STR_NAME_VAL(op_array->arg_info[i].name));
						tmp->var[tmp->varc].length = STR_NAME_LEN(op_array->arg_info[i].name);
					}
					if (op_array->arg_info[i].is_variadic) {
						tmp->var[tmp->varc].is_variadic = 1;
					}
					if (op_array->arg_info[i].is_variadic && !hit_variadic) {
						tmp->var[tmp->varc].is_variadic = 1;
						hit_variadic = 1;
					}
				}

				if (XINI_BASE(collect_params)) {
					if ((i < arguments_wanted) || ((zdata->func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE) && (i < arguments_sent))) {
						if (ZEND_CALL_ARG(zdata, tmp->varc+1)) {
							ZVAL_COPY(&(tmp->var[tmp->varc].data), ZEND_CALL_ARG(zdata, tmp->varc+1));
						}
					} else {
						ZVAL_COPY(&(tmp->var[tmp->varc].data), ZEND_CALL_VAR_NUM(zdata, zdata->func->op_array.last_var + zdata->func->op_array.T + i - arguments_wanted));
					}
				}
				tmp->varc++;
			}

			/* Sometimes not enough arguments are send to a user defined
			 * function, so we have to gather only the name for those extra. */
			if (tmp->user_defined == XDEBUG_USER_DEFINED && arguments_sent < arguments_wanted) {
				for (i = arguments_sent; i < arguments_wanted; i++) {
					if (op_array->arg_info[i].name) {
						tmp->var[tmp->varc].name = xdstrdup(STR_NAME_VAL(op_array->arg_info[i].name));
						tmp->var[tmp->varc].length = STR_NAME_LEN(op_array->arg_info[i].name);
					}
					ZVAL_UNDEF(&tmp->var[tmp->varc].data);
					tmp->var[tmp->varc].is_variadic = 0;
					tmp->varc++;
				}
			}
		}
	}

	/* Now we have location and name, we can run the filter */
	xdebug_filter_run_tracing(tmp);

	/* Count code coverage line for call */
	xdebug_coverage_count_line_if_branch_check_active(op_array, tmp->filename, tmp->lineno);

	if (XG_BASE(do_monitor_functions)) {
		char *func_name = xdebug_show_fname(tmp->function, 0, 0);
		int   func_name_len = strlen(func_name);
		void *dummy;

		if (xdebug_hash_find(XG_BASE(functions_to_monitor), func_name, func_name_len, (void *) &dummy)) {
			xdebug_function_monitor_record(func_name, tmp->filename, tmp->lineno);
		}

		xdfree(func_name);
	}

	if (XG_BASE(stack)) {
		if (XDEBUG_LLIST_TAIL(XG_BASE(stack))) {
			function_stack_entry *prev = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack)));
			tmp->prev = prev;
		}
		xdebug_llist_insert_next(XG_BASE(stack), XDEBUG_LLIST_TAIL(XG_BASE(stack)), tmp);
	}

	return tmp;
}

/* {{{ proto int xdebug_get_stack_depth()
   Returns the stack depth */
PHP_FUNCTION(xdebug_get_stack_depth)
{
	/* We substract one so that the function call to xdebug_get_stack_depth()
	 * is not part of the returned depth. */
	RETURN_LONG(XG_BASE(stack)->size - 1);
}

/* {{{ proto array xdebug_get_function_stack()
   Returns an array representing the current stack */
PHP_FUNCTION(xdebug_get_function_stack)
{
	xdebug_llist_element *le;
	unsigned int          j;
	unsigned int          k;
	zval                 *frame;
	zval                 *params;

	array_init(return_value);
	le = XDEBUG_LLIST_HEAD(XG_BASE(stack));

	for (k = 0; k < XG_BASE(stack)->size - 1; k++, le = XDEBUG_LLIST_NEXT(le)) {
		function_stack_entry *i = XDEBUG_LLIST_VALP(le);

		if (i->function.function) {
			if (strcmp(i->function.function, "xdebug_get_function_stack") == 0) {
				return;
			}
		}

		/* Initialize frame array */
		XDEBUG_MAKE_STD_ZVAL(frame);
		array_init(frame);

		/* Add data */
		if (i->function.function) {
			add_assoc_string_ex(frame, "function", HASH_KEY_SIZEOF("function"), i->function.function);
		}
		if (i->function.class) {
			add_assoc_string_ex(frame, "type",     HASH_KEY_SIZEOF("type"),     (char*) (i->function.type == XFUNC_STATIC_MEMBER ? "static" : "dynamic"));
			add_assoc_string_ex(frame, "class",    HASH_KEY_SIZEOF("class"),    i->function.class   );
		}
		add_assoc_string_ex(frame, "file", HASH_KEY_SIZEOF("file"), i->filename);
		add_assoc_long_ex(frame, "line", HASH_KEY_SIZEOF("line"), i->lineno);

		/* Add parameters */
		XDEBUG_MAKE_STD_ZVAL(params);
		array_init(params);
		add_assoc_zval_ex(frame, "params", HASH_KEY_SIZEOF("params"), params);

		for (j = 0; j < i->varc; j++) {
			int         variadic_opened = 0;
			xdebug_str *argument = NULL;

			if (i->var[j].is_variadic) {
				zval *vparams;

				XDEBUG_MAKE_STD_ZVAL(vparams);
				array_init(vparams);

				if (i->var[j].name) {
					add_assoc_zval(params, i->var[j].name, vparams);
				} else {
					add_index_zval(params, j, vparams);
				}
				efree(params);
				params = vparams;
				variadic_opened = 1;
				continue;
			}
			if (!Z_ISUNDEF(i->var[j].data)) {
				argument = xdebug_get_zval_value_line(&i->var[j].data, 0, NULL);
			} else {
				argument = xdebug_str_create_from_char((char*) "???");
			}
			if (i->var[j].name && !variadic_opened && argument) {
				add_assoc_stringl_ex(params, i->var[j].name, i->var[j].length, argument->d, argument->l);
			} else {
				add_index_stringl(params, j - 1, argument->d, argument->l);
			}
			if (argument) {
				xdebug_str_free(argument);
				argument = NULL;
			}
		}

		if (i->include_filename) {
			add_assoc_string_ex(frame, "include_filename", HASH_KEY_SIZEOF("include_filename"), i->include_filename);
		}

		add_next_index_zval(return_value, frame);
		efree(params);
		efree(frame);
	}
}
/* }}} */

static void xdebug_attach_used_var_names(void *return_value, xdebug_hash_element *he)
{
	xdebug_str *name = (xdebug_str*) he->ptr;

	add_next_index_string(return_value, name->d);
}

/* {{{ proto array xdebug_get_declared_vars()
   Returns an array representing the current stack */
PHP_FUNCTION(xdebug_get_declared_vars)
{
	xdebug_llist_element *le;
	function_stack_entry *i;
	xdebug_hash *tmp_hash;

	array_init(return_value);
	le = XDEBUG_LLIST_TAIL(XG_BASE(stack));
	le = XDEBUG_LLIST_PREV(le);
	i = XDEBUG_LLIST_VALP(le);

	/* Add declared vars */
	if (i->declared_vars) {
		tmp_hash = xdebug_declared_var_hash_from_llist(i->declared_vars);
		xdebug_hash_apply(tmp_hash, (void *) return_value, xdebug_attach_used_var_names);
		xdebug_hash_destroy(tmp_hash);
	}
}
/* }}} */
