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

#ifndef XDEBUG_STACK_H
#define XDEBUG_STACK_H

#include "lib/str.h"

#define XDEBUG_STACK_NO_DESC 0x01

int xdebug_function_name_is_closure(char *fname);
char* xdebug_wrap_closure_location_around_function_name(zend_op_array *opa, char *fname);
void xdebug_func_dtor_by_ref(xdebug_func *elem); /* TODO: Remove this API */
void xdebug_func_dtor(xdebug_func *elem);
void xdebug_build_fname(xdebug_func *tmp, zend_execute_data *edata);
void xdebug_build_fname_from_oparray(xdebug_func *tmp, zend_op_array *opa);
function_stack_entry *xdebug_add_stack_frame(zend_execute_data *zdata, zend_op_array *op_array, int type);
void xdebug_append_error_head(xdebug_str *str, int html, const char *error_type_str);
void xdebug_append_error_description(xdebug_str *str, int html, const char *error_type_str, const char *buffer, const char *error_filename, const int error_lineno);
void xdebug_append_printable_stack(xdebug_str *str, int html);
void xdebug_append_error_footer(xdebug_str *str, int html);
void xdebug_log_stack(const char *error_type_str, char *buffer, const char *error_filename, const int error_lineno);
char *xdebug_strip_php_stack_trace(char *buffer);

#endif
