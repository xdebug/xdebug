/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2023 Derick Rethans                               |
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
#include "php_xdebug.h"

#include "main/php_ini.h"

#include "ext/standard/html.h"
#include "ext/standard/php_smart_string.h"
#include "zend_exceptions.h"
#include "zend_generators.h"

#include "monitor.h"
#include "stack.h"
#include "superglobals.h"

#include "base/filter.h"
#include "coverage/code_coverage.h"
#include "lib/compat.h"
#include "lib/lib_private.h"
#include "lib/str.h"
#include "lib/var_export_html.h"
#include "lib/var_export_line.h"
#include "profiler/profiler.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static const char* text_formats[22] = {
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
	"SCREAM:  Error suppression ignored for\n",
	NULL,
	NULL,

	// 13+ (for xdebug_append_printable_stack_from_zval)
	"\n%sCall Stack:\n",
	"",
	"%sThe stack is empty or not available\n",
	"%s%10.4F %10ld %3d. %s() %s:%d\n",
	"\n%s",
	"\n%sNested Exceptions:\n",
	"", // nested exceptions footer
	NULL, // alternative to 16 for html only
	"\t" // indenter
};

static const char* ansi_formats[22] = {
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
	"[1m[31mSCREAM[0m:  Error suppression ignored for\n",
	NULL,
	NULL,

	// 13+ (for xdebug_append_printable_stack_from_zval)
	"\n%s[1mCall Stack:[22m\n",
	"",
	"%sThe stack is empty or not available\n",
	"%s%10.4F %10ld %3d. %s() %s:%d\n",
	"\n%s",
	"\n%s[1mNested Exceptions:[22m\n",
	"", // nested exceptions footer
	NULL, // alternative to 16 for html only
	"\t" // indenter
};

static const char* html_formats[22] = {
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
	"<tr><th align='left' bgcolor='#f57900' colspan=\"5\"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> SCREAM: Error suppression ignored for</th></tr>\n",

	// 13+ (for xdebug_append_printable_stack_from_zval)
	"%s<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>\n<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>\n",
	"",
	"%s<tr><td colspan='5' bgcolor='#eeeeec'>The stack is empty or not available</td></tr>\n",
	"%s<tr><td bgcolor='#eeeeec' align='center'>%d</td><td bgcolor='#eeeeec' align='center'>%.4F</td><td bgcolor='#eeeeec' align='right'>%ld</td><td bgcolor='#eeeeec'>%s()</td><td title='%s' bgcolor='#eeeeec'><a style='color: black' href='%s'>%s<b>:</b>%d</a></td></tr>\n",
	"%s<table class='xdebug-error xe-nested' style='width: 80%; margin: 1em' dir='ltr' border='1' cellspacing='0' cellpadding='1'>\n",
	"%s<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Nested Exceptions</th></tr><tr><td colspan='5'>\n",
	"</table></tr>\n", // nested exceptions footer
	"<tr><td bgcolor='#eeeeec' align='center'>%d</td><td bgcolor='#eeeeec' align='center'>%.4F</td><td bgcolor='#eeeeec' align='right'>%ld</td><td bgcolor='#eeeeec'>%s()</td><td title='%s' bgcolor='#eeeeec'>%s<b>:</b>%d</td></tr>\n",
	"" // indenter (not used for HTML)
};

static const char** select_formats(int html)
{
	if (html) {
		return html_formats;
	} else if ((XINI_DEV(cli_color) == 1 && xdebug_is_output_tty()) || (XINI_DEV(cli_color) == 2)) {
		return ansi_formats;
	} else {
		return text_formats;
	}
}

void xdebug_log_stack(const char *error_type_str, char *buffer, const char *error_filename, const int error_lineno)
{
	char *tmp_log_message;
	int   i;
	function_stack_entry *fse;

	tmp_log_message = xdebug_sprintf( "PHP %s:  %s in %s on line %d", error_type_str, buffer, error_filename, error_lineno);
	php_log_err(tmp_log_message);
	xdfree(tmp_log_message);

	if (!XG_BASE(stack) || XDEBUG_VECTOR_COUNT(XG_BASE(stack)) < 1) {
		return;
	}

	fse = XDEBUG_VECTOR_HEAD(XG_BASE(stack));

	php_log_err((char*) "PHP Stack trace:");

	for (i = 0; i < XDEBUG_VECTOR_COUNT(XG_BASE(stack)); i++, fse++)
	{
		int c = 0; /* Comma flag */
		unsigned int j = 0; /* Counter */
		char *tmp_name;
		xdebug_str log_buffer = XDEBUG_STR_INITIALIZER;
		int variadic_opened = 0;
		int sent_variables = fse->varc;

		if (sent_variables > 0 && fse->var[sent_variables-1].is_variadic && Z_ISUNDEF(fse->var[sent_variables-1].data)) {
			sent_variables--;
		}

		tmp_name = xdebug_show_fname(fse->function, XDEBUG_SHOW_FNAME_DEFAULT);
		xdebug_str_add_fmt(&log_buffer, "PHP %3d. %s(", fse->level, tmp_name);
		xdfree(tmp_name);

		/* Printing vars */
		for (j = 0; j < sent_variables; j++) {
			xdebug_str *tmp_value;

			if (c) {
				xdebug_str_add_literal(&log_buffer, ", ");
			} else {
				c = 1;
			}

			if (fse->var[j].is_variadic) {
				xdebug_str_add_literal(&log_buffer, "...");
				variadic_opened = 1;
			}

			if (fse->var[j].name) {
				xdebug_str_add_fmt(&log_buffer, "$%s = ", ZSTR_VAL(fse->var[j].name));
			}

			if (fse->var[j].is_variadic) {
				xdebug_str_add_literal(&log_buffer, "variadic(");
				c = 0;
				continue;
			}

			if (!Z_ISUNDEF(fse->var[j].data)) {
				tmp_value = xdebug_get_zval_value_line(&fse->var[j].data, 0, NULL);
				xdebug_str_add_str(&log_buffer, tmp_value);
				xdebug_str_free(tmp_value);
			} else {
				xdebug_str_add_literal(&log_buffer, "*uninitialized*");
			}
		}

		if (variadic_opened) {
			xdebug_str_add_literal(&log_buffer, ")");
		}

		xdebug_str_add_fmt(&log_buffer, ") %s:%d", ZSTR_VAL(fse->filename), fse->lineno);
		php_log_err(log_buffer.d);
		xdebug_str_destroy(&log_buffer);
	}
}

