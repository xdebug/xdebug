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
   | Authors:  Derick Rethans <d.rethans@jdimedia.nl>                     |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

#if HAVE_XDEBUG

#define XDEBUG_VERSION "1.1.0dev"

#ifndef PHP_WIN32
#include <sys/time.h>
#else
#include "win32/time.h"
#endif

#include "TSRM.h"
#include "SAPI.h"
#include "php_ini.h"
#include "ext/standard/html.h"
#include "ext/standard/info.h"
#include "ext/standard/php_smart_str.h"
#include "php_globals.h"

#include "zend.h"
#include "zend_API.h"
#include "zend_execute.h"
#include "zend_compile.h"
#include "zend_extensions.h"

#include "xdebug_com.h"
#include "xdebug_llist.h"
#include "xdebug_var.h"
#include "php_xdebug.h"

static int le_xdebug;

zend_op_array* (*old_compile_file)(zend_file_handle* file_handle, int type TSRMLS_DC);
zend_op_array* xdebug_compile_file(zend_file_handle*, int TSRMLS_DC);

void (*old_execute)(zend_op_array *op_array TSRMLS_DC);
void xdebug_execute(zend_op_array *op_array TSRMLS_DC);

#if ZEND_EXTENSION_API_NO >= 20020824
void (*old_execute_internal)(zend_execute_data *current_execute_data, int return_value_used TSRMLS_DC);
void xdebug_execute_internal(zend_execute_data *current_execute_data, int return_value_used TSRMLS_DC);
#endif

void (*old_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);
void (*new_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);
void xdebug_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);

