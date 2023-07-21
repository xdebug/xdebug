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
#ifndef XDEBUG_TRACING_H
#define XDEBUG_TRACING_H

#include "lib/php-header.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#include "zend_generators.h"
#pragma GCC diagnostic pop

typedef struct
{
	void *(*init)(char *fname, zend_string *script_filename, long options);
	void (*deinit)(void *ctxt);
	void (*write_header)(void *ctxt);
	void (*write_footer)(void *ctxt);
	char *(*get_filename)(void *ctxt);
	void (*function_entry)(void *ctxt, function_stack_entry *fse, int function_nr);
	void (*function_exit)(void *ctxt, function_stack_entry *fse, int function_nr);
	void (*return_value)(void *ctxt, function_stack_entry *fse, int function_nr, zval *return_value);
	void (*generator_return_value)(void *ctxt, function_stack_entry *fse, int function_nr, zend_generator *generator);
	void (*assignment)(void *ctxt, function_stack_entry *fse, char *full_varname, zval *value, char *right_full_varname, const char *op, char *file, int lineno);
} xdebug_trace_handler_t;

typedef struct _xdebug_tracing_globals_t {
	xdebug_trace_handler_t *trace_handler;
	void                   *trace_context;
} xdebug_tracing_globals_t;

typedef struct _xdebug_tracing_settings_t {
	char         *trace_output_name;
	zend_long     trace_options;
	zend_long     trace_format;

	zend_bool     collect_assignments;
	zend_bool     collect_return;
} xdebug_tracing_settings_t;

void xdebug_init_tracing_globals(xdebug_tracing_globals_t *xg);
void xdebug_tracing_minit(INIT_FUNC_ARGS);
void xdebug_tracing_rinit(void);
void xdebug_tracing_post_deactivate(void);
void xdebug_tracing_register_constants(INIT_FUNC_ARGS);

void xdebug_tracing_init_if_requested(zend_op_array *op_array);
void xdebug_tracing_execute_ex(int function_nr, function_stack_entry *fse);
void xdebug_tracing_execute_ex_end(int function_nr, function_stack_entry *fse, zend_execute_data *execute_data);
int xdebug_tracing_execute_internal(int function_nr, function_stack_entry *fse);
void xdebug_tracing_execute_internal_end(int function_nr, function_stack_entry *fse, zval *return_value);

void xdebug_tracing_save_trace_context(void **old_trace_context);
void xdebug_tracing_restore_trace_context(void *old_trace_context);

char* xdebug_return_trace_stack_retval(function_stack_entry* i, int fnr, zval* retval);
char* xdebug_return_trace_stack_generator_retval(function_stack_entry* i, zend_generator* generator);
char* xdebug_return_trace_assignment(function_stack_entry *i, char *varname, zval *retval, char *op, char *file, int fileno);

void xdebug_trace_function_begin(function_stack_entry *fse, int function_nr);
void xdebug_trace_function_end(function_stack_entry *fse, int function_nr);

char *xdebug_get_trace_filename(void);
#endif