void xdebug_append_error_head(xdebug_str *str, int html, const char *error_type_str)
{
	const char **formats = select_formats(html);

	if (html) {
		xdebug_str_add_fmt(str, formats[0], error_type_str, XG_DEV(in_at) ? " xe-scream" : "");
		if (XG_DEV(in_at)) {
			xdebug_str_add_const(str, formats[12]);
		}
	} else {
		xdebug_str_add_const(str, formats[0]);
		if (XG_DEV(in_at)) {
			xdebug_str_add_const(str, formats[10]);
		}
	}
}

void xdebug_append_error_description(xdebug_str *str, int html, const char *error_type_str, const char *buffer, const char *error_filename, const int error_lineno)
{
	const char **formats = select_formats(html);
	char *escaped;

	if (!html) {
		escaped = estrdup(buffer);
	} else {
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
	}

	if (strlen(XINI_LIB(file_link_format)) > 0 && html && strcmp(error_filename, "Unknown") != 0) {
		char *file_link;

		xdebug_format_file_link(&file_link, error_filename, error_lineno);
		xdebug_str_add_fmt(str, formats[11], error_type_str, escaped, file_link, error_filename, error_lineno);
		xdfree(file_link);
	} else {
		xdebug_str_add_fmt(str, formats[1], error_type_str, escaped, error_filename, error_lineno);
	}

	efree(escaped);
}

static void xdebug_append_error_description_from_object(xdebug_str *str, int html, zval *exception_obj)
{
	zval *message, *file, *line;
	zval dummy;

	if (Z_TYPE_P(exception_obj) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(exception_obj), zend_ce_throwable)) {
		return;
	}

	message = zend_read_property(Z_OBJCE_P(exception_obj), Z_OBJ_P(exception_obj), "message", sizeof("message")-1, 1, &dummy);
	file = zend_read_property(Z_OBJCE_P(exception_obj), Z_OBJ_P(exception_obj), "file", sizeof("file")-1, 1, &dummy);
	line = zend_read_property(Z_OBJCE_P(exception_obj), Z_OBJ_P(exception_obj), "line", sizeof("line")-1, 1, &dummy);

	if (!message || !file || !line || Z_TYPE_P(message) != IS_STRING || Z_TYPE_P(file) != IS_STRING || Z_TYPE_P(line) != IS_LONG) {
		return;
	}

	xdebug_append_error_description(str, html, STR_NAME_VAL(Z_OBJCE_P(exception_obj)->name), Z_STRVAL_P(message), Z_STRVAL_P(file), Z_LVAL_P(line));
}

static void xdebug_append_sub_header(xdebug_str *str, int html)
{
	const char **formats = select_formats(html);
	xdebug_str_add_fmt(str, formats[17], formats[21]);
}

static void add_single_value(xdebug_str *str, zval *zv, int html)
{
	xdebug_str *tmp_value = NULL;
	char       *tmp_html_value = NULL;
	size_t      newlen;

	if (html) {
		tmp_value = xdebug_get_zval_value_line(zv, 0, NULL);
		tmp_html_value = xdebug_xmlize(tmp_value->d, tmp_value->l, &newlen);

		xdebug_str_add_literal(str, "<span>");
		xdebug_str_add(str, tmp_html_value, 0);
		xdebug_str_add_literal(str, "</span>");

		xdebug_str_free(tmp_value);
		efree(tmp_html_value);
	} else {
		tmp_value = xdebug_get_zval_value_line(zv, 0, NULL);

		if (tmp_value) {
			xdebug_str_add_str(str, tmp_value);
			xdebug_str_free(tmp_value);
		} else {
			xdebug_str_add_literal(str, "???");
		}
	}
}

