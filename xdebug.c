/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000, 2001 The PHP Group             |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@vl-srm.net>                         |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

#if HAVE_XDEBUG

#define XDEBUG_VERSION "0.8.0-dev"

#include "TSRM.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_globals.h"
#include "php_xdebug.h"

#include "zend.h"
#include "zend_API.h"
#include "zend_execute.h"
#include "zend_compile.h"
#include "zend_extensions.h"

#include "xdebug_llist.h"

static int le_xdebug;

zend_op_array* (*old_compile_file)(zend_file_handle* file_handle, int type TSRMLS_DC);
zend_op_array* xdebug_compile_file(zend_file_handle*, int TSRMLS_DC);

void (*old_execute)(zend_op_array *op_array TSRMLS_DC);
void xdebug_execute(zend_op_array *op_array TSRMLS_DC);

void (*old_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);
void (*new_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);
void xdebug_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);

function_entry xdebug_functions[] = {
	PHP_FE(xdebug_get_function_stack, NULL)
	PHP_FE(xdebug_call_function,      NULL)
	PHP_FE(xdebug_call_file,          NULL)
	PHP_FE(xdebug_call_line,          NULL)

	PHP_FE(xdebug_enable,             NULL)
	PHP_FE(xdebug_disable,            NULL)
	PHP_FE(xdebug_is_enabled,         NULL)

	PHP_FE(xdebug_start_trace,        NULL)
	PHP_FE(xdebug_stop_trace,         NULL)
	PHP_FE(xdebug_get_function_trace, NULL)
#if MEMORY_LIMIT
	PHP_FE(xdebug_memory_usage,       NULL)
#endif
	{NULL, NULL, NULL}
};

