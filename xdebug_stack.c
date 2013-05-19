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
#include "xdebug_code_coverage.h"
#include "xdebug_compat.h"
#include "xdebug_profiler.h"
#include "xdebug_stack.h"
#include "xdebug_str.h"
#include "xdebug_superglobals.h"
#include "xdebug_var.h"
#include "ext/standard/html.h"

#include "main/php_ini.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static char* text_formats[11] = {
	"\n",
	"%s: %s in %s on line %d\n",
	"\nCall Stack:\n",
#if HAVE_PHP_MEMORY_USAGE
	"%10.4f %10ld %3d. %s(",
#else
	"%10.4f %3d. %s(",
#endif
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
#if HAVE_PHP_MEMORY_USAGE
	"%10.4f %10ld %3d. %s(",
#else
	"%10.4f %3d. %s(",
#endif
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
#if HAVE_PHP_MEMORY_USAGE
	"<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>\n<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>\n",
	"<tr><td bgcolor='#eeeeec' align='center'>%d</td><td bgcolor='#eeeeec' align='center'>%.4f</td><td bgcolor='#eeeeec' align='right'>%ld</td><td bgcolor='#eeeeec'>%s( ",
#else
	"<tr><th align='left' bgcolor='#e9b96e' colspan='4'>Call Stack</th></tr>\n<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>\n",
	"<tr><td bgcolor='#eeeeec' align='center'>%d</td><td bgcolor='#eeeeec' align='center'>%.4f</td><td bgcolor='#eeeeec'>%s( ",
#endif
	"<font color='#00bb00'>'%s'</font>",
	" )</td><td title='%s' bgcolor='#eeeeec'>..%s<b>:</b>%d</td></tr>\n",
#if HAVE_PHP_MEMORY_USAGE
	"<tr><th align='left' colspan='5' bgcolor='#e9b96e'>Variables in local scope (#%d)</th></tr>\n",
#else
	"<tr><th align='left' colspan='4' bgcolor='#e9b96e'>Variables in local scope (#%d)</th></tr>\n",
#endif
	"</table></font>\n",
	"<tr><td colspan='2' align='right' bgcolor='#eeeeec' valign='top'><pre>$%s&nbsp;=</pre></td><td colspan='3' bgcolor='#eeeeec'>%s</td></tr>\n",
	"<tr><td colspan='2' align='right' bgcolor='#eeeeec' valign='top'><pre>$%s&nbsp;=</pre></td><td colspan='3' bgcolor='#eeeeec' valign='top'><i>Undefined</i></td></tr>\n",
	" )</td><td title='%s' bgcolor='#eeeeec'><a style='color: black' href='%s'>..%s<b>:</b>%d</a></td></tr>\n",
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

#if PHP_VERSION_ID >= 50300
	if (!EG(active_symbol_table)) {
		zend_rebuild_symbol_table(TSRMLS_C);
	}
#endif

	tmp_ht = XG(active_symbol_table);
	XG(active_symbol_table) = EG(active_symbol_table);
	zvar = xdebug_get_php_symbol(name, strlen(name) + 1);
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
			int j = 0; /* Counter */
			char *tmp_name;
			xdebug_str log_buffer = {0, 0, NULL};

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
				tmp_varname = i->var[j].name ? xdebug_sprintf("$%s = ", i->var[j].name) : xdstrdup("");
				xdebug_str_add(&log_buffer, tmp_varname, 0);
				xdfree(tmp_varname);

				if (i->var[j].addr) {
					tmp_value = xdebug_get_zval_value(i->var[j].addr, 0, NULL);
					xdebug_str_add(&log_buffer, tmp_value, 0);
					xdfree(tmp_value);
				} else {
					xdebug_str_addl(&log_buffer, "*uninitialized*", 15, 0);
				}
			}

			xdebug_str_add(&log_buffer, xdebug_sprintf(") %s:%d", i->filename, i->lineno), 1);
			php_log_err(log_buffer.d TSRMLS_CC);
			xdebug_str_free(&log_buffer);
		}
	}
}