static void zval_from_stack_add_frame_parameters(zval *frame, function_stack_entry *fse, bool params_as_values)
{
	unsigned int  j;
	zval         *params;
	int           variadic_opened = 0;
	int           sent_variables = fse->varc;

	if (sent_variables > 0 && fse->var[sent_variables-1].is_variadic && Z_ISUNDEF(fse->var[sent_variables-1].data)) {
		sent_variables--;
	}

	XDEBUG_MAKE_STD_ZVAL(params);
	array_init(params);
	add_assoc_zval_ex(frame, "params", HASH_KEY_SIZEOF("params"), params);

	for (j = 0; j < sent_variables; j++) {

		if (fse->var[j].is_variadic) {
			zval *vparams;

			XDEBUG_MAKE_STD_ZVAL(vparams);
			array_init(vparams);

			if (fse->var[j].name) {
				add_assoc_zval_ex(params, ZSTR_VAL(fse->var[j].name), ZSTR_LEN(fse->var[j].name), vparams);
			} else {
				add_index_zval(params, j, vparams);
			}
			efree(params);
			params = vparams;
			variadic_opened = 1;
			continue;
		}

		if (params_as_values) {
			/* Named parameters */
			if (fse->var[j].name && !variadic_opened) {
				if (Z_TYPE(fse->var[j].data) == IS_UNDEF) {
					add_assoc_null_ex(params, ZSTR_VAL(fse->var[j].name), ZSTR_LEN(fse->var[j].name));
				} else {
					Z_TRY_ADDREF(fse->var[j].data);
					add_assoc_zval_ex(params, ZSTR_VAL(fse->var[j].name), ZSTR_LEN(fse->var[j].name), &fse->var[j].data);
				}
				continue;
			}

			/* Unnamed or Variadic parameters */
			if (Z_TYPE(fse->var[j].data) == IS_UNDEF) {
				add_index_null(params, j - variadic_opened);
			} else {
				Z_TRY_ADDREF(fse->var[j].data);
				add_index_zval(params, j - variadic_opened, &fse->var[j].data);
			}

			continue;
		} else {
			xdebug_str *argument = NULL;

			if (!Z_ISUNDEF(fse->var[j].data)) {
				argument = xdebug_get_zval_value_line(&fse->var[j].data, 0, NULL);
			} else {
				argument = xdebug_str_create_from_char((char*) "???");
			}
			if (fse->var[j].name && !variadic_opened && argument) {
				add_assoc_stringl_ex(params, ZSTR_VAL(fse->var[j].name), ZSTR_LEN(fse->var[j].name), argument->d, argument->l);
			} else {
				add_index_stringl(params, j - variadic_opened, argument->d, argument->l);
			}
			if (argument) {
				xdebug_str_free(argument);
				argument = NULL;
			}
		}
	}

	efree(params);
}

static void zval_from_stack_add_frame_variables(zval *frame, zend_execute_data *edata, HashTable *symbols, zend_op_array *opa)
{
	unsigned int j;
	zval         variables;

	array_init(&variables);

	add_assoc_zval_ex(frame, "variables", HASH_KEY_SIZEOF("variables"), &variables);

	xdebug_lib_set_active_data(edata);
	xdebug_lib_set_active_symbol_table(symbols);

	for (j = 0; j < (unsigned int) opa->last_var; j++) {
		xdebug_str *symbol_name;
		zval        symbol;

		symbol_name = xdebug_str_create_from_char(opa->vars[j]->val);
		xdebug_get_php_symbol(&symbol, symbol_name);
		xdebug_str_free(symbol_name);

		if (Z_TYPE(symbol) == IS_UNDEF) {
			add_assoc_null_ex(&variables, opa->vars[j]->val, opa->vars[j]->len);
		} else {
			add_assoc_zval_ex(&variables, opa->vars[j]->val, opa->vars[j]->len, &symbol);
		}
	}
}

static void zval_from_stack_add_frame(zval *output, function_stack_entry *fse, zend_execute_data *edata, bool add_local_vars, bool params_as_values)
{
	zval                 *frame;

	/* Initialize frame array */
	XDEBUG_MAKE_STD_ZVAL(frame);
	array_init(frame);

	/* Add data */
	add_assoc_double_ex(frame, "time", HASH_KEY_SIZEOF("time"), XDEBUG_SECONDS_SINCE_START(fse->nanotime));
	add_assoc_long_ex(frame, "memory", HASH_KEY_SIZEOF("memory"), fse->memory);
	if (fse->function.function) {
		add_assoc_str_ex(frame, "function", HASH_KEY_SIZEOF("function"), zend_string_copy(fse->function.function));
	}
	if (fse->function.object_class) {
		add_assoc_string_ex(frame, "type",     HASH_KEY_SIZEOF("type"),     (char*) (fse->function.type == XFUNC_STATIC_MEMBER ? "static" : "dynamic"));
		add_assoc_str_ex(frame,    "class",    HASH_KEY_SIZEOF("class"),    zend_string_copy(fse->function.object_class));
	}
	add_assoc_str_ex(frame, "file", HASH_KEY_SIZEOF("file"), zend_string_copy(fse->filename));
	add_assoc_long_ex(frame, "line", HASH_KEY_SIZEOF("line"), fse->lineno);

	zval_from_stack_add_frame_parameters(frame, fse, params_as_values);

	if (add_local_vars && fse->op_array && fse->op_array->vars) {
		zval_from_stack_add_frame_variables(frame, edata, fse->symbol_table, fse->op_array);
	}

	if (fse->include_filename) {
		add_assoc_str_ex(frame, "include_filename", HASH_KEY_SIZEOF("include_filename"), zend_string_copy(fse->include_filename));
	}

	add_next_index_zval(output, frame);
	efree(frame);
}

static void zval_from_stack(zval *output, bool add_local_vars, bool params_as_values)
{
	function_stack_entry *fse, *next_fse;
	unsigned int          i;

	array_init(output);

	fse = XDEBUG_VECTOR_HEAD(XG_BASE(stack));
	next_fse = fse + 1;

	for (i = 0; i < XDEBUG_VECTOR_COUNT(XG_BASE(stack)) - 1; i++, fse++, next_fse++) {
		zval_from_stack_add_frame(output, fse, next_fse->execute_data, add_local_vars, params_as_values);
	}
}

/* Helpers for last_exception_trace slots */

static zval *last_exception_find_trace(zend_object *obj)
{
	int i;

	for (i = 0; i < XDEBUG_LAST_EXCEPTION_TRACE_SLOTS; i++) {
		if (obj == XG_DEV(last_exception_trace).obj_ptr[i]) {
			return &XG_DEV(last_exception_trace).stack_trace[i];
		}
	}
	return NULL;
}

