/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 Derick Rethans                              |
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

#include "zend.h"
#include "php_xdebug.h"
#include "xdebug_str.h"
#include "xdebug_xml.h"

#ifndef __HAVE_XDEBUG_VAR_H__
#define __HAVE_XDEBUG_VAR_H__

zval* xdebug_get_php_symbol(char* name, int name_length);

void xdebug_var_export(zval **struc, xdebug_str *str, int level TSRMLS_DC);
void xdebug_var_export_xml(zval **struc, xdebug_str *str, int level TSRMLS_DC);
void xdebug_var_export_fancy(zval **struc, xdebug_str *str, int level TSRMLS_DC);
void xdebug_var_export_xml_node(zval **struc, char *name, xdebug_xml_node *node, int level TSRMLS_DC);

char* xmlize(char *string);
char* error_type (int type);
char* get_zval_value (zval *val);
char* get_zval_value_xml (char *name, zval *val);
char* get_zval_value_fancy(char *name, zval *val TSRMLS_DC);
xdebug_xml_node* get_zval_value_xml_node (char *name, zval *val);
char* show_fname (xdebug_func t, int html, int flags TSRMLS_DC);

#endif
