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

#ifndef XDEBUG_STACK_H
#define XDEBUG_STACK_H

#include "xdebug_str.h"

#define XDEBUG_STACK_NO_DESC 0x01

void xdebug_build_fname(xdebug_func *tmp, zend_execute_data *edata TSRMLS_DC);
function_stack_entry *xdebug_add_stack_frame(zend_execute_data *zdata, zend_op_array *op_array, int type TSRMLS_DC);
void xdebug_append_error_head(xdebug_str *str, int html, const char *error_type_str TSRMLS_DC);
void xdebug_append_error_description(xdebug_str *str, int html, const char *error_type_str, const char *buffer, const char *error_filename, const int error_lineno TSRMLS_DC);
void xdebug_append_printable_stack(xdebug_str *str, int html TSRMLS_DC);
void xdebug_append_error_footer(xdebug_str *str, int html TSRMLS_DC);
void xdebug_log_stack(const char *error_type_str, char *buffer, const char *error_filename, const int error_lineno TSRMLS_DC);
char *xdebug_strip_php_stack_trace(char *buffer);
char *xdebug_handle_stack_trace(int type, char *error_type_str, const char *error_filename, const uint error_lineno, char *buffer TSRMLS_DC);
void xdebug_do_jit(TSRMLS_D);
int xdebug_handle_hit_value(xdebug_brk_info *brk_info);

#endif