static zval *last_exception_get_slot(zend_object *obj)
{
	int slot = XG_DEV(last_exception_trace).next_slot;

	if (XG_DEV(last_exception_trace).obj_ptr[slot] != NULL) {
		zval_ptr_dtor(&XG_DEV(last_exception_trace).stack_trace[slot]);
		XG_DEV(last_exception_trace).obj_ptr[slot] = NULL;
	}

	XG_DEV(last_exception_trace).obj_ptr[slot] = obj;

	XG_DEV(last_exception_trace).next_slot = (slot + 1 == XDEBUG_LAST_EXCEPTION_TRACE_SLOTS ? 0 : slot + 1);

	return &XG_DEV(last_exception_trace).stack_trace[slot];
}

/* Formatting variables */

#define XDEBUG_VAR_FORMAT_INITIALISED   0
#define XDEBUG_VAR_FORMAT_UNINITIALISED 1

static const char* text_var_formats[2] = {
	"  $%s = %s\n",
	"  $%s = *uninitialized*\n",
};

static const char* ansi_var_formats[2] = {
	"  $%s = %s\n",
	"  $%s = *uninitialized*\n",
};

static const char* html_var_formats[2] = {
	"<tr><td colspan='2' align='right' bgcolor='#eeeeec' valign='top'><pre>$%s&nbsp;=</pre></td><td colspan='3' bgcolor='#eeeeec'>%s</td></tr>\n",
	"<tr><td colspan='2' align='right' bgcolor='#eeeeec' valign='top'><pre>$%s&nbsp;=</pre></td><td colspan='3' bgcolor='#eeeeec' valign='top'><i>Undefined</i></td></tr>\n",
};

static const char** get_var_format_string(int html)
{
	if (html) {
		return html_var_formats;
	} else if ((XINI_DEV(cli_color) == 1 && xdebug_is_output_tty()) || (XINI_DEV(cli_color) == 2)) {
		return ansi_var_formats;
	} else {
		return text_var_formats;
	}
}


static void xdebug_dump_used_var_with_contents(void *htmlq, xdebug_hash_element* he, void *argument)
{
	int          html = *(int*) htmlq;
	zval         zvar;
	xdebug_str  *contents;
	xdebug_str  *name = (xdebug_str*) he->ptr;
	HashTable   *tmp_ht;
	const char **formats;
	xdebug_str   *str = (xdebug_str *) argument;

	if (!he->ptr) {
		return;
	}

	/* Bail out on $this and $GLOBALS */
	if (strcmp(name->d, "this") == 0 || strcmp(name->d, "GLOBALS") == 0) {
		return;
	}

	if (EG(current_execute_data) && !(ZEND_CALL_INFO(EG(current_execute_data)) & ZEND_CALL_HAS_SYMBOL_TABLE)) {
		zend_rebuild_symbol_table();
	}

	tmp_ht = xdebug_lib_get_active_symbol_table();
	{
		zend_execute_data *ex = EG(current_execute_data);
		while (ex && (!ex->func || !ZEND_USER_CODE(ex->func->type))) {
			ex = ex->prev_execute_data;
		}
		if (ex) {
			xdebug_lib_set_active_data(ex);
			xdebug_lib_set_active_symbol_table(ex->symbol_table);
		}
	}

	xdebug_get_php_symbol(&zvar, name);
	xdebug_lib_set_active_symbol_table(tmp_ht);

	formats = get_var_format_string(PG(html_errors));

	if (Z_TYPE(zvar) == IS_UNDEF) {
		xdebug_str_add_fmt(str, formats[XDEBUG_VAR_FORMAT_UNINITIALISED], name->d);
		return;
	}

	if (html) {
		contents = xdebug_get_zval_value_html(NULL, &zvar, 0, NULL);
	} else {
		contents = xdebug_get_zval_value_line(&zvar, 0, NULL);
	}

	if (contents) {
		xdebug_str_add_fmt(str, formats[XDEBUG_VAR_FORMAT_INITIALISED], name->d, contents->d);
	} else {
		xdebug_str_add_fmt(str, formats[XDEBUG_VAR_FORMAT_UNINITIALISED], name->d);
	}

	if (contents) {
		xdebug_str_free(contents);
	}
	zval_ptr_dtor_nogc(&zvar);
}