static int create_file_link(char **filename, const char *error_filename, int error_lineno TSRMLS_DC)
{
	xdebug_str fname = {0, 0, NULL};
	char      *format = XG(file_link_format);
	
	while (*format)
	{
		if (*format != '%') {
			xdebug_str_addl(&fname, (char *) format, 1, 0);
		} else {
			format++;
			switch (*format)
			{
				case 'f': /* filename */
					xdebug_str_add(&fname, xdebug_sprintf("%s", error_filename), 1);
					break;

				case 'l': /* line number */
					xdebug_str_add(&fname, xdebug_sprintf("%d", error_lineno), 1);
					break;

				case '%': /* literal % */
					xdebug_str_addl(&fname, "%", 1, 0);
					break;
			}
		}
		format++;
	}
	
	*filename = fname.d;

	return fname.l;
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
#if PHP_VERSION_ID >= 50400
	size_t newlen;
#else
	int    newlen;
#endif

	if (html) {
		escaped = php_escape_html_entities((unsigned char *) buffer, strlen(buffer), &newlen, 0, 0, NULL TSRMLS_CC);
	} else {
		escaped = estrdup(buffer);
	}

	if (strlen(XG(file_link_format)) > 0 && html) {
		char *file_link;

		create_file_link(&file_link, error_filename, error_lineno TSRMLS_CC);
		xdebug_str_add(str, xdebug_sprintf(formats[11], error_type_str, escaped, file_link, error_filename, error_lineno), 1);
		xdfree(file_link);
	} else {
		xdebug_str_add(str, xdebug_sprintf(formats[1], error_type_str, escaped, error_filename, error_lineno), 1);
	}

	efree(escaped);
}

