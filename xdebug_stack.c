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
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   +----------------------------------------------------------------------+
 */
#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_code_coverage.h"
#include "xdebug_com.h"
#include "xdebug_compat.h"
#include "xdebug_monitor.h"
#include "xdebug_profiler.h"
#include "xdebug_stack.h"
#include "xdebug_str.h"
#include "xdebug_superglobals.h"
#include "xdebug_var.h"
#include "ext/standard/html.h"
#if PHP_VERSION_ID >= 70000
# include "ext/standard/php_smart_string.h"
#else
# include "ext/standard/php_smart_str.h"
#endif

#include "main/php_ini.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static char* text_formats[11] = {
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

static char* ansi_formats[11] = {
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

static char* html_formats[13] = {
	"<br />\n<font size='1'><table class='xdebug-error xe-%s%s' dir='ltr' border='1' cellspacing='0' cellpadding='1'>\n",
	"<tr><th align='left' bgcolor='#f57900' colspan=\"5\"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> %s: %s in %s on line <i>%d</i></th></tr>\n",
	"<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>\n<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>\n",
	"<tr><td bgcolor='#eeeeec' align='center'>%d</td><td bgcolor='#eeeeec' align='center'>%.4F</td><td bgcolor='#eeeeec' align='right'>%ld</td><td bgcolor='#eeeeec'>%s( ",
	"<font color='#00bb00'>'%s'</font>",
	" )</td><td title='%s' bgcolor='#eeeeec'>...%s<b>:</b>%d</td></tr>\n",
	"<tr><th align='left' colspan='5' bgcolor='#e9b96e'>Variables in local scope (#%d)</th></tr>\n",
	"</table></font>\n",
	"<tr><td colspan='2' align='right' bgcolor='#eeeeec' valign='top'><pre>$%s&nbsp;=</pre></td><td colspan='3' bgcolor='#eeeeec'>%s</td></tr>\n",
	"<tr><td colspan='2' align='right' bgcolor='#eeeeec' valign='top'><pre>$%s&nbsp;=</pre></td><td colspan='3' bgcolor='#eeeeec' valign='top'><i>Undefined</i></td></tr>\n",
	" )</td><td title='%s' bgcolor='#eeeeec'><a style='color: black' href='%s'>...%s<b>:</b>%d</a></td></tr>\n",
	"<tr><th align='left' bgcolor='#f57900' colspan=\"5\"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> %s: %s in <a style='color: black' href='%s'>%s</a> on line <i>%d</i></th></tr>\n",
	"<tr><th align='left' bgcolor='#f57900' colspan=\"5\"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> SCREAM: Error suppression ignored for</th></tr>\n"
};

static char** select_formats(int html TSRMLS_DC) {
	if (html) {
		return html_formats;
	}
	else if ((XG(cli_color) == 1 && xdebug_is_output_tty(TSRMLS_C)) || (XG(cli_color) == 2)) {
		return ansi_formats;
	}
	else {
		return text_formats;
	}
}

static void dump_used_var_with_contents(void *htmlq, xdebug_hash_element* he, void *argument)
{
	int        html = *(int *)htmlq;
	int        len;
	zval      *zvar;
	char      *contents;
	char      *name = (char*) he->ptr;
	HashTable *tmp_ht;
	char     **formats;
	xdebug_str *str = (xdebug_str *) argument;
	TSRMLS_FETCH();

	if (!he->ptr) {
		return;
	}

	/* Bail out on $this and $GLOBALS */
	if (strcmp(name, "this") == 0 || strcmp(name, "GLOBALS") == 0) {
		return;
	}

#if PHP_VERSION_ID >= 70100
	if (!(ZEND_CALL_INFO(EG(current_execute_data)) & ZEND_CALL_HAS_SYMBOL_TABLE)) {
#elif PHP_VERSION_ID >= 70000
	if (!EG(current_execute_data)->symbol_table) {
#else
	if (!EG(active_symbol_table)) {
#endif
		zend_rebuild_symbol_table(TSRMLS_C);
	}

	tmp_ht = XG(active_symbol_table);
#if PHP_VERSION_ID >= 70000
	{
		zend_execute_data *ex = EG(current_execute_data);
		while (ex && (!ex->func || !ZEND_USER_CODE(ex->func->type))) {
			ex = ex->prev_execute_data;
		}
		if (ex) {
			XG(active_execute_data) = ex;
			XG(active_symbol_table) = ex->symbol_table;
		}
	}
#else
	XG(active_symbol_table) = EG(active_symbol_table);
#endif
	zvar = xdebug_get_php_symbol(name TSRMLS_CC);
	XG(active_symbol_table) = tmp_ht;

	formats = select_formats(PG(html_errors) TSRMLS_CC);

	if (!zvar) {
		xdebug_str_add(str, xdebug_sprintf(formats[9], name), 1);
		return;
	}

	if (html) {
		contents = xdebug_get_zval_value_fancy(NULL, zvar, &len, 0, NULL TSRMLS_CC);
	} else {
		contents = xdebug_get_zval_value(zvar, 0, NULL);
	}

	if (contents) {
		xdebug_str_add(str, xdebug_sprintf(formats[8], name, contents), 1);
	} else {
		xdebug_str_add(str, xdebug_sprintf(formats[9], name), 1);
	}

	xdfree(contents);
}

void xdebug_log_stack(const char *error_type_str, char *buffer, const char *error_filename, const int error_lineno TSRMLS_DC)
{
	xdebug_llist_element *le;
	function_stack_entry *i;
	char                 *tmp_log_message;

	tmp_log_message = xdebug_sprintf( "PHP %s:  %s in %s on line %d", error_type_str, buffer, error_filename, error_lineno);
	php_log_err(tmp_log_message TSRMLS_CC);
	xdfree(tmp_log_message);

	if (XG(stack) && XG(stack)->size) {
		php_log_err("PHP Stack trace:" TSRMLS_CC);

		for (le = XDEBUG_LLIST_HEAD(XG(stack)); le != NULL; le = XDEBUG_LLIST_NEXT(le))
		{
			int c = 0; /* Comma flag */
			unsigned int j = 0; /* Counter */
			char *tmp_name;
			xdebug_str log_buffer = XDEBUG_STR_INITIALIZER;
			int variadic_opened = 0;

			i = XDEBUG_LLIST_VALP(le);
			tmp_name = xdebug_show_fname(i->function, 0, 0 TSRMLS_CC);
			xdebug_str_add(&log_buffer, xdebug_sprintf("PHP %3d. %s(", i->level, tmp_name), 1);
			xdfree(tmp_name);

			/* Printing vars */
			for (j = 0; j < i->varc; j++) {
				char *tmp_varname, *tmp_value;

				if (c) {
					xdebug_str_addl(&log_buffer, ", ", 2, 0);
				} else {
					c = 1;
				}

				if (
					(i->var[j].is_variadic && XG(collect_params) != 5)
#if PHP_VERSION_ID >= 50600 && PHP_VERSION_ID < 70000
					|| (!i->var[j].addr && i->is_variadic && j == i->varc - 1)
#endif
				) {
					xdebug_str_add(&log_buffer, "...", 0);
					variadic_opened = 1;
				}

				tmp_varname = i->var[j].name ? xdebug_sprintf("$%s = ", i->var[j].name) : xdstrdup("");
				xdebug_str_add(&log_buffer, tmp_varname, 0);
				xdfree(tmp_varname);

				if (i->var[j].is_variadic) {
					xdebug_str_add(&log_buffer, "variadic(", 0);
#if PHP_VERSION_ID >= 70000
					c = 0;
					continue;
#endif
				}

				if (i->var[j].addr) {
					tmp_value = xdebug_get_zval_value(i->var[j].addr, 0, NULL);
					xdebug_str_add(&log_buffer, tmp_value, 0);
					xdfree(tmp_value);
#if PHP_VERSION_ID >= 50600 && PHP_VERSION_ID < 70000
				} else if (i->is_variadic && j == i->varc - 1) {
					xdebug_str_addl(&log_buffer, "variadic(", 9, 0);
#endif
				} else {
					xdebug_str_addl(&log_buffer, "*uninitialized*", 15, 0);
				}
			}

			if (variadic_opened) {
				xdebug_str_add(&log_buffer, ")", 0);
			}

			xdebug_str_add(&log_buffer, xdebug_sprintf(") %s:%d", i->filename, i->lineno), 1);
			php_log_err(log_buffer.d TSRMLS_CC);
			xdebug_str_free(&log_buffer);
		}
	}
}

void xdebug_append_error_head(xdebug_str *str, int html, char *error_type_str TSRMLS_DC)
{
	char **formats = select_formats(html TSRMLS_CC);

	if (html) {
		xdebug_str_add(str, xdebug_sprintf(formats[0], error_type_str, XG(in_at) ? " xe-scream" : ""), 1);
		if (XG(in_at)) {
			xdebug_str_add(str, formats[12], 0);
		}
	} else {
		xdebug_str_add(str, formats[0], 0);
		if (XG(in_at)) {
			xdebug_str_add(str, formats[10], 0);
		}
	}
}

void xdebug_append_error_description(xdebug_str *str, int html, const char *error_type_str, char *buffer, const char *error_filename, const int error_lineno TSRMLS_DC)
{
	char **formats = select_formats(html TSRMLS_CC);
	char *escaped;

	if (html) {
#if PHP_VERSION_ID >= 70000
		zend_string *tmp;
#else
		char  *tmp;
		size_t newlen;
#endif
		char *first_closing = strchr(buffer, ']');

		/* We do need to escape HTML entities here, as HTML chars could be in
		 * the error message. However, PHP in some circumstances also adds an
		 * HTML link to a manual page. That bit, we don't need to escape. So
		 * this bit of code finds the portion that doesn't need escaping, adds
		 * it to a tmp string, and then adds an HTML escaped string for the
		 * rest of the original buffer. */
		if (first_closing && strstr(buffer, "() [<a href=") != NULL) {
#if PHP_VERSION_ID >= 70000
			smart_string special_escaped = { 0, 0, 0 };
#else
			smart_str special_escaped = { 0, 0, 0 };
#endif

			*first_closing = '\0';
			first_closing++;

#if PHP_VERSION_ID >= 70000
			smart_string_appends(&special_escaped, buffer);
			tmp = php_escape_html_entities((unsigned char *) first_closing, strlen(first_closing), 0, 0, NULL TSRMLS_CC);
			smart_string_appends(&special_escaped, tmp->val);
			zend_string_free(tmp);
#else
			smart_str_appends(&special_escaped, buffer);
			tmp = php_escape_html_entities((unsigned char *) first_closing, strlen(first_closing), &newlen, 0, 0, NULL TSRMLS_CC);
			smart_str_appends(&special_escaped, tmp);
			STR_FREE(tmp);
#endif

#if PHP_VERSION_ID >= 70000
			smart_string_0(&special_escaped);
			escaped = estrdup(special_escaped.c);
			smart_string_free(&special_escaped);
#else
			smart_str_0(&special_escaped);
			escaped = estrdup(special_escaped.c);
			smart_str_free(&special_escaped);
#endif
		} else if (strncmp(buffer, "assert()", 8) == 0) {
			/* Also don't escape if we're in an assert, as things are already
			 * escaped. It's all nice and consistent ey? */
			escaped = estrdup(buffer);
		} else {
#if PHP_VERSION_ID >= 70000
			tmp = php_escape_html_entities((unsigned char *) buffer, strlen(buffer), 0, 0, NULL TSRMLS_CC);
			escaped = estrdup(tmp->val);
			zend_string_free(tmp);
#else
			escaped = php_escape_html_entities((unsigned char *) buffer, strlen(buffer), &newlen, 0, 0, NULL TSRMLS_CC);
#endif
		}
	} else {
		escaped = estrdup(buffer);
	}

	if (strlen(XG(file_link_format)) > 0 && html) {
		char *file_link;

		xdebug_format_file_link(&file_link, error_filename, error_lineno TSRMLS_CC);
		xdebug_str_add(str, xdebug_sprintf(formats[11], error_type_str, escaped, file_link, error_filename, error_lineno), 1);
		xdfree(file_link);
	} else {
		xdebug_str_add(str, xdebug_sprintf(formats[1], error_type_str, escaped, error_filename, error_lineno), 1);
	}

#if PHP_VERSION_ID >= 70000
	efree(escaped);
#else
	STR_FREE(escaped);
#endif
}

static void add_single_value(xdebug_str *str, zval *zv, int html, int collecton_level TSRMLS_DC)
{
	char *tmp_value, *tmp_fancy_value, *tmp_fancy_synop_value, *tmp_serialized;
	int    len;
	size_t newlen;

	if (html) {
		switch (collecton_level) {
			case 1: /* synopsis */
				tmp_fancy_synop_value = xdebug_get_zval_synopsis_fancy("", zv, &len, 0, NULL TSRMLS_CC);
				xdebug_str_add(str, xdebug_sprintf("<span>%s</span>", tmp_fancy_synop_value), 1);
				xdfree(tmp_fancy_synop_value);
				break;
			case 2: /* synopsis + full in tooltip */
				tmp_value = xdebug_get_zval_value(zv, 0, NULL);
				tmp_fancy_value = xdebug_xmlize(tmp_value, strlen(tmp_value), &newlen);
				tmp_fancy_synop_value = xdebug_get_zval_synopsis_fancy("", zv, &len, 0, NULL TSRMLS_CC);
				xdebug_str_add(str, xdebug_sprintf("<span title='%s'>%s</span>", tmp_fancy_value, tmp_fancy_synop_value), 1);
				xdfree(tmp_value);
				efree(tmp_fancy_value);
				xdfree(tmp_fancy_synop_value);
				break;
			case 3: /* full */
			case 4: /* full (with var_name) */
			default:
				tmp_value = xdebug_get_zval_value(zv, 0, NULL);
				tmp_fancy_value = xdebug_xmlize(tmp_value, strlen(tmp_value), &newlen);
				xdebug_str_add(str, xdebug_sprintf("<span>%s</span>", tmp_fancy_value), 1);
				xdfree(tmp_value);
				efree(tmp_fancy_value);
				break;
			case 5: { /* serialized */
				tmp_serialized = xdebug_get_zval_value_serialized(zv, 0, NULL TSRMLS_CC);
				xdebug_str_add(str, xdebug_sprintf("<span>%s</span>", tmp_serialized), 1);
				xdfree(tmp_serialized);
			} break;
		}
	} else {
		switch (collecton_level) {
			case 1: /* synopsis */
			case 2:
				tmp_value = xdebug_get_zval_synopsis(zv, 0, NULL);
				break;
			case 3: /* full */
			case 4: /* full (with var_name) */
			default:
				tmp_value = xdebug_get_zval_value(zv, 0, NULL);
				break;
			case 5: /* serialized */
				tmp_value = xdebug_get_zval_value_serialized(zv, 0, NULL TSRMLS_CC);
				break;
		}
		if (tmp_value) {
			xdebug_str_add(str, xdebug_sprintf("%s", tmp_value), 1);
			xdfree(tmp_value);
		} else {
			xdebug_str_addl(str, "???", 3, 0);
		}
	}
}

void xdebug_append_printable_stack(xdebug_str *str, int html TSRMLS_DC)
{
	xdebug_llist_element *le;
	function_stack_entry *i;
	int    printed_frames = 0;
	char **formats = select_formats(html TSRMLS_CC);

	if (XG(stack) && XG(stack)->size) {
		i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_HEAD(XG(stack)));

		xdebug_str_add(str, formats[2], 0);

		for (le = XDEBUG_LLIST_HEAD(XG(stack)); le != NULL; le = XDEBUG_LLIST_NEXT(le))
		{
			int c = 0; /* Comma flag */
			unsigned int j = 0; /* Counter */
			char *tmp_name;
			int variadic_opened = 0;

			i = XDEBUG_LLIST_VALP(le);
			tmp_name = xdebug_show_fname(i->function, html, 0 TSRMLS_CC);
			if (html) {
				xdebug_str_add(str, xdebug_sprintf(formats[3], i->level, i->time - XG(start_time), i->memory, tmp_name), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf(formats[3], i->time - XG(start_time), i->memory, i->level, tmp_name), 1);
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
					(i->var[j].is_variadic && i->var[j].addr)
#if PHP_VERSION_ID >= 50600 && PHP_VERSION_ID < 70000
					|| (!i->var[j].addr && i->is_variadic && j == i->varc - 1)
#endif
				) {
					xdebug_str_add(str, "...", 0);
#if PHP_VERSION_ID < 70000
					variadic_opened = 1;
#endif
				}

				if (i->var[j].name && XG(collect_params) == 4) {
					if (html) {
						xdebug_str_add(str, xdebug_sprintf("<span>$%s = </span>", i->var[j].name), 1);
					} else {
						xdebug_str_add(str, xdebug_sprintf("$%s = ", i->var[j].name), 1);
					}
				}

#if PHP_VERSION_ID >= 70000
				if (!variadic_opened && i->var[j].is_variadic && i->var[j].addr) {
#else
				if (i->var[j].is_variadic && i->var[j].addr) {
#endif
					if (html) {
						xdebug_str_add(str, "<i>variadic</i>(", 0);
					} else {
						xdebug_str_add(str, "variadic(", 0);
					}
#if PHP_VERSION_ID >= 70000
					c = 0;
					variadic_opened = 1;
					continue;
#endif
				}

				if (i->var[j].addr) {
					add_single_value(str, i->var[j].addr, html, XG(collect_params) TSRMLS_CC);
#if PHP_VERSION_ID >= 50600 && PHP_VERSION_ID < 70000
				} else if (i->is_variadic && j == i->varc - 1) {
					if (html) {
						xdebug_str_add(str, "<i>variadic</i>(", 0);
					} else {
						xdebug_str_add(str, "variadic(", 0);
					}
#endif
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
				if (strlen(XG(file_link_format)) > 0) {
					char *just_filename = strrchr(i->filename, DEFAULT_SLASH);
					char *file_link;

					xdebug_format_file_link(&file_link, i->filename, i->lineno TSRMLS_CC);
					xdebug_str_add(str, xdebug_sprintf(formats[10], i->filename, file_link, just_filename, i->lineno), 1);
					xdfree(file_link);
				} else {
					char *just_filename = strrchr(i->filename, DEFAULT_SLASH);

					xdebug_str_add(str, xdebug_sprintf(formats[5], i->filename, just_filename, i->lineno), 1);
				}
			} else {
				xdebug_str_add(str, xdebug_sprintf(formats[5], i->filename, i->lineno), 1);
			}

			printed_frames++;
			if (XG(max_stack_frames) > 0 && printed_frames >= XG(max_stack_frames)) {
				break;
			}
		}

		if (XG(dump_globals) && !(XG(dump_once) && XG(dumped))) {
			char *tmp = xdebug_get_printable_superglobals(html TSRMLS_CC);

			if (tmp) {
				xdebug_str_add(str, tmp, 1);
			}
			XG(dumped) = 1;
		}

		if (XG(show_local_vars) && XG(stack) && XDEBUG_LLIST_TAIL(XG(stack))) {
			int scope_nr = XG(stack)->size;

			i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack)));
			if (i->user_defined == XDEBUG_INTERNAL && XDEBUG_LLIST_PREV(XDEBUG_LLIST_TAIL(XG(stack))) && XDEBUG_LLIST_VALP(XDEBUG_LLIST_PREV(XDEBUG_LLIST_TAIL(XG(stack))))) {
				i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_PREV(XDEBUG_LLIST_TAIL(XG(stack))));
				scope_nr--;
			}
			if (i->used_vars && i->used_vars->size) {
				xdebug_hash *tmp_hash;

				xdebug_str_add(str, xdebug_sprintf(formats[6], scope_nr), 1);
				tmp_hash = xdebug_used_var_hash_from_llist(i->used_vars);
				xdebug_hash_apply_with_argument(tmp_hash, (void*) &html, dump_used_var_with_contents, (void *) str);
				xdebug_hash_destroy(tmp_hash);
			}
		}
	}
}

