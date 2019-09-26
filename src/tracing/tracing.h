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
#ifndef XDEBUG_TRACING_H
#define XDEBUG_TRACING_H

#include "php.h"
#include "php_xdebug.h"

char* xdebug_return_trace_stack_retval(function_stack_entry* i, int fnr, zval* retval TSRMLS_DC);
char* xdebug_return_trace_stack_generator_retval(function_stack_entry* i, zend_generator* generator TSRMLS_DC);
char* xdebug_return_trace_assignment(function_stack_entry *i, char *varname, zval *retval, char *op, char *file, int fileno TSRMLS_DC);
FILE *xdebug_trace_open_file(char *fname, char *script_filename, long options, char **used_fname TSRMLS_DC);

void xdebug_trace_function_begin(function_stack_entry *fse, int function_nr TSRMLS_DC);
void xdebug_trace_function_end(function_stack_entry *fse, int function_nr TSRMLS_DC);

#endif