void xdebug_append_printable_stack(xdebug_str *str, int html TSRMLS_DC)
{
	xdebug_llist_element *le;
	function_stack_entry *i;
	int    len;
	char **formats = select_formats(html TSRMLS_CC);

	if (XG(stack) && XG(stack)->size) {
		i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_HEAD(XG(stack)));

		xdebug_str_add(str, formats[2], 0);

		for (le = XDEBUG_LLIST_HEAD(XG(stack)); le != NULL; le = XDEBUG_LLIST_NEXT(le))
		{
			int c = 0; /* Comma flag */
			int j = 0; /* Counter */
			char *tmp_name;
			
			i = XDEBUG_LLIST_VALP(le);
			tmp_name = xdebug_show_fname(i->function, html, 0 TSRMLS_CC);
			if (html) {
#if HAVE_PHP_MEMORY_USAGE
				xdebug_str_add(str, xdebug_sprintf(formats[3], i->level, i->time - XG(start_time), i->memory, tmp_name), 1);
#else
				xdebug_str_add(str, xdebug_sprintf(formats[3], i->level, i->time - XG(start_time), tmp_name), 1);
#endif
			} else {
#if HAVE_PHP_MEMORY_USAGE
				xdebug_str_add(str, xdebug_sprintf(formats[3], i->time - XG(start_time), i->memory, i->level, tmp_name), 1);
#else
				xdebug_str_add(str, xdebug_sprintf(formats[3], i->time - XG(start_time), i->level, tmp_name), 1);
#endif
			}
			xdfree(tmp_name);

			/* Printing vars */
			for (j = 0; j < i->varc; j++) {
				char *tmp_value, *tmp_fancy_value, *tmp_fancy_synop_value;
				int newlen;

				if (c) {
					xdebug_str_addl(str, ", ", 2, 0);
				} else {
					c = 1;
				}

				if (i->var[j].name && XG(collect_params) >= 4) {
					if (html) {
						xdebug_str_add(str, xdebug_sprintf("<span>$%s = </span>", i->var[j].name), 1);
					} else {
						xdebug_str_add(str, xdebug_sprintf("$%s = ", i->var[j].name), 1);
					}
				}

				if (i->var[j].addr) {
					if (html) {
						tmp_value = xdebug_get_zval_value(i->var[j].addr, 0, NULL);
						tmp_fancy_value = xdebug_xmlize(tmp_value, strlen(tmp_value), &newlen);
						tmp_fancy_synop_value = xdebug_get_zval_synopsis_fancy("", i->var[j].addr, &len, 0, NULL TSRMLS_CC);
						switch (XG(collect_params)) {
							case 1: /* synopsis */
								xdebug_str_add(str, xdebug_sprintf("<span>%s</span>", tmp_fancy_synop_value), 1);
								break;
							case 2: /* synopsis + full in tooltip */
								xdebug_str_add(str, xdebug_sprintf("<span title='%s'>%s</span>", tmp_fancy_value, tmp_fancy_synop_value), 1);
								break;
							case 3: /* full */
							default:
								xdebug_str_add(str, xdebug_sprintf("<span>%s</span>", tmp_fancy_value), 1);
								break;
						}
						xdfree(tmp_value);
						efree(tmp_fancy_value);
						xdfree(tmp_fancy_synop_value);
					} else {
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
							xdebug_str_add(str, xdebug_sprintf("%s", tmp_value), 1);
							xdfree(tmp_value);
						} else {
							xdebug_str_addl(str, "???", 3, 0);
						}
					}
				} else {
					xdebug_str_addl(str, "???", 3, 0);
				}
			}

			if (i->include_filename) {
				xdebug_str_add(str, xdebug_sprintf(formats[4], i->include_filename), 1);
			}

			if (html) {
				if (strlen(XG(file_link_format)) > 0) {
					char *just_filename = strrchr(i->filename, DEFAULT_SLASH);
					char *file_link;

					create_file_link(&file_link, i->filename, i->lineno TSRMLS_CC);
					xdebug_str_add(str, xdebug_sprintf(formats[10], i->filename, file_link, just_filename, i->lineno), 1);
					xdfree(file_link);
				} else {
					char *just_filename = strrchr(i->filename, DEFAULT_SLASH);

					xdebug_str_add(str, xdebug_sprintf(formats[5], i->filename, just_filename, i->lineno), 1);
				}
			} else {
				xdebug_str_add(str, xdebug_sprintf(formats[5], i->filename, i->lineno), 1);
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

static char *get_printable_stack(int html, int error_type, char *buffer, const char *error_filename, const int error_lineno TSRMLS_DC)
{
	char *prepend_string;
	char *append_string;
	char *error_type_str = xdebug_error_type(error_type);
	char *error_type_str_simple = xdebug_error_type_simple(error_type);
	xdebug_str str = {0, 0, NULL};

	prepend_string = INI_STR("error_prepend_string");
	append_string = INI_STR("error_append_string");

	xdebug_str_add(&str, prepend_string ? prepend_string : "", 0);
	xdebug_append_error_head(&str, html, error_type_str_simple TSRMLS_CC);
	xdebug_append_error_description(&str, html, error_type_str, buffer, error_filename, error_lineno TSRMLS_CC);
	xdebug_append_printable_stack(&str, html TSRMLS_CC);
	xdebug_append_error_footer(&str, html TSRMLS_CC);
	xdebug_str_add(&str, append_string ? append_string : "", 0);

	xdfree(error_type_str);
	xdfree(error_type_str_simple);

	return str.d;
}

#define XDEBUG_LOG_PRINT(fs, string, ...) if (fs) { fprintf(fs, string, ## __VA_ARGS__); }

void xdebug_init_debugger(TSRMLS_D)
{
	xdebug_open_log(TSRMLS_C);
	if (XG(remote_connect_back)) {
		zval **remote_addr = NULL;
		XDEBUG_LOG_PRINT(XG(remote_log_file), "I: Checking remote connect back address.\n");
		if (zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_X_FORWARDED_FOR", 21, (void**)&remote_addr) == FAILURE) {
			zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "REMOTE_ADDR", 12, (void**)&remote_addr);
		}
		if (remote_addr) {
			XDEBUG_LOG_PRINT(XG(remote_log_file), "I: Remote address found, connecting to %s:%ld.\n", Z_STRVAL_PP(remote_addr), XG(remote_port));
			XG(context).socket = xdebug_create_socket(Z_STRVAL_PP(remote_addr), XG(remote_port));
		} else {
			XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Remote address not found, connecting to configured address/port: %s:%ld. :-|\n", XG(remote_host), XG(remote_port));
			XG(context).socket = xdebug_create_socket(XG(remote_host), XG(remote_port));
		}
	} else {
		XDEBUG_LOG_PRINT(XG(remote_log_file), "I: Connecting to configured address/port: %s:%ld.\n", XG(remote_host), XG(remote_port));
		XG(context).socket = xdebug_create_socket(XG(remote_host), XG(remote_port));
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
			zend_alter_ini_entry("max_execution_time", sizeof("max_execution_time"), "0", strlen("0"), PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
			XG(remote_enabled) = 1;
		}
	} else {
		XDEBUG_LOG_PRINT(XG(remote_log_file), "E: Could not connect to client. :-(\n");
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
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 2) || PHP_MAJOR_VERSION >= 6
	PG(last_error_type) = type;
#endif
	PG(last_error_message) = strdup(buffer);
	PG(last_error_file) = strdup(error_filename);
	PG(last_error_lineno) = error_lineno;
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || PHP_MAJOR_VERSION >= 6
	error_handling  = EG(error_handling);
	exception_class = EG(exception_class);
#else
	error_handling  = PG(error_handling);
	exception_class = PG(exception_class);
#endif
	/* according to error handling mode, suppress error, throw exception or show it */
	if (error_handling != EH_NORMAL && EG(in_execution)) {
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

	if (EG(error_reporting) & type) {
		/* Log to logger */
		if (PG(log_errors)) {

#ifdef PHP_WIN32
			if (type==E_CORE_ERROR || type==E_CORE_WARNING) {
				MessageBox(NULL, buffer, error_type_str, MB_OK|ZEND_SERVICE_MB_STYLE);
			}
#endif
			xdebug_log_stack(error_type_str, buffer, error_filename, error_lineno TSRMLS_CC);
		}

		/* Display errors */
		if (PG(display_errors) && !PG(during_request_startup)) {
			char *printable_stack;

			/* We need to see if we have an uncaught exception fatal error now */
			if (type == E_ERROR && strncmp(buffer, "Uncaught exception", 18) == 0) {
				xdebug_str str = {0, 0, NULL};
				char *tmp_buf, *p;
				
				/* find first new line */
				p = strchr(buffer, '\n');
				if (!p) {
					p = buffer + strlen(buffer);
				} else {
					/* find last quote */
					p = ((char *) zend_memrchr(buffer, '\'', p - buffer)) + 1;
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
				printable_stack = get_printable_stack(PG(html_errors), type, buffer, error_filename, error_lineno TSRMLS_CC);
				if (XG(do_collect_errors) && (type != E_ERROR) && (type != E_COMPILE_ERROR) && (type != E_USER_ERROR)) {
					xdebug_llist_insert_next(XG(collected_errors), XDEBUG_LLIST_TAIL(XG(collected_errors)), printable_stack);
				} else {
					php_output_error(printable_stack TSRMLS_CC);
					xdfree(printable_stack);
				}
			}
		} else if (XG(do_collect_errors)) {
			char *printable_stack;
			printable_stack = get_printable_stack(PG(html_errors), type, buffer, error_filename, error_lineno TSRMLS_CC);
			xdebug_llist_insert_next(XG(collected_errors), XDEBUG_LLIST_TAIL(XG(collected_errors)), printable_stack);
		}
	}

	/* Start JIT if requested and not yet enabled */
	xdebug_do_jit(TSRMLS_C);

	/* Check for the pseudo exceptions to allow breakpoints on PHP error statuses */
	if (XG(remote_enabled) && XG(breakpoints_allowed)) {
		if (xdebug_hash_find(XG(context).exception_breakpoints, error_type_str, strlen(error_type_str), (void *) &extra_brk_info)) {
			if (xdebug_handle_hit_value(extra_brk_info)) {
				if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), (char *) error_filename, error_lineno, XDEBUG_BREAK, error_type_str, buffer)) {
					XG(remote_enabled) = 0;
				}
			}
		}
	}
	xdfree(error_type_str);

#if PHP_VERSION_ID < 50400

	/* Bail out if we can't recover */
	switch (type) {
		case E_CORE_ERROR:
		/* no break - intentionally */
		case E_ERROR:
#if PHP_VERSION_ID >= 50200 
		case E_RECOVERABLE_ERROR:
#endif
		/*case E_PARSE: the parser would return 1 (failure), we can bail out nicely */
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			EG(exit_status) = 255;
#if HAVE_PHP_MEMORY_USAGE
			/* restore memory limit */
# if PHP_VERSION_ID >= 50200 
			zend_set_memory_limit(PG(memory_limit));
# else
			AG(memory_limit) = PG(memory_limit);
# endif
#endif
			zend_objects_store_mark_destructed(&EG(objects_store) TSRMLS_CC);
			zend_bailout();
			return;
	}

#else

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
					sapi_header_line ctr = {0};

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

#endif

	if (PG(track_errors) && EG(active_symbol_table)) {
		zval *tmp;

		ALLOC_ZVAL(tmp);
		INIT_PZVAL(tmp);
		Z_STRVAL_P(tmp) = (char *) estrndup(buffer, buffer_len);
		Z_STRLEN_P(tmp) = buffer_len;
		Z_TYPE_P(tmp) = IS_STRING;
		zend_hash_update(EG(active_symbol_table), "php_errormsg", sizeof("php_errormsg"), (void **) & tmp, sizeof(zval *), NULL);
	}

	efree(buffer);
}