void xdebug_append_error_footer(xdebug_str *str, int html TSRMLS_DC)
{
	char **formats = select_formats(html TSRMLS_CC);

	xdebug_str_add(str, formats[7], 0);
}

static char *get_printable_stack(int html, int error_type, char *buffer, const char *error_filename, const int error_lineno, int include_decription TSRMLS_DC)
{
	char *prepend_string;
	char *append_string;
	char *error_type_str = xdebug_error_type(error_type);
	char *error_type_str_simple = xdebug_error_type_simple(error_type);
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	prepend_string = INI_STR("error_prepend_string");
	append_string = INI_STR("error_append_string");

	xdebug_str_add(&str, prepend_string ? prepend_string : "", 0);
	xdebug_append_error_head(&str, html, error_type_str_simple TSRMLS_CC);
	if (include_decription) {
		xdebug_append_error_description(&str, html, error_type_str, buffer, error_filename, error_lineno TSRMLS_CC);
	}
	xdebug_append_printable_stack(&str, html TSRMLS_CC);
	xdebug_append_error_footer(&str, html TSRMLS_CC);
	xdebug_str_add(&str, append_string ? append_string : "", 0);

	xdfree(error_type_str);
	xdfree(error_type_str_simple);

	return str.d;
}

#if PHP_VERSION_ID >= 70000
# define XDEBUG_ZEND_HASH_STR_FIND(ht, str, size, var) var = zend_hash_str_find(Z_ARRVAL(ht), str, size);
# define XDEBUG_ZEND_HASH_RETURN_TYPE zval *
# define XDEBUG_ZEND_HASH_RETURN_VALUE Z_STRVAL_P
#else
# define XDEBUG_ZEND_HASH_STR_FIND(ht, str, size, var) zend_hash_find(Z_ARRVAL_P(ht), str, size, (void**) &var);
# define XDEBUG_ZEND_HASH_RETURN_TYPE zval **
# define XDEBUG_ZEND_HASH_RETURN_VALUE Z_STRVAL_PP
#endif