void xdebug_append_printable_stack_from_zval(xdebug_str *str, bool indent, zval *trace, int html)
{
	const char **formats = select_formats(html);
	zval        *frame;
	int          counter = 0;

	xdebug_str_add_fmt(str, formats[13], indent ? formats[21] : ""); // header

	if (!trace || Z_TYPE_P(trace) != IS_ARRAY) {
		xdebug_str_add_fmt(str, formats[15], indent ? formats[21] : ""); // message
		xdebug_str_add_const(str, formats[14]); // footer
		return;
	}

	ZEND_HASH_FOREACH_VAL_IND(HASH_OF(trace), frame) {
		zval *time, *memory, *class, *type, *function, *file, *line;
		char *combined_function;

		counter++;

		if (Z_TYPE_P(frame) != IS_ARRAY) {
			continue;
		}

		time = zend_hash_str_find(HASH_OF(frame), "time", 4);
		memory = zend_hash_str_find(HASH_OF(frame), "memory", 6);
		class = zend_hash_str_find(HASH_OF(frame), "class", 5);
		type = zend_hash_str_find(HASH_OF(frame), "type", 4);
		function = zend_hash_str_find(HASH_OF(frame), "function", 8);
		file = zend_hash_str_find(HASH_OF(frame), "file", 4);
		line = zend_hash_str_find(HASH_OF(frame), "line", 4);

		if (!time || !memory || !function || !file || !line) {
			continue;
		}

		if (Z_TYPE_P(time) != IS_DOUBLE || Z_TYPE_P(memory) != IS_LONG || Z_TYPE_P(function) != IS_STRING || Z_TYPE_P(file) != IS_STRING || Z_TYPE_P(line) != IS_LONG) {
			continue;
		}

		if (class && type && Z_TYPE_P(class) == IS_STRING && Z_TYPE_P(type) == IS_STRING) {
			combined_function = xdebug_sprintf("%s%s%s", Z_STRVAL_P(class), strcmp(Z_STRVAL_P(type), "static") == 0 ? "::" : "->", Z_STRVAL_P(function));
		} else {
			combined_function = xdstrdup(Z_STRVAL_P(function));
		}

		if (html) {
			char *formatted_filename;
			xdebug_format_filename(&formatted_filename, "...%s%n", Z_STR_P(file));

			if (strlen(XINI_LIB(file_link_format)) > 0 && strcmp(Z_STRVAL_P(file), "Unknown") != 0) {
				char *file_link;

				xdebug_format_file_link(&file_link, Z_STRVAL_P(file), Z_LVAL_P(line));
				xdebug_str_add_fmt(str, formats[16], formats[21], counter, Z_DVAL_P(time), Z_LVAL_P(memory), combined_function, Z_STRVAL_P(file), file_link, formatted_filename, Z_LVAL_P(line));
				xdfree(file_link);
			} else {
				xdebug_str_add_fmt(str, formats[20], counter, Z_DVAL_P(time), Z_LVAL_P(memory), combined_function, Z_STRVAL_P(file), formatted_filename, Z_LVAL_P(line));
			}

			xdfree(formatted_filename);
		} else {
			xdebug_str_add_fmt(str, formats[16], indent ? formats[21] : "", Z_DVAL_P(time), Z_LVAL_P(memory), counter, combined_function, Z_STRVAL_P(file), Z_LVAL_P(line));
		}
		xdfree(combined_function);

	} ZEND_HASH_FOREACH_END();

	xdebug_str_add_const(str, formats[14]); // footer
}

static void xdebug_append_nested_section_header(xdebug_str *str, bool indent, int html)
{
	const char **formats = select_formats(html);

	xdebug_str_add_fmt(str, formats[18], indent ? formats[21] : "");
}

static void xdebug_append_nested_section_footer(xdebug_str *str, int html)
{
	const char **formats = select_formats(html);

	xdebug_str_add_const(str, formats[19]);
}