/* {{{ proto array xdebug_print_function_stack([string message])
   Displays a stack trace */
PHP_FUNCTION(xdebug_print_function_stack)
{
	char *message = NULL;
	int message_len;
	function_stack_entry *i;
	char *tmp;
  
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &message, &message_len) == FAILURE) {
		return;
	}
 
	i = xdebug_get_stack_frame(0 TSRMLS_CC);
	if (message) {
		tmp = get_printable_stack(PG(html_errors), 0, message, i->filename, i->lineno TSRMLS_CC);
	} else {
		tmp = get_printable_stack(PG(html_errors), 0, "user triggered", i->filename, i->lineno TSRMLS_CC);
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
	tmp = get_printable_stack(PG(html_errors), 0, "user triggered", i->filename, i->lineno TSRMLS_CC);
	RETVAL_STRING(tmp, 1);
	xdfree(tmp);
}
/* }}} */

/* {{{ proto string xdebug_call_class()
   Returns the name of the calling class */
PHP_FUNCTION(xdebug_call_class)
{
	function_stack_entry *i;
	long depth = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &depth) == FAILURE) {
		return;
	}
	i = xdebug_get_stack_frame(2 + depth TSRMLS_CC);
	if (i) {
		RETURN_STRING(i->function.class ? i->function.class : "", 1);
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
	long depth = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &depth) == FAILURE) {
		return;
	}
	i = xdebug_get_stack_frame(2 + depth TSRMLS_CC);
	if (i) {
		RETURN_STRING(i->function.function ? i->function.function : "{}", 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string xdebug_call_line()
   Returns the line number where the current function was called from. */
PHP_FUNCTION(xdebug_call_line)
{
	function_stack_entry *i;
	long depth = 0;

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

/* {{{ proto int xdebug_call_file()
   Returns the filename where the current function was called from. */
PHP_FUNCTION(xdebug_call_file)
{
	function_stack_entry *i;
	long depth = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &depth) == FAILURE) {
		return;
	}
	i = xdebug_get_stack_frame(1 + depth TSRMLS_CC);
	if (i) {
		RETURN_STRING(i->filename, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

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
			} else {
				tmp->function = xdstrdup(edata->function_state.function->common.function_name);
			}
		} else {
#if PHP_VERSION_ID >= 50399
			switch (edata->opline->extended_value) {
#else
			switch (edata->opline->op2.u.constant.value.lval) {
#endif
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

function_stack_entry *xdebug_add_stack_frame(zend_execute_data *zdata, zend_op_array *op_array, int type TSRMLS_DC)
{
	zend_execute_data    *edata;
	zend_op             **opline_ptr = NULL;
	function_stack_entry *tmp;
	zend_op              *cur_opcode;
	zval                **param;
	int                   i = 0;
	char                 *aggr_key = NULL;
	int                   aggr_key_len = 0;

#if PHP_VERSION_ID < 50500
	edata = EG(current_execute_data);
	opline_ptr = EG(opline_ptr);
#else
	if (type == XDEBUG_EXTERNAL) {
		edata = EG(current_execute_data)->prev_execute_data;
		if (edata) {
			opline_ptr = &edata->opline;
		}
	} else {
		edata = EG(current_execute_data);
		opline_ptr = EG(opline_ptr);
	}
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

	XG(function_count)++;
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

	if (!tmp->filename) {
		/* Includes/main script etc */
		tmp->filename  = (op_array && op_array->filename) ? xdstrdup(op_array->filename): NULL;
	}
	/* Call user function locations */
	if (
		!tmp->filename &&
		XDEBUG_LLIST_TAIL(XG(stack)) &&
		XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))) &&
		((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))))->filename
	) {
		tmp->filename = xdstrdup(((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))))->filename);
	}

	if (!tmp->filename) {
		tmp->filename = xdstrdup("UNKNOWN?");
	}