void xdebug_init_debugger(TSRMLS_D)
{
	xdebug_open_log(TSRMLS_C);
	if (XG(remote_connect_back)) {
		XDEBUG_ZEND_HASH_RETURN_TYPE remote_addr = NULL;

		XDEBUG_LOG_PRINT(XG(remote_log_file), "I: Checking remote connect back address.\n");
		if (XG(remote_addr_header) && XG(remote_addr_header)[0]) {
			XDEBUG_LOG_PRINT(XG(remote_log_file), "I: Checking user configured header '%s'.\n", XG(remote_addr_header));
			XDEBUG_ZEND_HASH_STR_FIND(PG(http_globals)[TRACK_VARS_SERVER], XG(remote_addr_header), HASH_KEY_STRLEN(XG(remote_addr_header)), remote_addr);
		}
		if (!remote_addr) {
			XDEBUG_LOG_PRINT(XG(remote_log_file), "I: Checking header 'HTTP_X_FORWARDED_FOR'.\n");
			XDEBUG_ZEND_HASH_STR_FIND(PG(http_globals)[TRACK_VARS_SERVER], "HTTP_X_FORWARDED_FOR", HASH_KEY_SIZEOF("HTTP_X_FORWARDED_FOR"), remote_addr);
		}
		if (!remote_addr) {
			XDEBUG_LOG_PRINT(XG(remote_log_file), "I: Checking header 'REMOTE_ADDR'.\n");
			XDEBUG_ZEND_HASH_STR_FIND(PG(http_globals)[TRACK_VARS_SERVER], "REMOTE_ADDR", HASH_KEY_SIZEOF("REMOTE_ADDR"), remote_addr);
		}
		if (remote_addr) {
			XDEBUG_LOG_PRINT(XG(remote_log_file), "I: Remote address found, connecting to %s:%ld.\n", XDEBUG_ZEND_HASH_RETURN_VALUE(remote_addr), (long int) XG(remote_port));
			XG(context).socket = xdebug_create_socket(XDEBUG_ZEND_HASH_RETURN_VALUE(remote_addr), XG(remote_port) TSRMLS_CC);
		} else {
			XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Remote address not found, connecting to configured address/port: %s:%ld. :-|\n", XG(remote_host), (long int) XG(remote_port));
			XG(context).socket = xdebug_create_socket(XG(remote_host), XG(remote_port) TSRMLS_CC);
		}
	} else {
		XDEBUG_LOG_PRINT(XG(remote_log_file), "I: Connecting to configured address/port: %s:%ld.\n", XG(remote_host), (long int) XG(remote_port));
		XG(context).socket = xdebug_create_socket(XG(remote_host), XG(remote_port) TSRMLS_CC);
	}
	if (XG(context).socket >= 0) {
		XDEBUG_LOG_PRINT(XG(remote_log_file), "I: Connected to client. :-)\n");
		XG(remote_enabled) = 0;

		/* Get handler from mode */
		XG(context).handler = xdebug_handler_get(XG(remote_handler));
		if (!XG(context).handler) {
			zend_error(E_WARNING, "The remote debug handler '%s' is not supported.", XG(remote_handler));
			XDEBUG_LOG_PRINT(XG(remote_log_file), "E: The remote debug handler '%s' is not supported. :-(\n", XG(remote_handler));
		} else if (!XG(context).handler->remote_init(&(XG(context)), XDEBUG_REQ)) {
			/* The request could not be started, ignore it then */
			XDEBUG_LOG_PRINT(XG(remote_log_file), "E: The debug session could not be started. :-(\n");
		} else {
			/* All is well, turn off script time outs */
#if PHP_VERSION_ID >= 70000
			zend_string *ini_name = zend_string_init("max_execution_time", sizeof("max_execution_time") - 1, 0);
			zend_string *ini_val = zend_string_init("0", sizeof("0") - 1, 0);

			zend_alter_ini_entry(ini_name, ini_val, PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);

			zend_string_release(ini_val);
			zend_string_release(ini_name);
#else
			zend_alter_ini_entry("max_execution_time", sizeof("max_execution_time"), "0", strlen("0"), PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
#endif
			XG(remote_enabled) = 1;
		}
	} else if (XG(context).socket == -1) {
		XDEBUG_LOG_PRINT(XG(remote_log_file), "E: Could not connect to client. :-(\n");
	} else if (XG(context).socket == -2) {
		XDEBUG_LOG_PRINT(XG(remote_log_file), "E: Time-out connecting to client. :-(\n");
	} else if (XG(context).socket == -3) {
		XDEBUG_LOG_PRINT(XG(remote_log_file), "E: No permission connecting to client. This could be SELinux related. :-(\n");
	}
	if (!XG(remote_enabled)) {
		xdebug_close_log(TSRMLS_C);
	}
}

void xdebug_do_jit(TSRMLS_D)
{
	if (!XG(remote_enabled) && XG(remote_enable) && (XG(remote_mode) == XDEBUG_JIT)) {
		xdebug_init_debugger(TSRMLS_C);
	}
}

static void php_output_error(const char *error TSRMLS_DC)
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

/* Error callback for formatting stack traces */
void xdebug_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args)
{
	char *buffer, *error_type_str;
	int buffer_len;
	xdebug_brk_info *extra_brk_info = NULL;
	error_handling_t  error_handling;
	zend_class_entry *exception_class;

	TSRMLS_FETCH();

	buffer_len = vspprintf(&buffer, PG(log_errors_max_len), format, args);

	error_type_str = xdebug_error_type(type);

	/* Store last error message for error_get_last() */
	if (PG(last_error_message)) {
		free(PG(last_error_message));
	}
	if (PG(last_error_file)) {
		free(PG(last_error_file));
	}
	PG(last_error_type) = type;
	PG(last_error_message) = strdup(buffer);
	PG(last_error_file) = strdup(error_filename);
	PG(last_error_lineno) = error_lineno;
	error_handling  = EG(error_handling);
	exception_class = EG(exception_class);
	/* according to error handling mode, suppress error, throw exception or show it */
	if (error_handling != EH_NORMAL
#if PHP_VERSION_ID < 70000
		&& EG(in_execution)
#endif
	) {
		switch (type) {
			case E_CORE_ERROR:
			case E_COMPILE_ERROR:
			case E_PARSE:
				/* fatal errors are real errors and cannot be made exceptions */
				break;
			case E_STRICT:
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
				if (error_handling == EH_THROW && !EG(exception)) {
					zend_throw_error_exception(exception_class, buffer, 0, type TSRMLS_CC);
				}
				efree(buffer);
				xdfree(error_type_str);
				return;
		}
	}

	if ((EG(error_reporting | XG(force_error_reporting))) & type) {
		/* Log to logger */
		if (PG(log_errors)) {

#ifdef PHP_WIN32
			if (type==E_CORE_ERROR || type==E_CORE_WARNING) {
				MessageBox(NULL, buffer, error_type_str, MB_OK);
			}
#endif
			xdebug_log_stack(error_type_str, buffer, error_filename, error_lineno TSRMLS_CC);
		}

		/* Display errors */
		if ((PG(display_errors) || XG(force_display_errors)) && !PG(during_request_startup)) {
			char *printable_stack;

			/* We need to see if we have an uncaught exception fatal error now */
			if (type == E_ERROR && strncmp(buffer, "Uncaught ", 9) == 0) {
				xdebug_str str = XDEBUG_STR_INITIALIZER;
				char *tmp_buf, *p;

				/* find first new line */
				p = strchr(buffer, '\n');
				if (!p) {
					p = buffer + strlen(buffer);
				} else {
#if PHP_VERSION_ID >= 70000
					/* find the last " in ", which isn't great and might not work... but in most cases it will */
					p = xdebug_strrstr(buffer, " in ");
#else
					/* find last quote */
					p = ((char *) zend_memrchr(buffer, '\'', p - buffer));
					if (p) {
						p++;
					}
#endif
					if (!p) {
						p = buffer + strlen(buffer);
					}
				}
				/* Create new buffer */
				tmp_buf = calloc(p - buffer + 1, 1);
				strncpy(tmp_buf, buffer, p - buffer );

				/* Append error */
				xdebug_append_error_head(&str, PG(html_errors), "uncaught-exception" TSRMLS_CC);
				xdebug_append_error_description(&str, PG(html_errors), error_type_str, tmp_buf, error_filename, error_lineno TSRMLS_CC);
				xdebug_append_printable_stack(&str, PG(html_errors) TSRMLS_CC);
				if (XG(last_exception_trace)) {
					xdebug_str_add(&str, XG(last_exception_trace), 0);
				}
				xdebug_append_error_footer(&str, PG(html_errors) TSRMLS_CC);
				php_output_error(str.d TSRMLS_CC);

				xdfree(str.d);
				free(tmp_buf);
			} else {
				printable_stack = get_printable_stack(PG(html_errors), type, buffer, error_filename, error_lineno, 1 TSRMLS_CC);
				if (XG(do_collect_errors) && (type != E_ERROR) && (type != E_COMPILE_ERROR) && (type != E_USER_ERROR)) {
					xdebug_llist_insert_next(XG(collected_errors), XDEBUG_LLIST_TAIL(XG(collected_errors)), printable_stack);
				} else {
					php_output_error(printable_stack TSRMLS_CC);
					xdfree(printable_stack);
				}
			}
		} else if (XG(do_collect_errors)) {
			char *printable_stack;
			printable_stack = get_printable_stack(PG(html_errors), type, buffer, error_filename, error_lineno, 1 TSRMLS_CC);
			xdebug_llist_insert_next(XG(collected_errors), XDEBUG_LLIST_TAIL(XG(collected_errors)), printable_stack);
		}
	}

	/* Start JIT if requested and not yet enabled */
	xdebug_do_jit(TSRMLS_C);

	/* Check for the pseudo exceptions to allow breakpoints on PHP error statuses */
	if (XG(remote_enabled) && XG(breakpoints_allowed)) {
		if (
			xdebug_hash_find(XG(context).exception_breakpoints, error_type_str, strlen(error_type_str), (void *) &extra_brk_info) ||
			xdebug_hash_find(XG(context).exception_breakpoints, "*", 1, (void *) &extra_brk_info)
		) {
			if (xdebug_handle_hit_value(extra_brk_info)) {
				char *type_str = xdebug_sprintf("%ld", type);

				if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), (char *) error_filename, error_lineno, XDEBUG_BREAK, error_type_str, type_str, buffer)) {
					XG(remote_enabled) = 0;
				}

				xdfree(type_str);
			}
		}
	}
	xdfree(error_type_str);

	if (type & XG(halt_level) & XDEBUG_ALLOWED_HALT_LEVELS) {
		type = E_USER_ERROR;
	}

	/* Bail out if we can't recover */
	switch (type) {
		case E_CORE_ERROR:
			if (!php_get_module_initialized()) {
				/* bad error in module startup - no way we can live with this */
				exit(-2);
			}
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

					ctr.line = "HTTP/1.0 500 Internal Server Error";
					ctr.line_len = sizeof("HTTP/1.0 500 Internal Server Error") - 1;
					sapi_header_op(SAPI_HEADER_REPLACE, &ctr TSRMLS_CC);
				}
				/* the parser would return 1 (failure), we can bail out nicely */
				if (type != E_PARSE) {
					/* restore memory limit */
					zend_set_memory_limit(PG(memory_limit));
					zend_objects_store_mark_destructed(&EG(objects_store) TSRMLS_CC);
					zend_bailout();
					return;
				}
			}
			break;
	}