void xdebug_append_printable_stack(xdebug_str *str, int html)
{
	int                   printed_frames = 0;
	const char          **formats = select_formats(html);
	int                   i;
	function_stack_entry *fse;

	if (!XG_BASE(stack) || XDEBUG_VECTOR_COUNT(XG_BASE(stack)) < 1) {
		return;
	}

	fse = XDEBUG_VECTOR_HEAD(XG_BASE(stack));

	xdebug_str_add_const(str, formats[2]);

	for (i = 0; i < XDEBUG_VECTOR_COUNT(XG_BASE(stack)); i++, fse++)
	{
		int c = 0; /* Comma flag */
		unsigned int j = 0; /* Counter */
		char *tmp_name;
		int variadic_opened = 0;
		int sent_variables = fse->varc;

		if (sent_variables > 0 && fse->var[sent_variables-1].is_variadic && Z_ISUNDEF(fse->var[sent_variables-1].data)) {
			sent_variables--;
		}

		if (xdebug_is_stack_frame_filtered(XDEBUG_FILTER_STACK, fse)) {
			continue;
		}
		tmp_name = xdebug_show_fname(fse->function, html ? XDEBUG_SHOW_FNAME_ALLOW_HTML : XDEBUG_SHOW_FNAME_DEFAULT);
		if (html) {
			xdebug_str_add_fmt(str, formats[3], fse->level, XDEBUG_SECONDS_SINCE_START(fse->nanotime), fse->memory, tmp_name);
		} else {
			xdebug_str_add_fmt(str, formats[3], XDEBUG_SECONDS_SINCE_START(fse->nanotime), fse->memory, fse->level, tmp_name);
		}
		xdfree(tmp_name);

		/* Printing vars */
		for (j = 0; j < sent_variables; j++) {
			if (c) {
				xdebug_str_add_literal(str, ", ");
			} else {
				c = 1;
			}

			if (
				(fse->var[j].is_variadic && Z_ISUNDEF(fse->var[j].data))
			) {
				xdebug_str_add_literal(str, "...");
			}

			if (fse->var[j].name) {
				if (html) {
					xdebug_str_add_literal(str, "<span>$");
					xdebug_str_add_zstr(str, fse->var[j].name);
					xdebug_str_add_literal(str, " = </span>");
				} else {
					xdebug_str_add_literal(str, "$");
					xdebug_str_add_zstr(str, fse->var[j].name);
					xdebug_str_add_literal(str, " = ");
				}
			}

			if (!variadic_opened && fse->var[j].is_variadic && Z_ISUNDEF(fse->var[j].data)) {
				if (html) {
					xdebug_str_add_literal(str, "<i>variadic</i>(");
				} else {
					xdebug_str_add_literal(str, "variadic(");
				}
				c = 0;
				variadic_opened = 1;
				continue;
			}

			if (!Z_ISUNDEF(fse->var[j].data)) {
				add_single_value(str, &fse->var[j].data, html);
			} else {
				xdebug_str_add_literal(str, "???");
			}
		}

		if (variadic_opened) {
			xdebug_str_add_literal(str, ")");
		}

		if (fse->include_filename) {
			if (html) {
				xdebug_str_add_literal(str, "<font color='#00bb00'>'");
				xdebug_str_add_zstr(str, fse->include_filename);
				xdebug_str_add_literal(str, "</font>");
			} else {
				xdebug_str_addc(str, '\'');
				xdebug_str_add_zstr(str, fse->include_filename);
				xdebug_str_addc(str, '\'');
			}
		}

		if (html) {
			char *formatted_filename;
			xdebug_format_filename(&formatted_filename, "...%s%n", fse->filename);

			if (strlen(XINI_LIB(file_link_format)) > 0 && strcmp(ZSTR_VAL(fse->filename), "Unknown") != 0) {
				char *file_link;

				xdebug_format_file_link(&file_link, ZSTR_VAL(fse->filename), fse->lineno);
				xdebug_str_add_fmt(str, formats[10], ZSTR_VAL(fse->filename), file_link, formatted_filename, fse->lineno);
				xdfree(file_link);
			} else {
				xdebug_str_add_fmt(str, formats[5], ZSTR_VAL(fse->filename), formatted_filename, fse->lineno);
			}

			xdfree(formatted_filename);
		} else {
			xdebug_str_add_fmt(str, formats[5], ZSTR_VAL(fse->filename), fse->lineno);
		}

		printed_frames++;
		if (XINI_DEV(max_stack_frames) > 0 && printed_frames >= XINI_DEV(max_stack_frames)) {
			break;
		}
	}

	if (XINI_DEV(dump_globals) && !(XINI_DEV(dump_once) && XG_LIB(dumped))) {
		char *tmp = xdebug_get_printable_superglobals(html);

		if (tmp) {
			xdebug_str_add(str, tmp, 1);
		}
		XG_LIB(dumped) = 1;
	}

	if (XINI_DEV(show_local_vars) && XG_BASE(stack) && XDEBUG_VECTOR_TAIL(XG_BASE(stack))) {
		int scope_nr = XDEBUG_VECTOR_COUNT(XG_BASE(stack));

		fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));
		if (fse->user_defined == XDEBUG_BUILT_IN && xdebug_vector_element_is_valid(XG_BASE(stack), fse -1)) {
			fse = fse - 1;
			scope_nr--;
		}

		xdebug_lib_register_compiled_variables(fse);

		if (fse->declared_vars && fse->declared_vars->size) {
			xdebug_hash *tmp_hash;

			xdebug_str_add_fmt(str, formats[6], scope_nr);
			tmp_hash = xdebug_declared_var_hash_from_llist(fse->declared_vars);
			xdebug_hash_apply_with_argument(tmp_hash, (void*) &html, xdebug_dump_used_var_with_contents, (void *) str);
			xdebug_hash_destroy(tmp_hash);
		}
	}
}

void xdebug_append_error_footer(xdebug_str *str, int html)
{
	const char **formats = select_formats(html);

	xdebug_str_add_const(str, formats[7]);
}

char *xdebug_get_printable_stack(int html, int error_type, const char *buffer, const char *error_filename, const int error_lineno, int include_decription)
{
	char *prepend_string;
	char *append_string;
	char *error_type_str = xdebug_error_type(error_type);
	char *error_type_str_simple = xdebug_error_type_simple(error_type);
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	prepend_string = INI_STR((char*) "error_prepend_string");
	append_string = INI_STR((char*) "error_append_string");

	if (prepend_string) {
		xdebug_str_add(&str, prepend_string, 0);
	}
	xdebug_append_error_head(&str, html, error_type_str_simple);
	if (include_decription) {
		xdebug_append_error_description(&str, html, error_type_str, buffer, error_filename, error_lineno);
	}
	xdebug_append_printable_stack(&str, html);
	xdebug_append_error_footer(&str, html);
	if (append_string) {
		xdebug_str_add(&str, append_string, 0);
	}

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

	if (strncmp(buffer, "Uncaught ", 9) != 0) {
		return NULL;
	}

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
		printable_stack = xdebug_get_printable_stack(PG(html_errors), type, buffer, error_filename, error_lineno, 1);
	}

	return printable_stack;
}

static void clear_last_error()
{
	if (PG(last_error_message)) {
		zend_string_release(PG(last_error_message));
		PG(last_error_message) = NULL;
	}
	if (PG(last_error_file)) {
# if PHP_VERSION_ID >= 80100
		zend_string_release(PG(last_error_file));
# else
		free(PG(last_error_file));
# endif
		PG(last_error_file) = NULL;
	}
}