#if HAVE_PHP_MEMORY_USAGE
	tmp->prev_memory = XG(prev_memory);
	tmp->memory = XG_MEMORY_USAGE();
	XG(prev_memory) = tmp->memory;
#else
	tmp->memory = 0;
	tmp->prev_memory = 0;
#endif
	tmp->time   = xdebug_get_utime();
	tmp->lineno = 0;
	tmp->prev   = 0;

	xdebug_build_fname(&(tmp->function), zdata TSRMLS_CC);
	if (!tmp->function.type) {
		tmp->function.function = xdstrdup("{main}");
		tmp->function.class    = NULL;
		tmp->function.type     = XFUNC_NORMAL;

	} else if (tmp->function.type & XFUNC_INCLUDES) {
		if (opline_ptr) {
			cur_opcode = *opline_ptr;
			tmp->lineno = cur_opcode->lineno;
		} else {
			tmp->lineno = 0;
		}

		if (tmp->function.type == XFUNC_EVAL) {
			tmp->include_filename = xdebug_sprintf("%s", XG(last_eval_statement));
		} else if (XG(collect_includes)) {
			tmp->include_filename = xdstrdup(zend_get_executed_filename(TSRMLS_C));
		}
	} else  {
		if (edata->opline) {
			cur_opcode = edata->opline;
			if (cur_opcode) {
				tmp->lineno = cur_opcode->lineno;
			}
		} else if (edata->prev_execute_data && edata->prev_execute_data->opline) {
			cur_opcode = edata->prev_execute_data->opline;
			if (cur_opcode) {
				tmp->lineno = cur_opcode->lineno;
			} 
		}

		if (XG(remote_enabled) || XG(collect_params) || XG(collect_vars)) {
			void **p;
			int    arguments_sent = 0, arguments_wanted = 0, arguments_storage = 0;

			/* This calculates how many arguments where sent to a function. It
			 * works for both internal and user defined functions.
			 * op_array->num_args works only for user defined functions so
			 * we're not using that here. */
#if PHP_VERSION_ID >= 50300
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
#else
			if (EG(argument_stack).top >= 2) {
				p = EG(argument_stack).top_element - 2;
				arguments_sent = (ulong) *p;
				arguments_wanted = arguments_sent;
			}
#endif

			if (tmp->user_defined == XDEBUG_EXTERNAL) {
				arguments_wanted = op_array->num_args;
			}

			if (arguments_wanted > arguments_sent) {
				arguments_storage = arguments_wanted;
			} else {
				arguments_storage = arguments_sent;
			}
			tmp->var = xdmalloc(arguments_storage * sizeof (xdebug_var));

			for (i = 0; i < arguments_sent; i++) {
				tmp->var[tmp->varc].name = NULL;
				tmp->var[tmp->varc].addr = NULL;

				/* Because it is possible that more parameters are sent, then
				 * actually wanted  we can only access the name in case there
				 * is an associated variable to receive the variable here. */
				if (tmp->user_defined == XDEBUG_EXTERNAL && i < arguments_wanted) {
					if (op_array->arg_info[i].name) {
						tmp->var[tmp->varc].name = xdstrdup(op_array->arg_info[i].name);
					}
				}

				if (XG(collect_params)) {
#if PHP_VERSION_ID >= 50300
					if (p) {
						param = (zval **) p++;				
						tmp->var[tmp->varc].addr = *param;
					}
#else
					param = NULL;
					if (zend_ptr_stack_get_arg(tmp->varc + 1, (void**) &param TSRMLS_CC) == SUCCESS) {
						if (param) {
							tmp->var[tmp->varc].addr = *param;
						}
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
						tmp->var[tmp->varc].name = xdstrdup(op_array->arg_info[i].name);
					}
					tmp->var[tmp->varc].addr = NULL;
					tmp->varc++;
				}
			}
		}
	}

	if (XG(do_code_coverage)) {
		xdebug_count_line(tmp->filename, tmp->lineno, 0, 0 TSRMLS_CC);
	}

	if (XG(profiler_aggregate)) {
		char *func_name = xdebug_show_fname(tmp->function, 0, 0 TSRMLS_CC);

		aggr_key = xdebug_sprintf("%s.%s.%d", tmp->filename, func_name, tmp->lineno);
		aggr_key_len = strlen(aggr_key);

		if (zend_hash_find(&XG(aggr_calls), aggr_key, aggr_key_len+1, (void**)&tmp->aggr_entry) == FAILURE) {
			xdebug_aggregate_entry xae;

			if (tmp->user_defined == XDEBUG_EXTERNAL) {
				xae.filename = xdstrdup(tmp->op_array->filename);
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

			zend_hash_add(&XG(aggr_calls), aggr_key, aggr_key_len+1, (void*)&xae, sizeof(xdebug_aggregate_entry), (void**)&tmp->aggr_entry);
		}
	}

	if (XG(stack)) {
		if (XDEBUG_LLIST_TAIL(XG(stack))) {
			function_stack_entry *prev = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack)));
			tmp->prev = prev;
			if (XG(profiler_aggregate)) {
				if (prev->aggr_entry->call_list) {
					if (!zend_hash_exists(prev->aggr_entry->call_list, aggr_key, aggr_key_len+1)) {
						zend_hash_add(prev->aggr_entry->call_list, aggr_key, aggr_key_len+1, (void*)&tmp->aggr_entry, sizeof(xdebug_aggregate_entry*), NULL);
					}
				} else {
					prev->aggr_entry->call_list = xdmalloc(sizeof(HashTable));
					zend_hash_init_ex(prev->aggr_entry->call_list, 1, NULL, NULL, 1, 0);
					zend_hash_add(prev->aggr_entry->call_list, aggr_key, aggr_key_len+1, (void*)&tmp->aggr_entry, sizeof(xdebug_aggregate_entry*), NULL);
				}
			}
		}
		xdebug_llist_insert_next(XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), tmp);
	}

	if (XG(profiler_aggregate)) {
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

/* {{{ proto integet xdebug_get_stack_depth()
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
	int                   j;
	unsigned int          k;
	zval                 *frame;
	zval                 *params;
	char                 *argument;

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
		MAKE_STD_ZVAL(frame);
		array_init(frame);

		/* Add data */
		if (i->function.function) {
			add_assoc_string_ex(frame, "function", sizeof("function"), i->function.function, 1);
		}
		if (i->function.class) {
			add_assoc_string_ex(frame, "type",     sizeof("type"),     i->function.type == XFUNC_STATIC_MEMBER ? "static" : "dynamic", 1);
			add_assoc_string_ex(frame, "class",    sizeof("class"),    i->function.class,    1);
		}
		add_assoc_string_ex(frame, "file", sizeof("file"), i->filename, 1);
		add_assoc_long_ex(frame, "line", sizeof("line"), i->lineno);

		/* Add parameters */
		MAKE_STD_ZVAL(params);
		array_init(params);
		for (j = 0; j < i->varc; j++) {
			if (i->var[j].addr) {
				argument = xdebug_get_zval_value(i->var[j].addr, 0, NULL);
			} else {
				argument = xdstrdup("");
			}
			if (i->var[j].name) {
				add_assoc_string_ex(params, i->var[j].name, strlen(i->var[j].name) + 1, argument, 1);
			} else {
				add_index_string(params, j, argument, 1);
			}
			xdfree(argument);
		}
		add_assoc_zval_ex(frame, "params", sizeof("params"), params);

		if (i->include_filename) {
			add_assoc_string_ex(frame, "include_filename", sizeof("include_filename"), i->include_filename, 1);
		}

		add_next_index_zval(return_value, frame);
	}
}
/* }}} */

void xdebug_attach_used_var_names(void *return_value, xdebug_hash_element *he)
{
	char *name = (char*) he->ptr;
	
	add_next_index_string(return_value, name, 1);
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