#if PHP_VERSION_ID >= 70000
	if (PG(track_errors) && EG(valid_symbol_table)) {
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
#else
	if (PG(track_errors) && EG(active_symbol_table)) {
		zval *tmp;
		ALLOC_ZVAL(tmp);
		INIT_PZVAL(tmp);
		Z_STRVAL_P(tmp) = (char *) estrndup(buffer, buffer_len);
		Z_STRLEN_P(tmp) = buffer_len;
		Z_TYPE_P(tmp) = IS_STRING;
		zend_hash_update(EG(active_symbol_table), "php_errormsg", sizeof("php_errormsg"), (void **) & tmp, sizeof(zval *), NULL);
	}
#endif

	efree(buffer);
}

/* {{{ proto void xdebug_print_function_stack([string message [, int options])
   Displays a stack trace */
PHP_FUNCTION(xdebug_print_function_stack)
{
	char *message = NULL;
#if PHP_VERSION_ID >= 70000
	size_t message_len;
#else
	int message_len;
#endif
	function_stack_entry *i;
	char *tmp;
	zppLONG options = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl", &message, &message_len, &options) == FAILURE) {
		return;
	}

	i = xdebug_get_stack_frame(0 TSRMLS_CC);
	if (message) {
		tmp = get_printable_stack(PG(html_errors), 0, message, i->filename, i->lineno, !(options & XDEBUG_STACK_NO_DESC) TSRMLS_CC);
	} else {
		tmp = get_printable_stack(PG(html_errors), 0, "user triggered", i->filename, i->lineno, !(options & XDEBUG_STACK_NO_DESC) TSRMLS_CC);
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

	i = xdebug_get_stack_frame(0 TSRMLS_CC);
	tmp = get_printable_stack(PG(html_errors), 0, "user triggered", i->filename, i->lineno, 1 TSRMLS_CC);
#if PHP_VERSION_ID >= 70000
	RETVAL_STRING(tmp);
#else
	RETVAL_STRING(tmp, 1);
#endif
	xdfree(tmp);
}
/* }}} */