/* Error callback for formatting stack traces */
#if PHP_VERSION_ID >= 80100
void xdebug_develop_error_cb(int orig_type, zend_string *error_filename, const unsigned int error_lineno, zend_string *message)
{
#else
void xdebug_develop_error_cb(int orig_type, const char *error_filename, const unsigned int error_lineno, zend_string *message)
{
#endif
	char *error_type_str;
	int display;
	int type = orig_type & E_ALL;
	error_handling_t  error_handling;
	zend_class_entry *exception_class;


	error_type_str = xdebug_error_type(type);

	/* check for repeated errors to be ignored */
	if (PG(ignore_repeated_errors) && PG(last_error_message)) {
			/* no check for PG(last_error_file) is needed since it cannot
			 * be NULL if PG(last_error_message) is not NULL */

			if (!zend_string_equals(PG(last_error_message), message) ||
				(!PG(ignore_repeated_source) && (
					(PG(last_error_lineno) != (int)error_lineno) ||
#if PHP_VERSION_ID >= 80100
					!zend_string_equals(PG(last_error_file), error_filename)
#else
					strcmp(PG(last_error_file), error_filename) != 0
#endif
				))
			) {
					display = 1;
			} else {
					display = 0;
			}
	} else {
			display = 1;
	}

	error_handling  = EG(error_handling);
	exception_class = EG(exception_class);

	/* according to error handling mode, throw exception or show it */
	if (error_handling == EH_THROW) {
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
				if (!EG(exception)) {
					zend_throw_error_exception(exception_class, message, 0, type);
				}
				xdfree(error_type_str);
				return;
		}
	}

	/* Store last error message for error_get_last() */
	if (display) {
		clear_last_error();
		if (!error_filename) {
#if PHP_VERSION_ID >= 80100
			error_filename = zend_string_init(ZEND_STRL("Unknown"), 0);
#else
			error_filename = "Unknown";
#endif
		}
		PG(last_error_type) = type;
		PG(last_error_message) = zend_string_copy(message);
#if PHP_VERSION_ID >= 80100
		PG(last_error_file) = zend_string_copy(error_filename);
#else
		PG(last_error_file) = strdup(error_filename);
#endif
		PG(last_error_lineno) = error_lineno;
	}

	if ((EG(error_reporting) | XINI_DEV(force_error_reporting)) & type) {
		/* Log to logger */
		if (PG(log_errors)) {

#ifdef PHP_WIN32
			if (type==E_CORE_ERROR || type==E_CORE_WARNING) {
				php_syslog(LOG_ALERT, "PHP %s: %s (%s)", error_type_str, ZSTR_VAL(message), GetCommandLine());
			}
#endif
#if PHP_VERSION_ID >= 80100
			xdebug_log_stack(error_type_str, ZSTR_VAL(message), ZSTR_VAL(error_filename), error_lineno);
#else
			xdebug_log_stack(error_type_str, ZSTR_VAL(message), error_filename, error_lineno);
#endif
			if (XINI_DEV(dump_globals) && !(XINI_DEV(dump_once) && XG_LIB(dumped))) {
				char *printable_stack = xdebug_get_printable_superglobals(0);

				if (printable_stack) {
					int pc;

					xdebug_arg *parts = xdebug_arg_ctor();

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
		if ((PG(display_errors) || XINI_DEV(force_display_errors)) && !PG(during_request_startup)) {
			char *printable_stack;

#if PHP_VERSION_ID >= 80100
			printable_stack = xdebug_handle_stack_trace(type, error_type_str, ZSTR_VAL(error_filename), error_lineno, ZSTR_VAL(message));
#else
			printable_stack = xdebug_handle_stack_trace(type, error_type_str, error_filename, error_lineno, ZSTR_VAL(message));
#endif

			if (XG_LIB(do_collect_errors) && (type != E_ERROR) && (type != E_COMPILE_ERROR) && (type != E_USER_ERROR)) {
				xdebug_llist_insert_next(XG_DEV(collected_errors), XDEBUG_LLIST_TAIL(XG_DEV(collected_errors)), printable_stack);
			} else {
				php_output_error(printable_stack);
				xdfree(printable_stack);
			}
		} else if (XG_LIB(do_collect_errors)) {
			char *printable_stack;
#if PHP_VERSION_ID >= 80100
			printable_stack = xdebug_get_printable_stack(PG(html_errors), type, ZSTR_VAL(message), ZSTR_VAL(error_filename), error_lineno, 1);
#else
			printable_stack = xdebug_get_printable_stack(PG(html_errors), type, ZSTR_VAL(message), error_filename, error_lineno, 1);
#endif
			xdebug_llist_insert_next(XG_DEV(collected_errors), XDEBUG_LLIST_TAIL(XG_DEV(collected_errors)), printable_stack);
		}
	}

	{
#if PHP_VERSION_ID >= 80100
		zend_string *tmp_error_filename = zend_string_copy(error_filename);
#else
		zend_string *tmp_error_filename = zend_string_init(error_filename, strlen(error_filename), 0);
#endif
		xdebug_debugger_error_cb(tmp_error_filename, error_lineno, type, error_type_str, ZSTR_VAL(message));
		zend_string_release(tmp_error_filename);
	}

	xdfree(error_type_str);

	if (type & XINI_DEV(halt_level) & XDEBUG_ALLOWED_HALT_LEVELS) {
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
				if (!(orig_type & E_DONT_BAIL)) {
					/* restore memory limit */
					zend_set_memory_limit(PG(memory_limit));
					zend_objects_store_mark_destructed(&EG(objects_store));
					_zend_bailout((char*) __FILE__, __LINE__);
					return;
				}
			}
			break;
	}
}

void xdebug_develop_throw_exception_hook(zend_object *exception, zval *file, zval *line, zval *code, char *code_str, zval *message)
{
	zend_class_entry *exception_ce = exception->ce;
	char *exception_trace;
	xdebug_str tmp_str = XDEBUG_STR_INITIALIZER;

	zval *z_previous_exception, *z_last_exception_slot, *z_previous_trace;
	zend_object *previous_exception_obj = exception;
	zval dummy;

	if (!PG(html_errors)) {
		xdebug_str_addc(&tmp_str, '\n');
	}
	xdebug_append_error_description(&tmp_str, PG(html_errors), STR_NAME_VAL(exception_ce->name), message ? Z_STRVAL_P(message) : "", Z_STRVAL_P(file), Z_LVAL_P(line));

	z_previous_trace = last_exception_find_trace(exception);
	if (z_previous_trace) {
		xdebug_append_printable_stack_from_zval(&tmp_str, false, z_previous_trace, PG(html_errors));
	} else {
		xdebug_append_printable_stack(&tmp_str, PG(html_errors));
	}

	/* Loop over previous exceptions until there are none left */
	{
		bool first = true;
		bool found = false;

		do {
			z_previous_exception = zend_read_property(exception_ce, previous_exception_obj, "previous", sizeof("previous")-1, 1, &dummy);
			if (!z_previous_exception || Z_TYPE_P(z_previous_exception) != IS_OBJECT) {
				break;
			}

			if (first) {
				first = false;
				found = true;
				xdebug_append_nested_section_header(&tmp_str, true, PG(html_errors));
			}

			xdebug_append_sub_header(&tmp_str, PG(html_errors));
			xdebug_append_error_description_from_object(&tmp_str, PG(html_errors), z_previous_exception);

			z_previous_trace = last_exception_find_trace(Z_OBJ_P(z_previous_exception));
			xdebug_append_printable_stack_from_zval(&tmp_str, true, z_previous_trace, PG(html_errors));

			previous_exception_obj = Z_OBJ_P(z_previous_exception);
		} while (true);

		if (found) {
			xdebug_append_nested_section_footer(&tmp_str, PG(html_errors));
		}
	}

	/* Remember last stack trace so it can be retrieved in an exception handler through
	 * xdebug_get_function_stack(['from_exception' => $e]) */
	z_last_exception_slot = last_exception_get_slot(exception);
	zval_from_stack(z_last_exception_slot, true, true);
	zval_from_stack_add_frame(z_last_exception_slot, XDEBUG_VECTOR_TAIL(XG_BASE(stack)), EG(current_execute_data), true, true);

	exception_trace = tmp_str.d;

	/* Save */
	if (XG_BASE(last_exception_trace)) {
		xdfree(XG_BASE(last_exception_trace));
	}
	XG_BASE(last_exception_trace) = exception_trace;

	/* Display if expected */
	if (XINI_DEV(show_ex_trace) || (instanceof_function(exception_ce, zend_ce_error) && XINI_DEV(show_error_trace))) {
		if (PG(log_errors)) {
			xdebug_log_stack(STR_NAME_VAL(exception_ce->name), Z_STRVAL_P(message), Z_STRVAL_P(file), Z_LVAL_P(line));
		}
		if (PG(display_errors)) {
			xdebug_str displ_tmp_str = XDEBUG_STR_INITIALIZER;
			xdebug_append_error_head(&displ_tmp_str, PG(html_errors), "exception");
			xdebug_str_add(&displ_tmp_str, exception_trace, 0);
			xdebug_append_error_footer(&displ_tmp_str, PG(html_errors));

			php_printf("%s", displ_tmp_str.d);
			xdebug_str_dtor(displ_tmp_str);
		}
	}
}

/* {{{ proto int xdebug_get_stack_depth()
   Returns the stack depth */
PHP_FUNCTION(xdebug_get_stack_depth)
{
	if (!XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		php_error(E_WARNING, "Function must be enabled in php.ini by setting 'xdebug.mode' to 'develop'");
		RETURN_LONG(0);
	}

	/* We substract one so that the function call to xdebug_get_stack_depth()
	 * is not part of the returned depth. */
	RETURN_LONG(XDEBUG_VECTOR_COUNT(XG_BASE(stack)) - 1);
}

/* {{{ proto array xdebug_get_function_stack()
   Returns an array representing the current stack */
PHP_FUNCTION(xdebug_get_function_stack)
{
	HashTable            *options = NULL;
	bool                  add_local_vars = false;
	bool                  params_as_values = false;

	if (!XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		php_error(E_WARNING, "Function must be enabled in php.ini by setting 'xdebug.mode' to 'develop'");
		array_init(return_value);
		return;
	}

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ARRAY_HT_OR_NULL(options)
	ZEND_PARSE_PARAMETERS_END();

	if (options) {
		zval *value;

		value = zend_hash_str_find(options, "from_exception", sizeof("from_exception") - 1);
		if (value && Z_TYPE_P(value) == IS_OBJECT && instanceof_function(Z_OBJCE_P(value), zend_ce_throwable)) {
			if (Z_OBJ_P(value) == XG_DEV(last_exception_trace).obj_ptr[0]) {
				Z_TRY_ADDREF(XG_DEV(last_exception_trace).stack_trace[0]);
				ZVAL_COPY_VALUE(return_value, &XG_DEV(last_exception_trace).stack_trace[0]);
			} else {
				array_init(return_value);
			}

			if (
				(zend_hash_str_find(options, "local_vars", sizeof("local_vars") - 1)) ||
				(zend_hash_str_find(options, "params_as_values", sizeof("params_as_values") - 1))
			) {
				php_error(E_WARNING, "The 'local_vars' or 'params_as_values' options are ignored when used with the 'from_exception' option");
			}

			return;
		}

		value = zend_hash_str_find(options, "local_vars", sizeof("local_vars") - 1);
		if (value) {
			add_local_vars = (Z_TYPE_P(value) == IS_TRUE);
		}

		value = zend_hash_str_find(options, "params_as_values", sizeof("params_as_values") - 1);
		if (value) {
			params_as_values = (Z_TYPE_P(value) == IS_TRUE);
		}
	}

	zval_from_stack(return_value, add_local_vars, params_as_values);

}
/* }}} */