zend_module_entry xdebug_module_entry = {
	STANDARD_MODULE_HEADER,
	"xdebug",
	xdebug_functions,
	PHP_MINIT(xdebug),
	PHP_MSHUTDOWN(xdebug),
	PHP_RINIT(xdebug),
	PHP_RSHUTDOWN(xdebug),
	PHP_MINFO(xdebug),
	"0.1",
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

ZEND_DECLARE_MODULE_GLOBALS(xdebug)

#if COMPILE_DL_XDEBUG
ZEND_GET_MODULE(xdebug)
#endif

PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("xdebug.max_nesting_level", "64", PHP_INI_SYSTEM, OnUpdateInt,  max_nesting_level, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.default_enable",  "1",  PHP_INI_SYSTEM, OnUpdateBool, default_enable,    zend_xdebug_globals, xdebug_globals)
PHP_INI_END()

static char *xdebug_sprintf (const char* fmt, ...)
{
	char   *new_str;
	int     size = 1;
	va_list args;

	new_str = (char *) emalloc (size);

	va_start(args, fmt);
	for (;;) {
		int n = vsnprintf (new_str, size, fmt, args);
		if (n > -1 && n < size) {
			break;
		}
		if (n < 0) {
			size *= 2;
		} else {
			size = n + 1;
		}
		new_str = (char *) erealloc (new_str, size);
	}
	va_end (args);

	return new_str;
}


static void php_xdebug_init_globals (zend_xdebug_globals *xg)
{
	xg->stack    = NULL;
	xg->level    = 0;
	xg->do_trace = 0;
}

PHP_MINIT_FUNCTION(xdebug)
{
	ZEND_INIT_MODULE_GLOBALS(xdebug, php_xdebug_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	old_compile_file = zend_compile_file;
	zend_compile_file = xdebug_compile_file;

	old_execute = zend_execute;
	zend_execute = xdebug_execute;

	old_error_cb = zend_error_cb;
	new_error_cb = xdebug_error_cb;

	return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(xdebug)
{
	zend_compile_file = old_compile_file;
	zend_execute = old_execute;
	zend_error_cb = old_error_cb;

	return SUCCESS;
}


void stack_element_dtor (void *dummy, void *elem)
{
	int                   i;
	function_stack_entry *e = elem;

	e->refcount--;

	if (e->refcount == 0) {
		if (e->function_name) {
			efree (e->function_name);
		}
		if (e->filename) {
			efree (e->filename);
		}
		for (i = 0; i < e->varc; i++) {
			efree (e->vars[i]);
		}
		efree (e);
	}
}



PHP_RINIT_FUNCTION(xdebug)
{
	CG(extended_info) = 1;
	XG(level)    = 0;
	XG(do_trace) = 0;
	XG(stack)    = xdebug_llist_alloc (stack_element_dtor);

	if (XG(default_enable)) {
		zend_error_cb = new_error_cb;
	}
	return SUCCESS;
}



PHP_RSHUTDOWN_FUNCTION(xdebug)
{
	xdebug_llist_destroy (XG(stack), NULL);
	XG(stack) = NULL;

	if (XG(do_trace)) {
		xdebug_llist_destroy (XG(trace), NULL);
		XG(trace) = NULL;
	}

	XG(level)    = 0;
	XG(do_trace) = 0;
	return SUCCESS;
}


PHP_MINFO_FUNCTION(xdebug)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "xdebug support", "enabled");
	php_info_print_table_row(2, "Version", XDEBUG_VERSION);
	php_info_print_table_row(2, "Stacktraces support", "enabled");
	php_info_print_table_row(2, "Function nesting protection support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

void xdebug_execute(zend_op_array *op_array TSRMLS_DC)
{
	struct function_stack_entry* tmp;

	if (op_array->function_name == NULL) {
		tmp = emalloc (sizeof (struct function_stack_entry));
		tmp->varc     = 0;
		tmp->refcount = 1;
		tmp->level    = XG(level) + 1;
		tmp->function_name = estrdup("{main}");

		tmp->filename  = op_array->filename ? estrdup(op_array->filename): NULL;
		tmp->lineno    = 0;

		xdebug_llist_insert_next (XG(stack), SRM_LLIST_TAIL(XG(stack)), tmp);
		if (XG(do_trace)) {
			tmp->refcount++;
			xdebug_llist_insert_next (XG(trace), SRM_LLIST_TAIL(XG(trace)), tmp);
		}

		old_execute (op_array TSRMLS_CC);

		xdebug_llist_remove (XG(stack), SRM_LLIST_TAIL(XG(stack)), stack_element_dtor);
		XG(level)--;
	} else {
		old_execute (op_array TSRMLS_CC);
	}
}

static inline void print_stack (int html, const char *error_type_str, char *buffer, const char *error_filename, const int error_lineno TSRMLS_DC)
{
	char *error_format;
	xdebug_llist_element *le;

	if (html) {
		php_printf ("<br />\n<table border='1' cellspacing='0'>\n");
	} else {
		printf ("\nStack trace:\n");
	}

	error_format = html ?
		"<tr><td bgcolor='#ffbbbb' colspan=\"3\"><b>%s</b>: %s in <b>%s</b> on line <b>%d</b><br />\n"
		: "\n%s: %s in %s on line %d\n";
	php_printf(error_format, error_type_str, buffer, error_filename, error_lineno);

	if (html) {
		php_printf ("<tr><th bgcolor='#aaaaaa' colspan='3'>Stacktrace</th></tr>\n");
		php_printf ("<tr><th bgcolor='#cccccc'>#</th><th bgcolor='#cccccc'>Function</th><th bgcolor='#cccccc'>Location</th></tr>\n");
	}


	for (le = SRM_LLIST_HEAD(XG(stack)); le != NULL; le = SRM_LLIST_NEXT(le))
	{
		int c = 0; /* Comma flag */
		int j = 0; /* Counter */
		struct function_stack_entry *i = SRM_LLIST_VALP(le);

		if (html) {
			php_printf ("<tr><td align='center'>%d</td><td>%s(", i->level, i->function_name);
		} else {
			printf ("%3d. %s(", i->level, i->function_name);
		}

		/* Printing vars */
		for (j = 0; j < i->varc; j++) {
			if (c) {
				if (html) {
					php_printf (", ");
				} else {
					printf (", ");
				}
			} else {
				c = 1;
			}
			if (html) {
				php_printf ("%s", i->vars[j]);
			} else {
				printf ("%s", i->vars[j]);
			}
		}

		if (html) {
			php_printf (")</td><td>%s<b>:</b>%d</td></tr>\n", i->filename, i->lineno);
		} else {
			printf (") %s:%d\n", i->filename, i->lineno);
		}
	}

	if (html) {
		php_printf ("</table>\n");
	}

}

static inline void print_trace (int html TSRMLS_DC)
{
	xdebug_llist_element *le;

	if (html) {
		php_printf ("<br />\n<table border='1' cellspacing='0'>\n");
	} else {
		printf ("\nFunction trace:\n");
	}

	if (html) {
		php_printf ("<tr><th bgcolor='#aaaaaa' colspan='3'>Function trace</th></tr>\n");
		php_printf ("<tr><th bgcolor='#cccccc'>#</th><th bgcolor='#cccccc'>Function</th><th bgcolor='#cccccc'>Location</th></tr>\n");
	}


	for (le = SRM_LLIST_HEAD(XG(trace)); le != NULL; le = SRM_LLIST_NEXT(le))
	{
		int c = 0; /* Comma flag */
		int j = 0; /* Counter */
		struct function_stack_entry *i = SRM_LLIST_VALP(le);

		if (html) {
			php_printf ("<tr><td align='left'><pre>");
			for (j = 0; j < i->level - 1; j++) {
				php_printf ("  ");
			}
			php_printf ("-></pre></td><td>%s(", i->function_name);
		} else {
			for (j = 0; j < i->level; j++) {
				printf ("  ");
			}
			printf ("-> %s(",i->function_name);
		}

		/* Printing vars */
		for (j = 0; j < i->varc; j++) {
			if (c) {
				if (html) {
					php_printf (", ");
				} else {
					printf (", ");
				}
			} else {
				c = 1;
			}
			if (html) {
				php_printf ("%s", i->vars[j]);
			} else {
				printf ("%s", i->vars[j]);
			}
		}

		if (html) {
			php_printf (")</td><td>%s<b>:</b>%d</td></tr>\n", i->filename, i->lineno);
		} else {
			printf (") %s:%d\n", i->filename, i->lineno);
		}
	}

	if (html) {
		php_printf ("</table>\n");
	}

}

void xdebug_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args)
{
	char *error_type_str;
	int buffer_len;
	char buffer[1024];

	TSRMLS_FETCH();

	buffer_len = vsnprintf(buffer, sizeof(buffer)-1, format, args);
	buffer[sizeof(buffer)-1]=0;
	if (buffer_len > sizeof(buffer) - 1 || buffer_len < 0) {
		buffer_len = sizeof(buffer) - 1;
	}

	switch (type) {
		case E_ERROR:
		case E_CORE_ERROR:
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			error_type_str = "Fatal error";
			break;
		case E_WARNING:
		case E_CORE_WARNING:
		case E_COMPILE_WARNING:
		case E_USER_WARNING:
			error_type_str = "Warning";
			break;
		case E_PARSE:
			error_type_str = "Parse error";
			break;
		case E_NOTICE:
		case E_USER_NOTICE:
			error_type_str = "Notice";
			break;
		default:
			error_type_str = "Unknown error";
			break;
	}

	print_stack (PG(html_errors), error_type_str, buffer, error_filename, error_lineno TSRMLS_CC);

	/* Bail out if we can't recover */
	switch (type) {
		case E_CORE_ERROR:
		/* no break - intentionally */
		case E_ERROR:
		/*case E_PARSE: the parser would return 1 (failure), we can bail out nicely */
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			zend_bailout();
			break;
	}
}

/* {{{ zend_op_array srm_compile_file (file_handle, type)
 *    This function provides a hook for the execution of bananas */
zend_op_array *xdebug_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC)
{
	zend_op_array *op_array;

	op_array = old_compile_file (file_handle, type TSRMLS_CC);

	return op_array;
}
/* }}} */

PHP_FUNCTION(xdebug_get_function_stack)
{
	char buffer[1024];
	xdebug_llist_element *le;
	unsigned int          k;

	array_init(return_value);
	le = SRM_LLIST_HEAD(XG(stack));
	
	for (k = 0; k < XG(stack)->size - 1; k++, le = SRM_LLIST_NEXT(le)) {
		struct function_stack_entry *i = SRM_LLIST_VALP(le);

		snprintf (buffer, 1024, "%s:%d: %s", i->filename, i->lineno, i->function_name);
		add_next_index_string(return_value, (char*) &buffer, 1);
	}
}

PHP_FUNCTION(xdebug_call_function)
{
	xdebug_llist_element           *le;
	struct function_stack_entry *i;
	
	le = SRM_LLIST_TAIL(XG(stack));
	if (le) {
		if (le->prev) {
			le = SRM_LLIST_PREV(le);
			if (le->prev) {
				le = SRM_LLIST_PREV(le);
			}
		}
		i = SRM_LLIST_VALP(le);

		RETURN_STRING(i->function_name ? i->function_name : "{}", 1);
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_call_line)
{
	xdebug_llist_element           *le;
	struct function_stack_entry *i;
	
	le = SRM_LLIST_TAIL(XG(stack));
	if (le) {
		if (le->prev) {
			le = SRM_LLIST_PREV(le);
		}
		i = SRM_LLIST_VALP(le);

		RETURN_LONG(i->lineno);
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_call_file)
{
	xdebug_llist_element           *le;
	struct function_stack_entry *i;
	
	le = SRM_LLIST_TAIL(XG(stack));
	if (le) {
		if (le->prev) {
			le = SRM_LLIST_PREV(le);
		}
		i = SRM_LLIST_VALP(le);

		RETURN_STRING(i->filename, 1);
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_enable)
{
	zend_error_cb = new_error_cb;
}

PHP_FUNCTION(xdebug_disable)
{
	zend_error_cb = old_error_cb;
}

PHP_FUNCTION(xdebug_is_enabled)
{
	RETURN_BOOL(zend_error_cb == new_error_cb);
}


PHP_FUNCTION(xdebug_start_trace)
{
	if (XG(do_trace) == 0) {
		XG(trace)    = xdebug_llist_alloc (stack_element_dtor);
		XG(do_trace) = 1;
	} else {
		php_error (E_NOTICE, "Function trace already started");
	}
}

PHP_FUNCTION(xdebug_stop_trace)
{
	if (XG(do_trace) == 1) {
		XG(do_trace) = 0;
		xdebug_llist_destroy (XG(trace), NULL);
		XG(trace)    = NULL;
	} else {
		php_error (E_NOTICE, "Function trace was not started");
	}
}

PHP_FUNCTION(xdebug_get_function_trace)
{
	if (XG(do_trace)) {
		print_trace (PG(html_errors) TSRMLS_CC);
	} else {
		php_error (E_NOTICE, "Function tracing was not started, use xdebug_start_trace() before calling this function");
	}
}


#if MEMORY_LIMIT
PHP_FUNCTION(xdebug_memory_usage)
{
	RETURN_LONG(AG(allocated_memory));
}
#endif

/*************************************************************************************************************************************/

static char* get_val (zend_op *op)
{
	zend_op *prev;

	if (op->op1.op_type != IS_CONST) {
		/* Check for constant */
		prev = op - 1;
		if (prev->op1.op_type == IS_CONST) {
			if (prev->op1.u.constant.type == IS_STRING) {
				return xdebug_sprintf ("[%s]", prev->op1.u.constant.value.str.val);
			}
		} else if (prev->op1.op_type == IS_TMP_VAR) { /* Check for array */
			if (prev->opcode == ZEND_ADD_ARRAY_ELEMENT) {
				return estrdup ("array()");
			}
		}
		return estrdup("?");
	}

	switch (op->op1.u.constant.type) {
		case IS_NULL:
			return estrdup("NULL");
			break;
		case IS_LONG:
			return xdebug_sprintf ("%ld", op->op1.u.constant.value.lval);
			break;
		case IS_DOUBLE:
			return xdebug_sprintf ("%g", op->op1.u.constant.value.dval);
			break;
		case IS_STRING:
			return xdebug_sprintf ("'%s'", op->op1.u.constant.value.str.val);
			break;
		case IS_BOOL:
			return op->op1.u.constant.value.lval ? estrdup ("TRUE") : estrdup ("FALSE");
			break;
		case IS_CONSTANT:
			return estrdup (op->op1.u.constant.value.str.val);
			break;
		default:
			return estrdup ("??");
	}
}


static char *get_var (zend_op *op, int opcode)
{
	zend_op *prev;

	prev = op - 1;

	while (prev->opcode != ZEND_EXT_FCALL_BEGIN) {
		if (prev->opcode != ZEND_FETCH_FUNC_ARG) {
			return estrdup ("?");
		}

		if (prev->op1.op_type == IS_CONST) {
			return xdebug_sprintf ("$%s", prev->op1.u.constant.value.str.val);
		} else {
			return estrdup ("$");
		}

		prev--;
	}

	if (prev->opcode == ZEND_EXT_FCALL_BEGIN) {
		return estrdup ("?");
	} else {
		return estrdup ("??");
	}
}

ZEND_DLEXPORT void xdebug_function_begin (zend_op_array *op_array)
{
	struct function_stack_entry* tmp;
	zend_op *cur_opcode;
	zend_op *end_opcode;
	char buffer[1024];
	TSRMLS_FETCH();
	
	tmp = emalloc (sizeof (struct function_stack_entry));
	tmp->varc     = 0;
	tmp->refcount = 1;
	tmp->level    = XG(level) + 1;

	cur_opcode = *EG(opline_ptr);
	end_opcode = op_array->opcodes + op_array->last + 1;

	while (cur_opcode < end_opcode) {
		int opcode = cur_opcode->opcode;

		if (opcode == ZEND_DO_FCALL			||
			opcode == ZEND_DO_FCALL_BY_NAME	||
			opcode == ZEND_INCLUDE_OR_EVAL	||
			opcode == ZEND_EXT_FCALL_END)
		{
			break;
		}
		switch (opcode) {
		  case ZEND_SEND_VAL:
			tmp->vars[tmp->varc] = get_val (cur_opcode);
			tmp->varc++;
			break;

		  case ZEND_SEND_VAR:
			tmp->vars[tmp->varc] = get_var (cur_opcode, opcode);
			tmp->varc++;
			break;

		  case ZEND_SEND_REF:
			tmp->vars[tmp->varc] = get_var (cur_opcode, opcode);
			tmp->varc++;
			break;

		}
		cur_opcode++;
	}

	tmp->function_name = NULL;
	switch (cur_opcode->opcode) {
		case ZEND_INCLUDE_OR_EVAL:
			/* Determine type */
			switch (cur_opcode->op2.u.constant.value.lval) {
				case ZEND_INCLUDE_ONCE:
					tmp->function_name = estrdup ("include_once");
					break;
				case ZEND_REQUIRE_ONCE:
					tmp->function_name = estrdup ("require_once");
					break;
				case ZEND_INCLUDE:
					tmp->function_name = estrdup ("include");
					break;
				case ZEND_REQUIRE:
					tmp->function_name = estrdup ("require");
					break;
				case ZEND_EVAL:
					tmp->function_name = estrdup ("eval");
					break;
			}
			
			switch (cur_opcode->op1.op_type) {
				case IS_CONST:
					sprintf (buffer, "'%s'", cur_opcode->op1.u.constant.value.str.val);
					tmp->vars[tmp->varc] = estrdup (buffer);
					tmp->varc++;
					break;
				default:
					break;
			}

			/* If it's not a constant name, back track to see what the variable name of the parameter was */
			if (tmp->varc == 0 && (cur_opcode - 2)->opcode == ZEND_FETCH_R) {
				if ((cur_opcode - 2)->op1.op_type == IS_CONST) {
					sprintf (buffer, "$%s", (cur_opcode - 2)->op1.u.constant.value.str.val);
					tmp->vars[tmp->varc] = estrdup (buffer);
					tmp->varc++;
				}
			}

			/* Otherwise fall back to ? */
			if (tmp->varc == 0) {
				tmp->vars[tmp->varc] = estrdup ("?");
				tmp->varc++;
			}
			break;
		case ZEND_DO_FCALL:
			switch (cur_opcode->op1.op_type) {
				case IS_CONST:
					tmp->function_name = estrndup (cur_opcode->op1.u.constant.value.str.val, cur_opcode->op1.u.constant.value.str.len);
					break;
				case IS_VAR:
					if (CG(class_entry).name) {
						sprintf (buffer, "%s->%p", CG(class_entry).name, cur_opcode->op2.u.constant.value.str.val);
					} else {
						sprintf (buffer, "?->%s", cur_opcode->op1.u.constant.value.str.val);
					}
					tmp->function_name = estrdup (buffer);
					break;
				default:
					sprintf (buffer, "?->%s", cur_opcode->op1.u.constant.value.str.val);
					tmp->function_name = estrdup (buffer);
					break;
			}
			break;
		case ZEND_DO_FCALL_BY_NAME: {
			zend_op* tmpOpCode;

			tmpOpCode = cur_opcode;
			while (tmpOpCode->opcode != ZEND_INIT_FCALL_BY_NAME) {
				tmpOpCode--;
			}
			switch (tmpOpCode->op1.op_type)  {
				case IS_UNUSED:
					switch (tmpOpCode->op2.op_type) {
						case IS_CONST:
							sprintf(buffer, "%s",
								tmpOpCode->op2.u.constant.value.str.val
							);
							tmp->function_name = estrdup(buffer);
							break;
						default:  /* FIXME need better IS_VAR handling */
							tmp->function_name = estrdup("null");
							break;
	
					}
					break;
				case IS_CONST:
					switch (tmpOpCode->op2.op_type) {
						case IS_CONST:
							sprintf(buffer, "%s::%s",
								tmpOpCode->op1.u.constant.value.str.val,
								tmpOpCode->op2.u.constant.value.str.val
							);
							tmp->function_name = estrdup(buffer);
							break;
						default:  /* FIXME need better IS_VAR handling */
							tmp->function_name = estrdup("null");
							break;
	
					}
					break;
				case IS_VAR:
					if (tmpOpCode->op1.op_type == IS_CONST)   {
						switch(tmpOpCode->op2.op_type) {
							case IS_CONST:
								sprintf(buffer, "%s->%s",
									tmpOpCode->op1.u.constant.value.str.val,
									tmpOpCode->op2.u.constant.value.str.val
								);
								break;
							default:
								sprintf(buffer, "%s-><???>",
									tmpOpCode->op1.u.constant.value.str.val
								);
								break;
						}
					}
					else if(CG(class_entry).name) {
						switch(tmpOpCode->op2.op_type) {
							case IS_CONST:
								sprintf(buffer, "%s->%s",
									CG(class_entry).name,
									tmpOpCode->op2.u.constant.value.str.val
								);
								break;
							default:
								sprintf(buffer, "%s-><???>",
									CG(class_entry).name
								);
								break;
						}
					}
					else {
						switch(tmpOpCode->op2.op_type) {
							case IS_CONST:
								sprintf(buffer, "<???>::%s",
									tmpOpCode->op2.u.constant.value.str.val
								);
								break;
							default:
								sprintf(buffer, "<???>::<???>");
								break;
						}
					}
					tmp->function_name = estrdup(buffer);
				}

			}
			break;
	}
	tmp->filename  = op_array->filename ? estrdup(op_array->filename): NULL;
	tmp->lineno    = cur_opcode->lineno;

	xdebug_llist_insert_next (XG(stack), SRM_LLIST_TAIL(XG(stack)), tmp);
	if (XG(do_trace)) {
		tmp->refcount++;
		xdebug_llist_insert_next (XG(trace), SRM_LLIST_TAIL(XG(trace)), tmp);
	}

	XG(level)++;
	if (XG(level) == XG(max_nesting_level)) {
		php_error (E_ERROR, "Maximum function nesting level (%d) reached, possible infinite recursion", XG(max_nesting_level));
	}
}

ZEND_DLEXPORT void xdebug_function_end (zend_op_array *op_array)
{
	TSRMLS_FETCH();

	xdebug_llist_remove (XG(stack), SRM_LLIST_TAIL(XG(stack)), stack_element_dtor);
	XG(level)--;
}

ZEND_DLEXPORT void xdebug_statement_call (zend_op_array *op_array)
{
	/* printf ("call statement, %s line %d\n", op_array->filename, op_array->opcodes[0].lineno); */
}


ZEND_DLEXPORT int xdebug_zend_startup(zend_extension *extension)
{
	TSRMLS_FETCH();
	CG(extended_info) = 1;
	return zend_startup_module(&xdebug_module_entry);
}

ZEND_DLEXPORT void xdebug_zend_shutdown(zend_extension *extension)
{
	/* Do nothing. */
}

#ifndef ZEND_EXT_API
#define ZEND_EXT_API	ZEND_DLEXPORT
#endif
ZEND_EXTENSION();

ZEND_DLEXPORT zend_extension zend_extension_entry = {
	"eXtended Debugger (xdebug)",
	XDEBUG_VERSION,
	"Derick Rethans",
	"http://www.jdimedia.nl/derick/xdebug.php",
	"Copyright (c) 2002 JDI Media Solutions",
	xdebug_zend_startup,
	xdebug_zend_shutdown,
	NULL,           /* activate_func_t */
	NULL,           /* deactivate_func_t */
	NULL,           /* message_handler_func_t */
	NULL,           /* op_array_handler_func_t */
	xdebug_statement_call, /* statement_handler_func_t */
	xdebug_function_begin, /* fcall_begin_handler_func_t */
	xdebug_function_end,   /* fcall_end_handler_func_t */
	NULL,           /* op_array_ctor_func_t */
	NULL,           /* op_array_dtor_func_t */
	STANDARD_ZEND_EXTENSION_PROPERTIES
};


#endif