/* {{{ proto string xdebug_call_class()
   Returns the name of the calling class */
PHP_FUNCTION(xdebug_call_class)
{
	function_stack_entry *i;
	zppLONG depth = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &depth) == FAILURE) {
		return;
	}
	i = xdebug_get_stack_frame(2 + depth TSRMLS_CC);
	if (i) {
#if PHP_VERSION_ID >= 70000
		RETURN_STRING(i->function.class ? i->function.class : "");
#else
		RETURN_STRING(i->function.class ? i->function.class : "", 1);
#endif
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string xdebug_call_function()
   Returns the function name from which the current function was called from. */
PHP_FUNCTION(xdebug_call_function)
{
	function_stack_entry *i;
	zppLONG depth = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &depth) == FAILURE) {
		return;
	}
	i = xdebug_get_stack_frame(2 + depth TSRMLS_CC);
	if (i) {
#if PHP_VERSION_ID >= 70000
		RETURN_STRING(i->function.function ? i->function.function : "{}");
#else
		RETURN_STRING(i->function.function ? i->function.function : "{}", 1);
#endif
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int xdebug_call_line()
   Returns the line number where the current function was called from. */
PHP_FUNCTION(xdebug_call_line)
{
	function_stack_entry *i;
	zppLONG depth = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &depth) == FAILURE) {
		return;
	}
	i = xdebug_get_stack_frame(1 + depth TSRMLS_CC);
	if (i) {
		RETURN_LONG(i->lineno);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string xdebug_call_file()
   Returns the filename where the current function was called from. */
PHP_FUNCTION(xdebug_call_file)
{
	function_stack_entry *i;
	zppLONG depth = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &depth) == FAILURE) {
		return;
	}
	i = xdebug_get_stack_frame(1 + depth TSRMLS_CC);
	if (i) {
#if PHP_VERSION_ID >= 70000
		RETURN_STRING(i->filename);
#else
		RETURN_STRING(i->filename, 1);
#endif
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

static int find_line_number_for_current_execute_point(zend_execute_data *edata TSRMLS_DC)
{
	zend_execute_data *ptr = edata;

#if PHP_VERSION_ID >= 70000
	while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
		ptr = ptr->prev_execute_data;
	}
#else
	while (ptr && !ptr->opline)
	{
		ptr = ptr->prev_execute_data;
	}
#endif

	if (ptr && ptr->opline) {
		return ptr->opline->lineno;
	}

	return 0;
}