function_entry xdebug_functions[] = {
	PHP_FE(xdebug_get_function_stack,  NULL)
	PHP_FE(xdebug_call_function,       NULL)
	PHP_FE(xdebug_call_file,           NULL)
	PHP_FE(xdebug_call_line,           NULL)

	PHP_FE(xdebug_enable,              NULL)
	PHP_FE(xdebug_disable,             NULL)
	PHP_FE(xdebug_is_enabled,          NULL)

	PHP_FE(xdebug_start_trace,         NULL)
	PHP_FE(xdebug_stop_trace,          NULL)
	PHP_FE(xdebug_get_function_trace,  NULL)
	PHP_FE(xdebug_dump_function_trace, NULL)
#if MEMORY_LIMIT
	PHP_FE(xdebug_memory_usage,        NULL)
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
	STD_PHP_INI_ENTRY("xdebug.max_nesting_level", "64",                 PHP_INI_SYSTEM, OnUpdateInt,    max_nesting_level, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.default_enable",  "1",                  PHP_INI_SYSTEM, OnUpdateBool,   default_enable,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.manual_url",        "http://www.php.net", PHP_INI_SYSTEM, OnUpdateString, manual_url,        zend_xdebug_globals, xdebug_globals)

	/* Remote debugger settings */
	STD_PHP_INI_BOOLEAN("xdebug.remote_enable",   "1",                  PHP_INI_SYSTEM, OnUpdateBool,   remote_enable,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_port",       "7869",               PHP_INI_SYSTEM, OnUpdateInt,    remote_port,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_host",       "localhost",          PHP_INI_SYSTEM, OnUpdateString, remote_host,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_mode",       "php3",               PHP_INI_SYSTEM, OnUpdateString, remote_mode,       zend_xdebug_globals, xdebug_globals)
PHP_INI_END()

#define MICRO_IN_SEC 1000000.00
	
static double get_utime()
{
#ifdef HAVE_GETTIMEOFDAY
	struct timeval tp;
	long sec = 0L;
	double msec = 0.0;

	if (gettimeofday((struct timeval *) &tp, NULL) == 0) {
		sec = tp.tv_sec;
		msec = (double) (tp.tv_usec / MICRO_IN_SEC);

		if (msec >= 1.0) msec -= (long) msec;
		return msec + sec;
	} else {
#endif
		return 0;
	}
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

#if ZEND_EXTENSION_API_NO >= 20020824
	old_execute_internal = zend_execute_internal;
	zend_execute_internal = xdebug_execute_internal;
#endif

	old_error_cb = zend_error_cb;
	new_error_cb = xdebug_error_cb;

	return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(xdebug)
{
	zend_compile_file = old_compile_file;
	zend_execute = old_execute;
#if ZEND_EXTENSION_API_NO >= 20020824
	zend_execute_internal = old_execute_internal;
#endif
	zend_error_cb = old_error_cb;

	return SUCCESS;
}


void stack_element_dtor (void *dummy, void *elem)
{
	int                   i;
	function_stack_entry *e = elem;

	e->refcount--;

	if (e->refcount == 0) {
		if (e->function.function) {
			xdfree (e->function.function);
		}
		if (e->function.class) {
			xdfree (e->function.class);
		}
		if (e->filename) {
			xdfree (e->filename);
		}

		for (i = 0; i < e->varc; i++) {
			if ((e->vars[i]).name) {
				xdfree ((e->vars[i]).name);
			}
			xdfree ((e->vars[i]).value);
		}

		xdfree (e);
	}
}



PHP_RINIT_FUNCTION(xdebug)
{
	CG(extended_info) = 1;
	XG(level)    = 0;
	XG(do_trace) = 0;
	XG(stack)    = xdebug_llist_alloc (stack_element_dtor);
	XG(trace_file) = NULL;

	if (XG(default_enable)) {
		zend_error_cb = new_error_cb;
	}

	/* Start context if requested */
	XG(remote_enabled) = 0;
	if (XG(remote_enable)) {
		XG(context).socket = xdebug_create_socket(XG(remote_host), XG(remote_port));
		if (XG(context).socket >= 0) {
			XG(remote_enabled) = 1;

			/* Get handler from mode */
			XG(remote_handler) = xdebug_handler_get(XG(remote_mode));
			XG(remote_handler)->remote_init(XG(context));
		}
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

	if (XG(trace_file)) {
		fprintf (XG(trace_file), "End of function trace\n");
		fclose (XG(trace_file));
	}

	XG(level)    = 0;
	XG(do_trace) = 0;

	if (XG(remote_enabled)) {
		XG(remote_handler)->remote_deinit(XG(context));
		xdebug_close_socket(XG(context).socket); 
	}

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
		tmp = xdmalloc (sizeof (struct function_stack_entry));
		tmp->varc     = 0;
		tmp->refcount = 1;
		tmp->level    = ++XG(level);
		tmp->delayed_fname = 0;
		tmp->delayed_cname = 0;
		tmp->delayed_include   = 0;
		tmp->function.function = xdstrdup("{main}");
		tmp->function.class    = NULL;
		tmp->function.type     = XFUNC_NORMAL;

		tmp->filename  = op_array->filename ? xdstrdup(op_array->filename): NULL;
		tmp->lineno    = 0;
#if MEMORY_LIMIT
		tmp->memory    = AG(allocated_memory);
#else
		tmp->memory    = 0;
#endif
		tmp->time      = get_utime();

		/* Handle delayed include for stack */
		if (XG(stack)->size > 0) {
			if (((function_stack_entry*) XDEBUG_LLIST_TAIL(XG(stack))->ptr)->delayed_include == 1) {
				((function_stack_entry*) XDEBUG_LLIST_TAIL(XG(stack))->ptr)->vars[0].name = xdstrdup ("");
				((function_stack_entry*) XDEBUG_LLIST_TAIL(XG(stack))->ptr)->vars[0].value = op_array->filename ? xdstrdup(op_array->filename): NULL;
				((function_stack_entry*) XDEBUG_LLIST_TAIL(XG(stack))->ptr)->varc++;
			}
		}
		xdebug_llist_insert_next (XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), tmp);
		if (XG(do_trace)) {
			tmp->refcount++;
			xdebug_llist_insert_next (XG(trace), XDEBUG_LLIST_TAIL(XG(trace)), tmp);
		}

		old_execute (op_array TSRMLS_CC);

		xdebug_llist_remove (XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), stack_element_dtor);
		XG(level)--;
	} else {
		old_execute (op_array TSRMLS_CC);
	}
}

#if ZEND_EXTENSION_API_NO >= 20020824
void xdebug_execute_internal(zend_execute_data *current_execute_data, int return_value_used TSRMLS_DC)
{
	zval                **param;
	void                **p         = EG(argument_stack).top_element-2;
	int                   arg_count = (ulong) *p;
	int                   i         = 0;
	function_stack_entry *fse       = XG(stack)->tail->ptr;
	
	for (i = 0; i < arg_count; i++) {
		fse->vars[fse->varc].name  = NULL;
		if (zend_ptr_stack_get_arg(fse->varc + 1, (void**) &param TSRMLS_CC) == SUCCESS) {
			fse->vars[fse->varc].value = get_zval_value(*param);
		} else {
			fse->vars[fse->varc].value = xdstrdup ("{missing}");
		}
		fse->varc++;
	}

	execute_internal(current_execute_data, return_value_used TSRMLS_CC);
}
#endif

static inline char* show_fname (struct function_stack_entry* entry TSRMLS_DC)
{
	char *tmp;
	xdebug_func f;

	f = entry->function;

	switch (f.type) {
		case XFUNC_NORMAL: {
			zend_function *zfunc;

			if (!(strcmp ("cli", sapi_module.name) == 0) && zend_hash_find(EG(function_table), f.function, strlen(f.function) + 1, (void**) &zfunc) == SUCCESS) {
				if (zfunc->type == ZEND_INTERNAL_FUNCTION) {
					return xdebug_sprintf ("<a href='%s/%s' target='_new'>%s</a>\n", XG(manual_url), f.function, f.function);
				} else {
					return xdstrdup (f.function);
				}
			} else {
				return xdstrdup (f.function);
			}
			break;
		}

		case XFUNC_NEW:
			if (!f.class) {
				f.class = "?";
			}
			if (!f.function) {
				f.function = "?";
			}
			tmp = xdmalloc (strlen (f.class) + 4 + 1);
			sprintf (tmp, "new %s", f.class);
			return tmp;
			break;

		case XFUNC_STATIC_MEMBER:
			if (!f.class) {
				f.class = "?";
			}
			if (!f.function) {
				f.function = "?";
			}
			tmp = xdmalloc (strlen (f.function) + strlen (f.class) + 2 + 1);
			sprintf (tmp, "%s::%s", f.class, f.function);
			return tmp;
			break;

		case XFUNC_MEMBER:
			if (!f.class) {
				f.class = "?";
			}
			if (!f.function) {
				f.function = "?";
			}
			tmp = xdmalloc (strlen (f.function) + strlen (f.class) + 2 + 1);
			sprintf (tmp, "%s->%s", f.class, f.function);
			return tmp;
			break;

		case XFUNC_EVAL:
			return xdstrdup ("eval");
			break;

		case XFUNC_INCLUDE:
			return xdstrdup ("include");
			break;

		case XFUNC_INCLUDE_ONCE:
			return xdstrdup ("include_once");
			break;

		case XFUNC_REQUIRE:
			return xdstrdup ("require");
			break;

		case XFUNC_REQUIRE_ONCE:
			return xdstrdup ("require_once");
			break;

		default:
			return xdstrdup ("{unknown, please report}");
	}
}

static inline void print_stack (int html, const char *error_type_str, char *buffer, const char *error_filename, const int error_lineno TSRMLS_DC)
{
	char *error_format;
	xdebug_llist_element *le;
	int new_len;
	int is_cli = (strcmp ("cli", sapi_module.name) == 0);

	if (html) {
		php_printf ("<br />\n<table border='1' cellspacing='0'>\n");
	}

	error_format = html ?
		"<tr><td bgcolor='#ffbbbb' colspan=\"3\"><b>%s</b>: %s in <b>%s</b> on line <b>%d</b><br />\n"
		: "\n%s: %s in %s on line %d\n";
	php_printf(error_format, error_type_str, buffer, error_filename, error_lineno);

	if (XG(stack)) {
		if (html) {
			php_printf ("<tr><th bgcolor='#aaaaaa' colspan='3'>Call Stack</th></tr>\n");
			php_printf ("<tr><th bgcolor='#cccccc'>#</th><th bgcolor='#cccccc'>Function</th><th bgcolor='#cccccc'>Location</th></tr>\n");
		} else {
			printf ("\nCall Stack:\n");
		}

		if (PG(log_errors) && !is_cli) {
			php_log_err("PHP Stack trace:" TSRMLS_CC);
		}

		for (le = XDEBUG_LLIST_HEAD(XG(stack)); le != NULL; le = XDEBUG_LLIST_NEXT(le))
		{
			int c = 0; /* Comma flag */
			int j = 0; /* Counter */
			struct function_stack_entry *i = XDEBUG_LLIST_VALP(le);
			char *tmp_name;
			char log_buffer[4096];
			
			tmp_name = show_fname (i TSRMLS_CC);
			if (html) {
				php_printf ("<tr><td bgcolor='#ffffff' align='center'>%d</td><td bgcolor='#ffffff'>%s(", i->level, tmp_name);
			} else {
				printf ("%3d. %s(", i->level, tmp_name);
			}
			if (PG(log_errors) && !is_cli) {
				snprintf(log_buffer, 1024, "PHP %3d. %s(", i->level, tmp_name);
			}
			xdfree (tmp_name);

			/* Printing vars */
			for (j = 0; j < i->varc; j++) {
				char *tmp_varname;

				if (c) {
					if (html) {
						php_printf (", ");
					} else {
						printf (", ");
					}
					if (PG(log_errors) && !is_cli) {
						strcat(log_buffer, ", ");
					}
				} else {
					c = 1;
				}
				tmp_varname = i->vars[j].name ? xdebug_sprintf ("$%s = ", i->vars[j].name) : xdstrdup("");
				if (html) {
					php_printf ("%s%s", tmp_varname,
						php_escape_html_entities (i->vars[j].value, strlen(i->vars[j].value), &new_len, 1, 1, NULL));
				} else {
					printf ("%s%s", tmp_varname, i->vars[j].value);
				}
				if (PG(log_errors) && !is_cli) {
					snprintf(
						log_buffer + strlen(log_buffer),
						1024 - strlen(log_buffer),
						"%s%s", tmp_varname, i->vars[j].value
					);
				}
				xdfree(tmp_varname);
			}

			if (html) {
				php_printf (")</td><td bgcolor='#ffffff'>%s<b>:</b>%d</td></tr>\n", i->filename, i->lineno);
			} else {
				printf (") %s:%d\n", i->filename, i->lineno);
			}
			if (PG(log_errors) && !is_cli) {
				snprintf(
					log_buffer + strlen(log_buffer),
					1024 - strlen(log_buffer),
					") %s:%d", i->filename, i->lineno
				);
				php_log_err(log_buffer TSRMLS_CC);
			}
		}

		if (html) {
			php_printf ("</table>\n");
		}
	}
}

static inline void print_trace (int html TSRMLS_DC)
{
	xdebug_llist_element *le;
	int new_len;
	double start_time = 0;

	if (XG(trace)) {
		if (html) {
			php_printf ("<br />\n<table border='1' cellspacing='0'>\n");
		} else {
			printf ("\nFunction trace:\n");
		}

		if (html) {
#if MEMORY_LIMIT
			php_printf ("<tr><th bgcolor='#aaaaaa' colspan='5'>Function trace</th></tr>\n");
			php_printf ("<tr><th bgcolor='#cccccc'>Time</th><th bgcolor='#cccccc'>#</th><th bgcolor='#cccccc'>Function</th><th bgcolor='#cccccc'>Location</th><th bgcolor='#cccccc'>Memory</th></tr>\n");
#else
			php_printf ("<tr><th bgcolor='#aaaaaa' colspan='4'>Function trace</th></tr>\n");
			php_printf ("<tr><th bgcolor='#cccccc'>Time</th><th bgcolor='#cccccc'>#</th><th bgcolor='#cccccc'>Function</th><th bgcolor='#cccccc'>Location</th></tr>\n");
#endif
		}

		for (le = XDEBUG_LLIST_HEAD(XG(trace)); le != NULL; le = XDEBUG_LLIST_NEXT(le))
		{
			int c = 0; /* Comma flag */
			int j = 0; /* Counter */
			struct function_stack_entry *i = XDEBUG_LLIST_VALP(le);
			char *tmp_name;

			if (i->function.function && strcmp (i->function.function, "xdebug_dump_function_trace") == 0) {
				return;
			}

			tmp_name = show_fname(i TSRMLS_CC);

			if (html) {
				/* Start row */
				php_printf ("<tr>");

				/* Do timestamp */
				php_printf ("<td bgcolor='#ffffff' align='center'>");
				if (start_time) {
					php_printf ("%.6f", i->time - start_time);
				} else {
					start_time = i->time;
					php_printf ("0");
				}
				php_printf ("</td>");

				/* Do rest of line */
				php_printf ("<td bgcolor='#ffffff' align='left'><pre>");
				for (j = 0; j < i->level - 1; j++) {
					php_printf ("  ");
				}
				php_printf ("-></pre></td><td bgcolor='#ffffff'>%s(", tmp_name);
			} else {
				for (j = 0; j < i->level; j++) {
					printf ("  ");
				}
				printf ("-> %s(", tmp_name);
			}
			xdfree (tmp_name);

			/* Printing vars */
			for (j = 0; j < i->varc; j++) {
				char *tmp_varname;

				if (c) {
					if (html) {
						php_printf (", ");
					} else {
						printf (", ");
					}
				} else {
					c = 1;
				}

				tmp_varname = i->vars[j].name ? xdebug_sprintf ("$%s = ", i->vars[j].name) : xdstrdup("");
				if (html) {
					php_printf ("%s%s", tmp_varname,
						php_escape_html_entities (i->vars[j].value, strlen(i->vars[j].value), &new_len, 1, 1, NULL));
				} else {
					printf ("%s%s", tmp_varname, i->vars[j].value);
				}
				xdfree (tmp_varname);
			}

			if (html) {
				/* Do filename and line no */
				php_printf (")</td><td bgcolor='#ffffff'>%s<b>:</b>%d</td>", i->filename, i->lineno);
#if MEMORY_LIMIT
				/* Do memory */
				php_printf ("<td bgcolor='#ffffff' align='right'>%lu</td>", i->memory);
#endif
				/* Close row */
				php_printf ("</tr>\n");
			} else {
				printf (") %s:%d\n", i->filename, i->lineno);
			}
		}

		if (html) {
			php_printf ("</table>\n");
		}
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

	error_type_str = error_type(type);

	if (EG(error_reporting) & type) {
		print_stack (!(strcmp ("cli", sapi_module.name) == 0), error_type_str, buffer, error_filename, error_lineno TSRMLS_CC);
	}

	/* Log to logger */
	if (PG(log_errors) && !(strcmp ("cli", sapi_module.name) == 0)) {
		char log_buffer[1024];

#ifdef PHP_WIN32
		if (type==E_CORE_ERROR || type==E_CORE_WARNING) {
			MessageBox(NULL, buffer, error_type_str, MB_OK|ZEND_SERVICE_MB_STYLE);
		}
#endif
		snprintf(log_buffer, 1024, "PHP %s:  %s in %s on line %d", error_type_str, buffer, error_filename, error_lineno);
		php_log_err(log_buffer TSRMLS_CC);
	}
	xdfree(error_type_str);

	if (XG(remote_enabled)) {
		XG(remote_handler)->remote_error(XG(context), type, buffer, error_filename, error_lineno, XG(stack));
	}
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
	xdebug_llist_element *le;
	int                   j;
	unsigned int          k;
	zval                 *frame;
	zval                 *params;

	array_init(return_value);
	le = XDEBUG_LLIST_HEAD(XG(stack));

	for (k = 0; k < XG(stack)->size - 1; k++, le = XDEBUG_LLIST_NEXT(le)) {
		struct function_stack_entry *i = XDEBUG_LLIST_VALP(le);

		if (i->function.function) {
			if (strcmp (i->function.function, "xdebug_get_function_stack") == 0) {
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
			add_assoc_string_ex(frame, "class",    sizeof("class"),    i->function.class,    1);
		}
		add_assoc_string_ex(frame, "file", sizeof("file"), i->filename, 1);
		add_assoc_long_ex  (frame, "line", sizeof("line"), i->lineno);

		/* Add parameters */
		MAKE_STD_ZVAL(params);
		array_init(params);
		for (j = 0; j < i->varc; j++) {
			add_assoc_string_ex(params, i->vars[j].name, strlen (i->vars[j].name) + 1, i->vars[j].value, 1);
		}
		add_assoc_zval_ex  (frame, "params", sizeof("params"), params);

		add_next_index_zval(return_value, frame);
	}
}

PHP_FUNCTION(xdebug_call_function)
{
	xdebug_llist_element        *le;
	struct function_stack_entry *i;

	le = XDEBUG_LLIST_TAIL(XG(stack));
	if (le) {
		if (le->prev) {
			le = XDEBUG_LLIST_PREV(le);
			if (le->prev) {
				le = XDEBUG_LLIST_PREV(le);
			}
		}
		i = XDEBUG_LLIST_VALP(le);

		RETURN_STRING(i->function.function ? i->function.function : "{}", 1);
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_call_line)
{
	xdebug_llist_element        *le;
	struct function_stack_entry *i;

	le = XDEBUG_LLIST_TAIL(XG(stack));
	if (le) {
		if (le->prev) {
			le = XDEBUG_LLIST_PREV(le);
		}
		i = XDEBUG_LLIST_VALP(le);

		RETURN_LONG(i->lineno);
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_call_file)
{
	xdebug_llist_element        *le;
	struct function_stack_entry *i;

	le = XDEBUG_LLIST_TAIL(XG(stack));
	if (le) {
		if (le->prev) {
			le = XDEBUG_LLIST_PREV(le);
		}
		i = XDEBUG_LLIST_VALP(le);

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
	char *fname = NULL;
	int   fname_len = 0;

	if (XG(do_trace) == 0) {
		if (zend_parse_parameters (ZEND_NUM_ARGS() TSRMLS_CC, "|s", &fname, &fname_len) == FAILURE) {
			return;
		}

		XG(trace)    = xdebug_llist_alloc (stack_element_dtor);
		XG(do_trace) = 1;

		if (fname) {
			XG(trace_file) = fopen (fname, "a");
			if (XG(trace_file)) {
				fprintf (XG(trace_file), "\nStart of function trace\n");
			}
		} else {
			XG(trace_file) = NULL;
		}
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
		if (XG(trace_file)) {
			fprintf (XG(trace_file), "End of function trace\n");
			fclose (XG(trace_file));
		}
	} else {
		php_error (E_NOTICE, "Function trace was not started");
	}
}

PHP_FUNCTION(xdebug_dump_function_trace)
{
	if (XG(do_trace)) {
		print_trace (PG(html_errors) TSRMLS_CC);
	} else {
		php_error (E_NOTICE, "Function tracing was not started, use xdebug_start_trace() before calling this function");
	}
}

PHP_FUNCTION(xdebug_get_function_trace)
{
	xdebug_llist_element *le;
	int                   j;
	unsigned int          k;
	zval                 *frame;
	zval                 *params;

	if (!XG(do_trace)) {
		php_error (E_NOTICE, "Function tracing was not started, use xdebug_start_trace() before calling this function");
		RETURN_FALSE;
	}

	array_init(return_value);
	le = XDEBUG_LLIST_HEAD(XG(trace));
	
	for (k = 0; k < XG(trace)->size - 1; k++, le = XDEBUG_LLIST_NEXT(le)) {
		struct function_stack_entry *i = XDEBUG_LLIST_VALP(le);

		if (i->function.function) {
			if (strcmp (i->function.function, "xdebug_get_function_trace") == 0) {
				return;
			}
		}
		/* Initialize frame array */
		MAKE_STD_ZVAL(frame);
		array_init(frame);

		/* Add data */
		if (i->function.function) {
			add_assoc_string_ex(frame, "function", sizeof("function"), i->function.function, 1);
		} else {
			switch (i->function.type) {
				case XFUNC_NEW:
					add_assoc_string_ex(frame, "function", sizeof("function"), "{new}", 1);
					break;

				case XFUNC_EVAL:
					add_assoc_string_ex(frame, "function", sizeof("function"), "{eval}", 1);
					break;

				case XFUNC_INCLUDE:
					add_assoc_string_ex(frame, "function", sizeof("function"), "{include}", 1);
					break;

				case XFUNC_INCLUDE_ONCE:
					add_assoc_string_ex(frame, "function", sizeof("function"), "{include_once}", 1);
					break;

				case XFUNC_REQUIRE:
					add_assoc_string_ex(frame, "function", sizeof("function"), "{require}", 1);
					break;

				case XFUNC_REQUIRE_ONCE:
					add_assoc_string_ex(frame, "function", sizeof("function"), "{require_once}", 1);
					break;

			}
		}
		if (i->function.class) {
			add_assoc_string_ex(frame, "class", sizeof("class"), i->function.class, 1);
		}
		add_assoc_string_ex(frame, "file", sizeof("file"), i->filename, 1);
		add_assoc_long_ex  (frame, "line", sizeof("line"), i->lineno);

		/* Add parameters */
		MAKE_STD_ZVAL(params);
		array_init(params);
		for (j = 0; j < i->varc; j++) {
			if (i->vars[j].name) {
				add_assoc_string_ex(params, i->vars[j].name, strlen (i->vars[j].name) + 1, i->vars[j].value, 1);
			} else {
				add_assoc_string_ex(params, "1", sizeof("1"), i->vars[j].value, 1);
			}
		}
		add_assoc_zval_ex  (frame, "params", sizeof("params"), params);

		add_next_index_zval(return_value, frame);
	}
}


#if MEMORY_LIMIT
PHP_FUNCTION(xdebug_memory_usage)
{
	RETURN_LONG(AG(allocated_memory));
}
#endif

/*************************************************************************************************************************************/

static inline zval *get_zval(znode *node, temp_variable *Ts, int *is_var)
{
	switch (node->op_type) {
		case IS_CONST:
			return &node->u.constant;
			break;

		case IS_TMP_VAR:
			*is_var = 1;
			return &Ts[node->u.var].tmp_var;
			break;

		case IS_VAR:
			*is_var = 1;
			if (Ts[node->u.var].var.ptr) {
				return Ts[node->u.var].var.ptr;
			} else {
				fprintf(stderr, "\nIS_VAR\n");
			}
			break;

		case IS_UNUSED:
			fprintf(stderr, "\nIS_UNUSED\n");
			break;

		default:
			fprintf(stderr, "\ndefault %d\n", node->op_type);
			break;
	}

	*is_var = 1;

	return NULL;
}

xdebug_func find_func_name(zend_op_array *op_array, zend_op *my_opcode, int *varc, xdebug_var *var0 TSRMLS_DC)
{
	zend_op *cur_opcode = my_opcode;
	zend_op *end_opcode;
	zval *var;
	int  func_nest = 0;
	int  go_back   = 0;
	xdebug_func cf;
	int is_var = 0;
	zend_op* tmpOpCode;
	zend_op* initOpCode = 0;
	int done = 0;

	memset(&cf, 0, sizeof(xdebug_func));

	end_opcode = op_array->opcodes + op_array->last;

	while (cur_opcode < end_opcode) {
		switch (cur_opcode->opcode) {
			case ZEND_NEW:
#if HAVE_EXECUTE_DATA_PTR
				var = get_zval(&(cur_opcode->op1), EG(current_execute_data)->Ts, &is_var);
				assert(var);
				cf.class = xdstrdup(var->value.str.val);
#else
				cf.class = xdstrdup("{unknown}");
#endif
				break;

			case ZEND_INIT_FCALL_BY_NAME:
				if (!initOpCode) { 
					initOpCode = cur_opcode;
				}
				break;

			case ZEND_EXT_FCALL_BEGIN:
				func_nest++;
				break;

			case ZEND_EXT_FCALL_END:
				func_nest--;
				break;

			case ZEND_DO_FCALL:
				if (func_nest == 1) {
#if HAVE_EXECUTE_DATA_PTR
					var = get_zval(&(cur_opcode->op1), EG(current_execute_data)->Ts, &is_var);
					assert(var);
					cf.function = xdstrdup(var->value.str.val);
#else
					cf.function = xdstrdup("{unknown}");
#endif
					cf.type = XFUNC_NORMAL;
					done = 1;
				}
				break;

			case ZEND_INCLUDE_OR_EVAL:
				if (cur_opcode->op2.u.constant.value.lval == ZEND_EVAL) {
					cf.type = XFUNC_EVAL;
				} else {
					switch (cur_opcode->op2.u.constant.value.lval) {
						case ZEND_INCLUDE_ONCE: cf.type = XFUNC_INCLUDE_ONCE; break;
						case ZEND_REQUIRE_ONCE: cf.type = XFUNC_REQUIRE_ONCE; break;
						case ZEND_INCLUDE:      cf.type = XFUNC_INCLUDE; break;
						case ZEND_REQUIRE:      cf.type = XFUNC_REQUIRE; break;
					}
				}
#if HAVE_EXECUTE_DATA_PTR
				(*varc)++;
				(*var0).name = NULL;
				var = get_zval(&(cur_opcode->op1), EG(current_execute_data)->Ts, &is_var);
				assert(var);
				(*var0).value = xdebug_sprintf ("'%s'", var->value.str.val);
#endif
				done = 1;
				break;

			case ZEND_DO_FCALL_BY_NAME:
				if (func_nest == 1) {
					if (!initOpCode) {
						tmpOpCode = my_opcode - 1;
					} else {
						tmpOpCode = initOpCode;
					}

#if ZEND_EXTENSION_API_NO >= 20020731
					assert(tmpOpCode->opcode == ZEND_INIT_FCALL_BY_NAME);
#endif

					/*
						tmpOpCode = initOpCode;
						printf("using 0x%08x %s\n", initOpCode, getOpcodeName(initOpCode->opcode));
					} else {
						tmpOpCode = cur_opcode;
						while (tmpOpCode->opcode != ZEND_INIT_FCALL_BY_NAME || go_back != 0) {
							tmpOpCode--;
							if (tmpOpCode->opcode == ZEND_INIT_FCALL_BY_NAME && go_back > 0) {
								go_back--;
							}
						}
						printf("found 0x%08x %s\n", tmpOpCode, getOpcodeName(tmpOpCode->opcode));
					}
					*/

					if (tmpOpCode->extended_value & ZEND_CTOR_CALL) {
						cf.type = XFUNC_NEW;
					} else {
						cf.type = XFUNC_NORMAL; /* may be overwritten later */
					}
#if 0
					if (EG(current_execute_data)->fbc) {
						is_var = 1; /* can we do this smarter - see below? */
						cf.fname = EG(current_execute_data)->fbc->common.function_name;
					} else 
#endif
					{
#if HAVE_EXECUTE_DATA_PTR
						var = get_zval(&(tmpOpCode->op2), EG(current_execute_data)->Ts, &is_var);
						assert(var);
						cf.function = xdstrdup(var->value.str.val);
#else
						cf.function = xdstrdup("{unknown}");
#endif
					}

					if (tmpOpCode->op1.op_type != IS_UNUSED) {
						if (tmpOpCode->op1.op_type == IS_CONST) {
							cf.type = XFUNC_STATIC_MEMBER;
							cf.class = xdstrdup(tmpOpCode->op1.u.constant.value.str.val);
						} else {
#if HAVE_EXECUTE_DATA_PTR
							if (EG(current_execute_data)->object.ptr) {
								cf.type = XFUNC_MEMBER;
								cf.class = xdstrdup(EG(current_execute_data)->object.ptr->value.obj.ce->name);
							} else {
								assert(cf.class);
							}
#else
							cf.class = xdstrdup("{unknown}");
#endif
						}
					} 
					done = 1;
				}
				break;
		}

		if (done) {
			break;
		} else {
			cur_opcode++;
		}
	}
	return cf;
}


ZEND_DLEXPORT void xdebug_function_begin (zend_op_array *op_array)
{
	struct function_stack_entry* tmp;
	zend_op *cur_opcode;
#if ZEND_EXTENSION_API_NO < 20020731
	zend_op *end_opcode;
	char buffer[1024];
#endif
	int  func_nest = 0;
	int  go_back   = 0;
	TSRMLS_FETCH();

	tmp = xdmalloc (sizeof (struct function_stack_entry));
	tmp->varc          = 0;
	tmp->refcount      = 1;
	tmp->level         = XG(level) + 1;
	tmp->arg_done      = 0;
	tmp->delayed_fname = 0;
	tmp->delayed_cname = 0;
	tmp->delayed_include   = 0;
	tmp->function.function = NULL;
	tmp->function.class    = NULL;
	tmp->function.type     = XFUNC_UNKNOWN;

	cur_opcode = *EG(opline_ptr);

#if ZEND_EXTENSION_API_NO >= 20020731
	tmp->function  = find_func_name(op_array, cur_opcode, &(tmp->varc), &(tmp->vars[0]) TSRMLS_CC);
#else
	end_opcode = op_array->opcodes + op_array->last + 1;

	while (cur_opcode < end_opcode) {
		int opcode = cur_opcode->opcode;

		if ((opcode == ZEND_DO_FCALL         ||
		     opcode == ZEND_DO_FCALL_BY_NAME ||
		     opcode == ZEND_INCLUDE_OR_EVAL  ||
		     opcode == ZEND_EXT_FCALL_END) && func_nest == 1)
		{
			break;
		}
		if (opcode == ZEND_EXT_FCALL_BEGIN) {
			func_nest++;
			go_back++;
		}
		if (opcode == ZEND_EXT_FCALL_END) {
			func_nest--;
		}

		cur_opcode++;
	}

	switch (cur_opcode->opcode) {
		case ZEND_INCLUDE_OR_EVAL: {
			zval *inc_filename;
			/* Determine type */
			switch (cur_opcode->op2.u.constant.value.lval) {
				case ZEND_INCLUDE_ONCE:
					XFUNC_SET(tmp, XFUNC_NORMAL, "", "include_once");
					break;
				case ZEND_REQUIRE_ONCE:
					XFUNC_SET(tmp, XFUNC_NORMAL, "", "require_once");
					break;
				case ZEND_INCLUDE:
					XFUNC_SET(tmp, XFUNC_NORMAL, "", "include");
					break;
				case ZEND_REQUIRE:
					XFUNC_SET(tmp, XFUNC_NORMAL, "", "require");
					break;
				case ZEND_EVAL:
					XFUNC_SET(tmp, XFUNC_NORMAL, "", "eval");
					break;
			}
			if (cur_opcode->op1.op_type == IS_CONST) {
				tmp->vars[tmp->varc].name = xdstrdup ("");
				tmp->vars[tmp->varc].value = xdstrdup (cur_opcode->op1.u.constant.value.str.val);
				tmp->varc++;
				tmp->delayed_include = 0;
			} else {
				tmp->delayed_include = 1;
			}
			break;
		}

		case ZEND_DO_FCALL: {
			zend_function *zfunc;

			switch (cur_opcode->op1.op_type) {
				case IS_CONST:
					XFUNC_SET(tmp, XFUNC_NORMAL, "", cur_opcode->op1.u.constant.value.str.val);
					break;
				default:
					XFUNC_SET_DELAYED_C(tmp, XFUNC_MEMBER, cur_opcode->op1.u.constant.value.str.val);
					break;
			}
			break;
		}
		case ZEND_DO_FCALL_BY_NAME: {
			zend_op* tmpOpCode;

			tmpOpCode = cur_opcode;
			while (tmpOpCode->opcode != ZEND_INIT_FCALL_BY_NAME || go_back != 0) {
				tmpOpCode--;
				if (tmpOpCode->opcode == ZEND_INIT_FCALL_BY_NAME && go_back > 0) {
					go_back--;
				}
			}
			switch (tmpOpCode->op1.op_type)  {
				case IS_UNUSED:
					switch (tmpOpCode->op2.op_type) {
						case IS_CONST:
							sprintf(buffer, "%s",
								tmpOpCode->op2.u.constant.value.str.val
							);
							XFUNC_SET(tmp, XFUNC_NORMAL, "", buffer);
							break;
						default:
#if HAVE_EXECUTE_DATA_PTR
							XFUNC_SET_DELAYED_F(tmp, XFUNC_NORMAL, "");
#else
							XFUNC_SET(tmp, XFUNC_NORMAL, "", "{unknown}");
#endif
							break;

					}
					break;
				case IS_CONST:
					switch (tmpOpCode->op2.op_type) {
						case IS_CONST:
							XFUNC_SET(
								tmp, XFUNC_STATIC_MEMBER,
								tmpOpCode->op1.u.constant.value.str.val,
								tmpOpCode->op2.u.constant.value.str.val
							);
							break;
						default:  /* FIXME need better IS_VAR handling */
							XFUNC_SET_DELAYED_F(
								tmp, XFUNC_STATIC_MEMBER,
								tmpOpCode->op1.u.constant.value.str.val
							);
							break;

					}
					break;
				case IS_VAR:
					if (tmpOpCode->op1.op_type == IS_CONST)   {
						switch(tmpOpCode->op2.op_type) {
							case IS_CONST:
								XFUNC_SET(
									tmp, XFUNC_MEMBER,
									tmpOpCode->op1.u.constant.value.str.val,
									tmpOpCode->op2.u.constant.value.str.val
								);
								break;
							default:
								XFUNC_SET_DELAYED_F(
									tmp, XFUNC_MEMBER,
									tmpOpCode->op1.u.constant.value.str.val
								);
								break;
						}
					}
					else {
						switch (tmpOpCode->op2.op_type) {
							case IS_CONST:
								XFUNC_SET_DELAYED_C(
									tmp, XFUNC_MEMBER,
									tmpOpCode->op2.u.constant.value.str.val
								);
								break;
							default:
								XFUNC_SET_DELAYED_F(
									tmp, XFUNC_MEMBER,
									"{unknown}"
								);
								break;
						}
					}
				}
			}
			break;
	}
#endif

	tmp->filename  = op_array->filename ? xdstrdup(op_array->filename): NULL;
	tmp->lineno    = cur_opcode->lineno;
	
	xdebug_llist_insert_next (XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), tmp);
	if (XG(do_trace)) {
		/* Set timestamp and memory footprint */
#if MEMORY_LIMIT
		tmp->memory = AG(allocated_memory);
#else
		tmp->memory = 0;
#endif
		tmp->time   = get_utime();

		/* Update refcount and add into hash */
		tmp->refcount++;
		xdebug_llist_insert_next (XG(trace), XDEBUG_LLIST_TAIL(XG(trace)), tmp);

		if (XG(trace_file)) {
			int j = 0;
			int c = 0;
			char *tmp_name;

			for (j = 1; j < tmp->level; j++) {
				fprintf (XG(trace_file), "  ");
			}
			tmp_name = show_fname(tmp TSRMLS_CC);
			fprintf (XG(trace_file), "-> %s(", tmp_name);
			xdfree (tmp_name);

			fprintf (XG(trace_file), ") %s:%d\n", tmp->filename, tmp->lineno);
			fflush (XG(trace_file));
		}
	}

	XG(level)++;
	if (XG(level) == XG(max_nesting_level)) {
		php_error (E_ERROR, "Maximum function nesting level (%d) reached, possible infinite recursion", XG(max_nesting_level));
	}
}

ZEND_DLEXPORT void xdebug_function_end (zend_op_array *op_array)
{
	TSRMLS_FETCH();

	xdebug_llist_remove (XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), stack_element_dtor);
	XG(level)--;
}

ZEND_DLEXPORT void xdebug_statement_call (zend_op_array *op_array)
{
	function_stack_entry *fse;
	zend_op              *tmp_op;
	zval                **param;
	unsigned int          i;
	TSRMLS_FETCH();

	if (XG(stack)->size == 0) {
		return;
	}
	fse = XDEBUG_LLIST_TAIL(XG(stack))->ptr;

	if (fse->delayed_fname) { /* variable function name */
#if HAVE_EXECUTE_DATA_PTR
		fse->function.function = xdstrdup (EG(current_execute_data)->function_state.function->common.function_name);
#else
		fse->function.function = xdstrdup ("{unknown}");
#endif
	}
	fse->delayed_fname = 0;

	if (fse->delayed_cname) { /* variable class name */
		if (((zval*) EG(active_symbol_table)->pListHead->pDataPtr)->type == IS_OBJECT) {
			fse->function.class = xdstrdup (((zval*) EG(active_symbol_table)->pListHead->pDataPtr)->value.obj.ce->name);
		}
	}
	fse->delayed_cname = 0;


	if (!fse->arg_done) { /* start scanning for REC_VARS */

		fse->varc = 0;

		tmp_op = op_array->opcodes;
		for (i = 0; i < op_array->size; i++) {
			/* Check if optype = RECV_VAR */
			if (tmp_op->opcode == ZEND_RECV) {
				fse->vars[fse->varc].name  = xdstrdup ((tmp_op - 1)->op1.u.constant.value.str.val);
				if (zend_ptr_stack_get_arg(fse->varc + 1, (void**) &param TSRMLS_CC) == SUCCESS) {
					fse->vars[fse->varc].value = get_zval_value(*param);
				} else {
					fse->vars[fse->varc].value = xdstrdup ("{missing}");
				}
				fse->varc++;
			}
			tmp_op++;
		}
		fse->arg_done = 1;
	}
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
#define ZEND_EXT_API    ZEND_DLEXPORT
#endif
ZEND_EXTENSION();

ZEND_DLEXPORT zend_extension zend_extension_entry = {
	"eXtended Debugger (xdebug)",
	XDEBUG_VERSION,
	"Derick Rethans",
	"http://xdebug.derickrethans.nl/",
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
