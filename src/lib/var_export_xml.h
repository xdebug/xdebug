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

#ifndef __HAVE_XDEBUG_LIB_VAR_EXPORT_XML_H__
#define __HAVE_XDEBUG_LIB_VAR_EXPORT_XML_H__

#include "var.h"
#include "xml.h"

void xdebug_var_xml_attach_static_vars(xdebug_xml_node *node, xdebug_var_export_options *options, zend_class_entry *ce);
void xdebug_var_xml_attach_static_var_with_contents(zval **zv, int num_args, va_list args, zend_hash_key *hash_key);
void xdebug_var_xml_attach_uninitialized_var(xdebug_var_export_options *options, xdebug_xml_node *node, xdebug_str *name);

#define xdebug_get_zval_value_xml_node(name, val, options) xdebug_get_zval_value_xml_node_ex(name, val, XDEBUG_VAR_TYPE_NORMAL, options)
xdebug_xml_node* xdebug_get_zval_value_xml_node_ex(xdebug_str *name, zval *val, int var_type, xdebug_var_export_options *options);

void xdebug_var_export_xml_node(zval **struc, xdebug_str *name, xdebug_xml_node *node, xdebug_var_export_options *options, int level);

#endif