#if PHP_VERSION_ID >= 70000
static void xdebug_build_fname(xdebug_func *tmp, zend_execute_data *edata TSRMLS_DC)
{
	memset(tmp, 0, sizeof(xdebug_func));

#if PHP_VERSION_ID >= 70100
	if (edata && edata->func && edata->func == (zend_function*) &zend_pass_function) {
		tmp->type     = XFUNC_ZEND_PASS;
		tmp->function = xdstrdup("{zend_pass}");
	} else
#endif

	if (edata && edata->func) {
		tmp->type = XFUNC_NORMAL;
#if PHP_VERSION_ID >= 70100
		if ((Z_TYPE(edata->This)) == IS_OBJECT) {
#else
		if (edata->This.value.obj) {
#endif
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
			if (strcmp(edata->func->common.function_name->val, "{closure}") == 0) {
				tmp->function = xdebug_sprintf(
					"{closure:%s:%d-%d}",
					edata->func->op_array.filename->val,
					edata->func->op_array.line_start,
					edata->func->op_array.line_end
				);
			} else if (strncmp(edata->func->common.function_name->val, "call_user_func", 14) == 0) {
				const char *fname = NULL;
				int         lineno = 0;

				if (edata->prev_execute_data && edata->prev_execute_data->func->type == ZEND_USER_FUNCTION) {
					fname = edata->prev_execute_data->func->op_array.filename->val;
				}

				if (
					!fname &&
					XDEBUG_LLIST_TAIL(XG(stack)) &&
					XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))) &&
					((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))))->filename
				) {
					fname = ((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))))->filename;
				}

				if (!fname) {
					fname = "whoops";
				}

				lineno = find_line_number_for_current_execute_point(edata TSRMLS_CC);

				tmp->function = xdebug_sprintf(
					"%s:{%s:%d}",
					edata->func->common.function_name->val,
					fname,
					lineno
				);
			} else {
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
			return xdebug_build_fname(tmp, edata->prev_execute_data);
		} else {
			tmp->type = XFUNC_UNKNOWN;
		}
	}
}
#else
static void xdebug_build_fname(xdebug_func *tmp, zend_execute_data *edata TSRMLS_DC)
{
	memset(tmp, 0, sizeof(xdebug_func));

	if (edata) {
		if (edata->function_state.function->common.function_name) {
			if (edata->object) {
				tmp->type = XFUNC_MEMBER;
				if (edata->function_state.function->common.scope) { /* __autoload has no scope */
					tmp->class = xdstrdup(edata->function_state.function->common.scope->name);
				}
			} else if (EG(scope) && edata->function_state.function->common.scope && edata->function_state.function->common.scope->name) {
				tmp->type = XFUNC_STATIC_MEMBER;
				tmp->class = xdstrdup(edata->function_state.function->common.scope->name);
			} else {
				tmp->type = XFUNC_NORMAL;
			}
			if (strcmp(edata->function_state.function->common.function_name, "{closure}") == 0) {
				tmp->function = xdebug_sprintf(
					"{closure:%s:%d-%d}",
					edata->function_state.function->op_array.filename,
					edata->function_state.function->op_array.line_start,
					edata->function_state.function->op_array.line_end
				);
			} else if (strncmp(edata->function_state.function->common.function_name, "call_user_func", 14) == 0) {
				const char *fname = NULL;
				int         lineno = 0;

				if (edata->prev_execute_data && edata->prev_execute_data->function_state.function->type == ZEND_USER_FUNCTION) {
					fname = edata->prev_execute_data->function_state.function->op_array.filename;
				}

				if (
					!fname &&
					XDEBUG_LLIST_TAIL(XG(stack)) &&
					XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))) &&
					((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))))->filename
				) {
					fname = ((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))))->filename;
				}

				if (!fname) {
					fname = "whoops";
				}

				lineno = find_line_number_for_current_execute_point(edata TSRMLS_CC);

				tmp->function = xdebug_sprintf(
					"%s:{%s:%d}",
					edata->function_state.function->common.function_name,
					fname,
					lineno
				);
			} else {
				tmp->function = xdstrdup(edata->function_state.function->common.function_name);
			}
		} else {
			switch (edata->opline->extended_value) {
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
		}
	}
}
#endif

function_stack_entry *xdebug_add_stack_frame(zend_execute_data *zdata, zend_op_array *op_array, int type TSRMLS_DC)
{
	zend_execute_data    *edata;
	zend_op             **opline_ptr = NULL;
	function_stack_entry *tmp;
	zend_op              *cur_opcode;
#if PHP_VERSION_ID < 70000
	zval                **param;
#endif
	int                   i = 0;
	char                 *aggr_key = NULL;
	int                   aggr_key_len = 0;
#if PHP_VERSION_ID >= 70000
	int                   hit_variadic = 0;
	zend_string          *aggr_key_str = NULL;
#endif

#if PHP_VERSION_ID < 50500
	edata = EG(current_execute_data);
	opline_ptr = &EG(current_execute_data)->opline;
#else
	if (type == XDEBUG_EXTERNAL) {
		edata = EG(current_execute_data)->prev_execute_data;
		if (edata) {
			opline_ptr = (zend_op**) &edata->opline;
		}
	} else {
		edata = EG(current_execute_data);
		opline_ptr = (zend_op**) &EG(current_execute_data)->opline;
	}
# if PHP_VERSION_ID >= 70000
	zdata = EG(current_execute_data);
# endif
#endif

	tmp = xdmalloc (sizeof (function_stack_entry));
	tmp->var           = NULL;
	tmp->varc          = 0;
	tmp->refcount      = 1;
	tmp->level         = XG(level);
	tmp->arg_done      = 0;
	tmp->used_vars     = NULL;
	tmp->user_defined  = type;
	tmp->filename      = NULL;
	tmp->include_filename  = NULL;
	tmp->profile.call_list = NULL;
	tmp->op_array      = op_array;
	tmp->symbol_table  = NULL;
	tmp->execute_data  = NULL;
#if PHP_VERSION_ID >= 50600
	tmp->is_variadic   = 0;
#endif

	XG(function_count)++;
	tmp->function_nr = XG(function_count);
#if PHP_VERSION_ID >= 70000
	{
		zend_execute_data *ptr = edata;
		while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
			ptr = ptr->prev_execute_data;
		}
		if (ptr) {
			tmp->filename = xdstrdup(ptr->func->op_array.filename->val);
		}
	}
#else
	if (edata && edata->op_array) {
		/* Normal function calls */
		tmp->filename  = xdstrdup(edata->op_array->filename);
	} else if (
		edata &&
		edata->prev_execute_data &&
		XDEBUG_LLIST_TAIL(XG(stack)) &&
		((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))))->filename
	) {
		tmp->filename = xdstrdup(((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))))->filename);
	}
#endif

	if (!tmp->filename) {
		/* Includes/main script etc */
#if PHP_VERSION_ID >= 70000
		tmp->filename  = (type == XDEBUG_EXTERNAL && op_array && op_array->filename) ? xdstrdup(op_array->filename->val): NULL;
#else
		tmp->filename  = (op_array && op_array->filename) ? xdstrdup(op_array->filename): NULL;
#endif
	}
	/* Call user function locations */
	if (
		!tmp->filename &&
		XG(stack) &&
		XDEBUG_LLIST_TAIL(XG(stack)) &&
		XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))) &&
		((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))))->filename
	) {
		tmp->filename = xdstrdup(((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))))->filename);
	}

	if (!tmp->filename) {
		tmp->filename = xdstrdup("UNKNOWN?");
	}
	tmp->prev_memory = XG(prev_memory);
	tmp->memory = zend_memory_usage(0 TSRMLS_CC);
	XG(prev_memory) = tmp->memory;
	tmp->time   = xdebug_get_utime();
	tmp->lineno = 0;
	tmp->prev   = 0;

	xdebug_build_fname(&(tmp->function), zdata TSRMLS_CC);
	if (!tmp->function.type) {
		tmp->function.function = xdstrdup("{main}");
		tmp->function.class    = NULL;
		tmp->function.type     = XFUNC_NORMAL;

	} else if (tmp->function.type & XFUNC_INCLUDES) {
		tmp->lineno = 0;
		if (opline_ptr) {
			cur_opcode = *opline_ptr;
			if (cur_opcode) {
				tmp->lineno = cur_opcode->lineno;
			}
		}

		if (tmp->function.type == XFUNC_EVAL) {
			tmp->include_filename = xdebug_sprintf("%s", XG(last_eval_statement));
		} else if (XG(collect_includes)) {
			tmp->include_filename = xdstrdup(zend_get_executed_filename(TSRMLS_C));
		}
	} else  {
		tmp->lineno = find_line_number_for_current_execute_point(edata TSRMLS_CC);
#if PHP_VERSION_ID >= 70000
		tmp->is_variadic = zdata->func->common.fn_flags & ZEND_ACC_VARIADIC;
#elif PHP_VERSION_ID >= 50600
		tmp->is_variadic = op_array && (op_array->fn_flags & ZEND_ACC_VARIADIC);
#endif

		if (XG(remote_enabled) || XG(collect_params) || XG(collect_vars)) {
			int    arguments_sent = 0, arguments_wanted = 0, arguments_storage = 0;

			/* This calculates how many arguments where sent to a function. It
			 * works for both internal and user defined functions.
			 * op_array->num_args works only for user defined functions so
			 * we're not using that here. */
#if PHP_VERSION_ID >= 70000
			arguments_sent = ZEND_CALL_NUM_ARGS(zdata);
			arguments_wanted = arguments_sent;

			if (ZEND_USER_CODE(zdata->func->type)) {
				arguments_wanted = op_array->num_args;
			}

			if (ZEND_USER_CODE(zdata->func->type) && zdata->func->common.fn_flags & ZEND_ACC_VARIADIC) {
				arguments_wanted++;
				arguments_sent++;
			}
#else
			void **p;
			void **curpos = NULL;
			if ((!edata->opline) || ((edata->opline->opcode == ZEND_DO_FCALL_BY_NAME) || (edata->opline->opcode == ZEND_DO_FCALL))) {
				curpos = edata->function_state.arguments;
				arguments_sent = (int)(zend_uintptr_t) *curpos;
				arguments_wanted = arguments_sent;
				p = curpos - arguments_sent;
			} else {
				p = zend_vm_stack_top(TSRMLS_C) - 1;
				arguments_sent = (ulong) *p;
				arguments_wanted = arguments_sent;
				p = curpos = NULL;
			}

			if (tmp->user_defined == XDEBUG_EXTERNAL) {
				arguments_wanted = op_array->num_args;
			}
#endif
			if (arguments_wanted > arguments_sent) {
				arguments_storage = arguments_wanted;
			} else {
				arguments_storage = arguments_sent;
			}
			tmp->var = xdmalloc(arguments_storage * sizeof (xdebug_var));

#if PHP_VERSION_ID >= 70000
			for (i = 0; i < arguments_sent; i++) {
#else
			for (i = 0; i < arguments_sent; i++) {
#endif
				tmp->var[tmp->varc].name = NULL;
				tmp->var[tmp->varc].addr = NULL;
				tmp->var[tmp->varc].is_variadic = 0;

				/* Because it is possible that more parameters are sent, then
				 * actually wanted  we can only access the name in case there
				 * is an associated variable to receive the variable here. */
				if (tmp->user_defined == XDEBUG_EXTERNAL && i < arguments_wanted) {
					if (op_array->arg_info[i].name) {
						tmp->var[tmp->varc].name = xdstrdup(STR_NAME_VAL(op_array->arg_info[i].name));
					}
#if PHP_VERSION_ID >= 50600
					if (op_array->arg_info[i].is_variadic) {
						tmp->var[tmp->varc].is_variadic = 1;
					}
#endif
#if PHP_VERSION_ID >= 70000
					if (op_array->arg_info[i].is_variadic && !hit_variadic) {
						tmp->var[tmp->varc].is_variadic = 1;
						hit_variadic = 1;
					}
#endif
				}

				if (XG(collect_params)) {
#if PHP_VERSION_ID >= 70000
					if ((i < arguments_wanted) || ((zdata->func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE) && (i < arguments_sent))) {
						if (ZEND_CALL_ARG(zdata, tmp->varc+1)) {
							tmp->var[tmp->varc].addr = ZEND_CALL_ARG(zdata, tmp->varc+1);
						}
					} else {
						tmp->var[tmp->varc].addr = ZEND_CALL_VAR_NUM(zdata, zdata->func->op_array.last_var + zdata->func->op_array.T + i - arguments_wanted);
					}
#else
					if (p) {
						param = (zval **) p++;
						tmp->var[tmp->varc].addr = *param;
					}
#endif
				}
				tmp->varc++;
			}

			/* Sometimes not enough arguments are send to a user defined
			 * function, so we have to gather only the name for those extra. */
			if (tmp->user_defined == XDEBUG_EXTERNAL && arguments_sent < arguments_wanted) {
				for (i = arguments_sent; i < arguments_wanted; i++) {
					if (op_array->arg_info[i].name) {
						tmp->var[tmp->varc].name = xdstrdup(STR_NAME_VAL(op_array->arg_info[i].name));
					}
					tmp->var[tmp->varc].addr = NULL;
					tmp->var[tmp->varc].is_variadic = 0;
					tmp->varc++;
				}
			}
		}
	}

	if (XG(do_code_coverage)) {
		xdebug_count_line(tmp->filename, tmp->lineno, 0, 0 TSRMLS_CC);
	}

	if (XG(do_monitor_functions)) {
		char *func_name = xdebug_show_fname(tmp->function, 0, 0 TSRMLS_CC);
		int   func_name_len = strlen(func_name);
		void *dummy;

		if (xdebug_hash_find(XG(functions_to_monitor), func_name, func_name_len, (void *) &dummy)) {
			xdebug_function_monitor_record(func_name, tmp->filename, tmp->lineno TSRMLS_CC);
		}

		xdfree(func_name);
	}

	if (XG(profiler_aggregate)) {
		char *func_name = xdebug_show_fname(tmp->function, 0, 0 TSRMLS_CC);

		aggr_key = xdebug_sprintf("%s.%s.%d", tmp->filename, func_name, tmp->lineno);
		aggr_key_len = strlen(aggr_key);
#if PHP_VERSION_ID >= 70000
		aggr_key_str = zend_string_init(aggr_key, aggr_key_len, 0);
		if ((tmp->aggr_entry = zend_hash_find_ptr(&XG(aggr_calls), aggr_key_str)) == NULL) {
#else
		if (zend_hash_find(&XG(aggr_calls), aggr_key, aggr_key_len+1, (void**)&tmp->aggr_entry) == FAILURE) {
#endif
			xdebug_aggregate_entry xae;

			if (tmp->user_defined == XDEBUG_EXTERNAL) {
#if PHP_VERSION_ID >= 70000
				xae.filename = xdstrdup(tmp->op_array->filename->val);
#else
				xae.filename = xdstrdup(tmp->op_array->filename);
#endif
			} else {
				xae.filename = xdstrdup("php:internal");
			}
			xae.function = func_name;
			xae.lineno = tmp->lineno;
			xae.user_defined = tmp->user_defined;
			xae.call_count = 0;
			xae.time_own = 0;
			xae.time_inclusive = 0;
			xae.call_list = NULL;

#if PHP_VERSION_ID >= 70000
			zend_hash_add_ptr(&XG(aggr_calls), aggr_key_str, &xae);
#else
			zend_hash_add(&XG(aggr_calls), aggr_key, aggr_key_len+1, (void*)&xae, sizeof(xdebug_aggregate_entry), (void**)&tmp->aggr_entry);
#endif
		}
	}

	if (XG(stack)) {
		if (XDEBUG_LLIST_TAIL(XG(stack))) {
			function_stack_entry *prev = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack)));
			tmp->prev = prev;
			if (XG(profiler_aggregate)) {
				if (prev->aggr_entry->call_list) {
#if PHP_VERSION_ID >= 70000
					if (!zend_hash_exists(prev->aggr_entry->call_list, aggr_key_str)) {
						zend_hash_add_ptr(prev->aggr_entry->call_list, aggr_key_str, tmp->aggr_entry);
#else
					if (!zend_hash_exists(prev->aggr_entry->call_list, aggr_key, aggr_key_len+1)) {
						zend_hash_add(prev->aggr_entry->call_list, aggr_key, aggr_key_len+1, (void*)&tmp->aggr_entry, sizeof(xdebug_aggregate_entry*), NULL);
#endif
					}
				} else {
					prev->aggr_entry->call_list = xdmalloc(sizeof(HashTable));
					zend_hash_init_ex(prev->aggr_entry->call_list, 1, NULL, NULL, 1, 0);
#if PHP_VERSION_ID >= 70000
					zend_hash_add_ptr(prev->aggr_entry->call_list, aggr_key_str, tmp->aggr_entry);
#else
					zend_hash_add(prev->aggr_entry->call_list, aggr_key, aggr_key_len+1, (void*)&tmp->aggr_entry, sizeof(xdebug_aggregate_entry*), NULL);
#endif
				}
			}
		}
		xdebug_llist_insert_next(XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), tmp);
	}

	if (XG(profiler_aggregate)) {
#if PHP_VERSION_ID >= 70000
		zend_string_release(aggr_key_str);
#endif
		xdfree(aggr_key);
	}

	return tmp;
}

int xdebug_handle_hit_value(xdebug_brk_info *brk_info)
{
	/* If this is a temporary breakpoint, disable the breakpoint */
	if (brk_info->temporary) {
		brk_info->disabled = 1;
	}

	/* Increase hit counter */
	brk_info->hit_count++;

	/* If the hit_value is 0, the condition check is disabled */
	if (!brk_info->hit_value) {
		return 1;
	}

	switch (brk_info->hit_condition) {
		case XDEBUG_HIT_GREATER_EQUAL:
			if (brk_info->hit_count >= brk_info->hit_value) {
				return 1;
			}
			break;
		case XDEBUG_HIT_EQUAL:
			if (brk_info->hit_count == brk_info->hit_value) {
				return 1;
			}
			break;
		case XDEBUG_HIT_MOD:
			if (brk_info->hit_count % brk_info->hit_value == 0) {
				return 1;
			}
			break;
		case XDEBUG_HIT_DISABLED:
			return 1;
			break;
	}
	return 0;
}

/* {{{ proto int xdebug_get_stack_depth()
   Returns the stack depth */
PHP_FUNCTION(xdebug_get_stack_depth)
{
	/* We substract one so that the function call to xdebug_get_stack_depth()
	 * is not part of the returned depth. */
	RETURN_LONG(XG(stack)->size - 1);
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
	char                 *argument = NULL;

	array_init(return_value);
	le = XDEBUG_LLIST_HEAD(XG(stack));

	for (k = 0; k < XG(stack)->size - 1; k++, le = XDEBUG_LLIST_NEXT(le)) {
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
			add_assoc_string_ex(frame, "function", HASH_KEY_SIZEOF("function"), i->function.function ADD_STRING_COPY);
		}
		if (i->function.class) {
			add_assoc_string_ex(frame, "type",     HASH_KEY_SIZEOF("type"),     i->function.type == XFUNC_STATIC_MEMBER ? "static" : "dynamic" ADD_STRING_COPY);
			add_assoc_string_ex(frame, "class",    HASH_KEY_SIZEOF("class"),    i->function.class    ADD_STRING_COPY);
		}
		add_assoc_string_ex(frame, "file", HASH_KEY_SIZEOF("file"), i->filename ADD_STRING_COPY);
		add_assoc_long_ex(frame, "line", HASH_KEY_SIZEOF("line"), i->lineno);

		/* Add parameters */
		XDEBUG_MAKE_STD_ZVAL(params);
		array_init(params);
		add_assoc_zval_ex(frame, "params", HASH_KEY_SIZEOF("params"), params);

		for (j = 0; j < i->varc; j++) {
			int variadic_opened = 0;

			if (i->var[j].is_variadic) {
				zval *vparams;

				XDEBUG_MAKE_STD_ZVAL(vparams);
				array_init(vparams);

				if (i->var[j].name) {
					add_assoc_zval(params, i->var[j].name, vparams);
				} else {
					add_index_zval(params, j, vparams);
				}
#if PHP_VERSION_ID >= 70000
				efree(params);
#endif
				params = vparams;
				variadic_opened = 1;
#if PHP_VERSION_ID >= 70000
				continue;
#endif
			}
			if (i->var[j].addr) {
				argument = xdebug_get_zval_value(i->var[j].addr, 0, NULL);
#if PHP_VERSION_ID >= 50600 && PHP_VERSION_ID < 70000
			} else if (i->is_variadic && j == i->varc - 1) {
				/* do nothing */
#endif
			} else {
				argument = xdstrdup("???");
			}
			if (i->var[j].name && !variadic_opened && argument) {
				add_assoc_string_ex(params, i->var[j].name, HASH_KEY_STRLEN(i->var[j].name), argument ADD_STRING_COPY);
#if PHP_VERSION_ID >= 50600 && PHP_VERSION_ID < 70000
			} else if (!argument && i->is_variadic && j == i->varc - 1) {
				zval *tmp_ar;

				XDEBUG_MAKE_STD_ZVAL(tmp_ar);
				array_init(tmp_ar);
				if (i->var[j].name) {
					add_assoc_zval(params, i->var[j].name, tmp_ar);
				} else {
					add_index_zval(params, j, tmp_ar);
				}
#endif
			} else {
#if PHP_VERSION_ID >= 70000
				add_index_string(params, j - 1, argument ADD_STRING_COPY);
#else
				add_index_string(params, j, argument ADD_STRING_COPY);
#endif
			}
			if (argument) {
				xdfree(argument);
				argument = NULL;
			}
		}

		if (i->include_filename) {
			add_assoc_string_ex(frame, "include_filename", HASH_KEY_SIZEOF("include_filename"), i->include_filename ADD_STRING_COPY);
		}

		add_next_index_zval(return_value, frame);
#if PHP_VERSION_ID >= 70000
		efree(params);
		efree(frame);
#endif
	}
}
/* }}} */

void xdebug_attach_used_var_names(void *return_value, xdebug_hash_element *he)
{
	char *name = (char*) he->ptr;

	add_next_index_string(return_value, name ADD_STRING_COPY);
}

/* {{{ proto array xdebug_get_declared_vars()
   Returns an array representing the current stack */
PHP_FUNCTION(xdebug_get_declared_vars)
{
	xdebug_llist_element *le;
	function_stack_entry *i;
	xdebug_hash *tmp_hash;

	array_init(return_value);
	le = XDEBUG_LLIST_TAIL(XG(stack));
	le = XDEBUG_LLIST_PREV(le);
	i = XDEBUG_LLIST_VALP(le);

	/* Add declared vars */
	if (i->used_vars) {
		tmp_hash = xdebug_used_var_hash_from_llist(i->used_vars);
		xdebug_hash_apply(tmp_hash, (void *) return_value, xdebug_attach_used_var_names);
		xdebug_hash_destroy(tmp_hash);
	}
}
/* }}} */
