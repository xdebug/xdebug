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

#include "php.h"
#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "zend.h"
#include "zend_extensions.h"

#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_mm.h"
#include "xdebug_var.h"
#include "xdebug_xml.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

char *error_type(int type)
{
	switch (type) {
		case E_ERROR:
		case E_CORE_ERROR:
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			return xdstrdup("Fatal error");
			break;
		case E_WARNING:
		case E_CORE_WARNING:
		case E_COMPILE_WARNING:
		case E_USER_WARNING:
			return xdstrdup("Warning");
			break;
		case E_PARSE:
			return xdstrdup("Parse error");
			break;
		case E_NOTICE:
		case E_USER_NOTICE:
			return xdstrdup("Notice");
			break;
#ifdef ZEND_ENGINE_2
		case E_STRICT:
			return xdstrdup("Strict standards");
			break;
#endif
		default:
			return xdstrdup("Unknown error");
			break;
	}
}

/*****************************************************************************
** PHP Variable related utility functions
*/
zval* xdebug_get_php_symbol(char* name, int name_length)
{
	HashTable           *st = NULL;
	zval               **retval;
	TSRMLS_FETCH();

	st = XG(active_symbol_table);
	if (st && st->nNumOfElements && zend_hash_find(st, name, name_length, (void **) &retval) == SUCCESS) {
		return *retval;
	}

	st = EG(active_op_array)->static_variables;
	if (st) {
		if (zend_hash_find(st, name, name_length, (void **) &retval) == SUCCESS) {
			return *retval;
		}
	}
	
	st = &EG(symbol_table);
	if (zend_hash_find(st, name, name_length, (void **) &retval) == SUCCESS) {
		return *retval;
	}
	return NULL;
}

static char* xdebug_get_property_info(char *mangled_property, char **property_name)
{
#ifdef ZEND_ENGINE_2
	char *prop_name, *class_name;

	zend_unmangle_property_name(mangled_property, &class_name, &prop_name);
	*property_name = prop_name;
	if (class_name) {
		if (class_name[0] == '*') {
			return "protected";
		} else {
			return "private";
		}
	} else {
		return "public";
	}
#else
	*property_name = mangled_property;
	return "var";
#endif
}


/*****************************************************************************
** Normal variable printing routines
*/

static int xdebug_array_element_export(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_str *str;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	str   = va_arg(args, struct xdebug_str*);

	if (hash_key->nKeyLength==0) { /* numeric key */
		xdebug_str_add(str, xdebug_sprintf("%ld => ", hash_key->h), 1);
	} else { /* string key */
		xdebug_str_add(str, xdebug_sprintf("'%s' => ", hash_key->arKey), 1);
	}
	xdebug_var_export(zv, str, level + 2 TSRMLS_CC);
	xdebug_str_addl(str, ", ", 2, 0);
	return 0;
}

static int xdebug_object_element_export(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_str *str;
	char *prop_name, *modifier;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	str   = va_arg(args, struct xdebug_str*);

	if (hash_key->nKeyLength != 0) {
		modifier = xdebug_get_property_info(hash_key->arKey, &prop_name);
		xdebug_str_add(str, xdebug_sprintf("%s $%s = ", modifier, prop_name), 1);
	}
	xdebug_var_export(zv, str, level + 2 TSRMLS_CC);
	xdebug_str_addl(str, "; ", 2, 0);
	return 0;
}

void xdebug_var_export(zval **struc, xdebug_str *str, int level TSRMLS_DC)
{
	HashTable *myht;
	char*     tmp_str;
	int       tmp_len;

	switch (Z_TYPE_PP(struc)) {
		case IS_BOOL:
			xdebug_str_add(str, xdebug_sprintf("%s", Z_LVAL_PP(struc) ? "TRUE" : "FALSE"), 1);
			break;

		case IS_NULL:
			xdebug_str_addl(str, "NULL", 4, 0);
			break;

		case IS_LONG:
			xdebug_str_add(str, xdebug_sprintf("%ld", Z_LVAL_PP(struc)), 1);
			break;

		case IS_DOUBLE:
			xdebug_str_add(str, xdebug_sprintf("%.*G", (int) EG(precision), Z_DVAL_PP(struc)), 1);
			break;

		case IS_STRING:
			tmp_str = php_addcslashes(Z_STRVAL_PP(struc), Z_STRLEN_PP(struc), &tmp_len, 0, "'\\", 2 TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("'%s'", tmp_str), 1);
			efree(tmp_str);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			if (myht->nApplyCount < 1) {
				xdebug_str_addl(str, "array (", 7, 0);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_array_element_export, 2, level, str);
				if (myht->nNumOfElements > 0) {
					xdebug_str_chop(str, 2);
				}
				xdebug_str_addl(str, ")", 1, 0);
			} else {
				xdebug_str_addl(str, "...", 3, 0);
			}
			break;

		case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);
			if (myht->nApplyCount < 1) {
				xdebug_str_add(str, xdebug_sprintf("class %s { ", Z_OBJCE_PP(struc)->name), 1);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_object_element_export, 2, level, str);
				if (myht->nNumOfElements > 0) {
					xdebug_str_chop(str, 2);
				}
				xdebug_str_addl(str, " }", 2, 0);
			} else {
				xdebug_str_addl(str, "...", 3, 0);
			}
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("resource(%ld) of type (%s)", Z_LVAL_PP(struc), type_name ? type_name : "Unknown"), 1);
			break;
		}

		default:
			xdebug_str_addl(str, "NULL", 4, 0);
			break;
	}
}

char* get_zval_value(zval *val)
{
	xdebug_str str = {0, 0, NULL};
	TSRMLS_FETCH();

	xdebug_var_export(&val, (xdebug_str*) &str, 1 TSRMLS_CC);

	return str.d;
}

/*****************************************************************************
** XML variable printing routines
*/

static int xdebug_array_element_export_xml(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_str *str;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	str   = va_arg(args, struct xdebug_str*);

	xdebug_str_addl(str, "<var", 4, 0);
	if (hash_key->nKeyLength == 0) { /* numeric key */
		xdebug_str_add(str, xdebug_sprintf(" name='%ld'", hash_key->h), 1);
	} else { /* string key */
		xdebug_str_add(str, xdebug_sprintf(" name='%s'", hash_key->arKey), 1);
	}
	xdebug_str_add(str, xdebug_sprintf(" id='%p'>", *zv), 1);
	xdebug_var_export_xml(zv, str, level + 2 TSRMLS_CC);
	xdebug_str_addl(str, "</var>", 6, 0);
	return 0;
}

static int xdebug_object_element_export_xml(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_str *str;
	char *prop_name, *modifier;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	str   = va_arg(args, struct xdebug_str*);

	xdebug_str_addl(str, "<var", 4, 0);
	if (hash_key->nKeyLength != 0) {
		modifier = xdebug_get_property_info(hash_key->arKey, &prop_name);
		xdebug_str_add(str, xdebug_sprintf(" name='%s' facet='%s'", prop_name, modifier), 1);
	}
	xdebug_str_add(str, xdebug_sprintf(" id='%p'>", *zv), 1);
	xdebug_var_export_xml(zv, str, level + 2 TSRMLS_CC);
	xdebug_str_addl(str, "</var>", 6, 0);
	return 0;
}

void xdebug_var_export_xml(zval **struc, xdebug_str *str, int level TSRMLS_DC)
{
	HashTable *myht;
	char*     tmp_str;

	if (!*struc) {
		xdebug_str_addl(str, "<uninitialized/>", 16, 0);
		return;
	}
	
	switch (Z_TYPE_PP(struc)) {
		case IS_BOOL:
			xdebug_str_add(str, xdebug_sprintf("<bool>%s</bool>", Z_LVAL_PP(struc) ? "1" : "0"), 1);
			break;

		case IS_NULL:
			xdebug_str_addl(str, "<null/>", 7, 0);
			break;

		case IS_LONG:
			xdebug_str_add(str, xdebug_sprintf("<int>%ld</int>", Z_LVAL_PP(struc)), 1);
			break;

		case IS_DOUBLE:
			xdebug_str_add(str, xdebug_sprintf("<float>%.*G</float>", (int) EG(precision), Z_DVAL_PP(struc)), 1);
			break;

		case IS_STRING:
			tmp_str = xmlize(Z_STRVAL_PP(struc));
			xdebug_str_add(str, xdebug_sprintf("<string>%s</string>", tmp_str), 1);
			efree(tmp_str);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			if (myht->nApplyCount < 1) {
				xdebug_str_addl(str, "<array>", 7, 0);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_array_element_export_xml, 2, level, str);
				xdebug_str_addl(str, "</array>", 8, 0);
			} else {
				xdebug_str_addl(str, "<array hidden='true' recursive='true'/>", 39, 0);
			}
			break;

		case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);
			if (myht->nApplyCount < 1) {
				xdebug_str_add(str, xdebug_sprintf("<object class='%s'>", Z_OBJCE_PP(struc)->name), 1);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_object_element_export_xml, 2, level, str);
				xdebug_str_addl(str, "</object>", 9, 0);
			} else {
				xdebug_str_addl(str, "<object hidden='true' recursive='true'/>", 40, 0);
			}
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("<resource id='%ld' type='%s'/>", Z_LVAL_PP(struc), type_name ? type_name : "Unknown"), 1);
			break;
		}

		default:
			xdebug_str_addl(str, "<null/>", 7, 0);
			break;
	}
}

char* get_zval_value_xml(char *name, zval *val)
{
	xdebug_str str = {0, 0, NULL};
	TSRMLS_FETCH();

	if (name) {
		xdebug_str_addl(&str, "<var name='", 11, 0);
		xdebug_str_add(&str, name, 0);
		xdebug_str_add(&str, xdebug_sprintf("' id='%p'>", val), 1);
	} else {
		xdebug_str_add(&str, xdebug_sprintf("<var id='%p'>", val), 1);
	}
	
	xdebug_var_export_xml(&val, (xdebug_str*) &str, 1 TSRMLS_CC);

	xdebug_str_addl(&str, "</var>", 7, 0);

	return str.d;
}

/*****************************************************************************
** XML node printing routines
*/

static int xdebug_array_element_export_xml_node(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_xml_node *parent;
	xdebug_xml_node *node;
	char *name = NULL, *parent_name = NULL, *full_name = NULL;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	parent = va_arg(args, xdebug_xml_node*);
	parent_name = va_arg(args, char *);

	node = xdebug_xml_node_init("property");
	
	if (hash_key->nKeyLength != 0) {
		name = xdstrdup(hash_key->arKey);
		if (parent_name[0] != '$') {
			full_name = xdebug_sprintf("$%s['%s']", parent_name, name);
		} else {
			full_name = xdebug_sprintf("%s['%s']", parent_name, name);
		}
	} else {
		name = xdebug_sprintf("%ld", hash_key->h);
		if (parent_name[0] != '$') {
			full_name = xdebug_sprintf("$%s[%s]", parent_name, name);
		} else {
			full_name = xdebug_sprintf("%s[%s]", parent_name, name);
		}
	}

	xdebug_xml_add_attribute_ex(node, "name", name, 0, 1);
	xdebug_xml_add_attribute_ex(node, "fullname", full_name, 0, 1);
	xdebug_xml_add_attribute_ex(node, "address", xdebug_sprintf("%ld", (long) *zv), 0, 1);

	xdebug_xml_add_child(parent, node);
	xdebug_var_export_xml_node(zv, full_name, node, level + 2 TSRMLS_CC);
	return 0;
}

static int xdebug_object_element_export_xml_node(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_xml_node *parent;
	xdebug_xml_node *node;
	char *prop_name, *modifier;
	char *parent_name = NULL, *full_name = NULL;
	TSRMLS_FETCH();

	level  = va_arg(args, int);
	parent = va_arg(args, xdebug_xml_node*);
	full_name = parent_name = va_arg(args, char *);

	node = xdebug_xml_node_init("property");
	
	if (hash_key->nKeyLength != 0) {
		modifier = xdebug_get_property_info(hash_key->arKey, &prop_name);
		xdebug_xml_add_attribute(node, "name", prop_name);
		/* XXX static vars? */
		if (parent_name[0] != '$') {
			full_name = xdebug_sprintf("$%s->%s", parent_name, prop_name);
		} else {
			full_name = xdebug_sprintf("%s->%s", parent_name, prop_name);
		}
		xdebug_xml_add_attribute_ex(node, "fullname", full_name, 0, 1);
		xdebug_xml_add_attribute(node, "facet", modifier);
	}
	xdebug_xml_add_attribute_ex(node, "address", xdebug_sprintf("%ld", (long) *zv), 0, 1);

	xdebug_xml_add_child(parent, node);
	xdebug_var_export_xml_node(zv, full_name, node, level + 2 TSRMLS_CC);
	return 0;
}

void xdebug_var_export_xml_node(zval **struc, char *name, xdebug_xml_node *node, int level TSRMLS_DC)
{
	HashTable *myht;

	switch (Z_TYPE_PP(struc)) {
		case IS_BOOL:
			xdebug_xml_add_attribute(node, "type", "bool");
			xdebug_xml_add_text(node, xdebug_sprintf("%d", Z_LVAL_PP(struc)));
			break;

		case IS_NULL:
			xdebug_xml_add_attribute(node, "type", "null");
			break;

		case IS_LONG:
			xdebug_xml_add_attribute(node, "type", "int");
			xdebug_xml_add_text(node, xdebug_sprintf("%ld", Z_LVAL_PP(struc)));
			break;

		case IS_DOUBLE:
			xdebug_xml_add_attribute(node, "type", "float");
			xdebug_xml_add_text(node, xdebug_sprintf("%.*G", (int) EG(precision), Z_DVAL_PP(struc)));
			break;

		case IS_STRING:
			xdebug_xml_add_attribute(node, "type", "string");
			xdebug_xml_add_text_encode(node, xdstrdup(Z_STRVAL_PP(struc)));
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			xdebug_xml_add_attribute(node, "type", "array");
			xdebug_xml_add_attribute(node, "children", myht->nNumOfElements > 0?"1":"0");
			if (myht->nApplyCount < 1) {
				xdebug_xml_add_attribute_ex(node, "numchildren", xdebug_sprintf("%d", myht->nNumOfElements), 0, 1);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_array_element_export_xml_node, 3, level, node, name);
			} else {
				xdebug_xml_add_attribute(node, "recursive", "1");
			}
			break;

		case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);
			xdebug_xml_add_attribute(node, "type", "object");
			xdebug_xml_add_attribute(node, "children", myht->nNumOfElements > 0?"1":"0");
			xdebug_xml_add_attribute_ex(node, "classname", xdstrdup(Z_OBJCE_PP(struc)->name), 0, 1);
			if (myht->nApplyCount < 1) {
				xdebug_xml_add_attribute_ex(node, "numchildren", xdebug_sprintf("%d", myht->nNumOfElements), 0, 1);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_object_element_export_xml_node, 3, level, node, name);
			} else {
				xdebug_xml_add_attribute(node, "recursive", "1");
			}
			break;

		case IS_RESOURCE: {
			char *type_name;

			xdebug_xml_add_attribute(node, "type", "resource");
			type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			xdebug_xml_add_text(node, xdebug_sprintf("resource id='%ld' type='%s'", Z_LVAL_PP(struc), type_name ? type_name : "Unknown"));
			break;
		}

		default:
			xdebug_xml_add_attribute(node, "type", "null");
			break;
	}
}

xdebug_xml_node* get_zval_value_xml_node(char *name, zval *val)
{
	xdebug_xml_node *node;
	char *full_name = NULL;
	TSRMLS_FETCH();

	node = xdebug_xml_node_init("property");
	if (name) {
		if (name[0] != '$') {
			full_name = xdebug_sprintf("$%s", name);
		} else {
			full_name = xdstrdup(name);
		}
		xdebug_xml_add_attribute_ex(node, "name", xdstrdup(name), 0, 1);
		xdebug_xml_add_attribute_ex(node, "fullname", xdstrdup(full_name), 0, 1);
	}
	xdebug_xml_add_attribute_ex(node, "address", xdebug_sprintf("%ld", (long) val), 0, 1);
	xdebug_var_export_xml_node(&val, name, node, 1 TSRMLS_CC);

	return node;
}

/*****************************************************************************
** Fancy variable printing routines
*/

#define BLUE       "#0000ff"
#define RED        "#ff0000"
#define GREEN      "#00bb00"
#define BLUE_GREEN "#00bbbb"
#define PURPLE     "#bb00bb"
#define LGREY      "#999999"
#define DGREY      "#777777"

static int xdebug_array_element_export_fancy(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_str *str;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	str   = va_arg(args, struct xdebug_str*);

	xdebug_str_add(str, xdebug_sprintf("%*s", level * 2, ""), 1);

	if (hash_key->nKeyLength==0) { /* numeric key */
		xdebug_str_add(str, xdebug_sprintf("%ld <font color='%s'>=&gt;</font> ", hash_key->h, DGREY), 1);
	} else { /* string key */
		xdebug_str_add(str, xdebug_sprintf("'%s' <font color='%s'>=&gt;</font> ", hash_key->arKey, DGREY), 1);
	}
	xdebug_var_export_fancy(zv, str, level + 2 TSRMLS_CC);

	return 0;
}

static int xdebug_object_element_export_fancy(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_str *str;
	char *key;
	char *prop_name, *modifier;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	str   = va_arg(args, struct xdebug_str*);

	xdebug_str_add(str, xdebug_sprintf("%*s", level * 2, ""), 1);

	key = hash_key->arKey;
	if (hash_key->nKeyLength != 0) {
		modifier = xdebug_get_property_info(hash_key->arKey, &prop_name);
		xdebug_str_add(str, xdebug_sprintf("<i>%s</i> '%s' <font color='%s'>=&gt;</font> ", modifier, prop_name, DGREY), 1);
	}
	xdebug_var_export_fancy(zv, str, level + 2 TSRMLS_CC);
	return 0;
}

void xdebug_var_export_fancy(zval **struc, xdebug_str *str, int level TSRMLS_DC)
{
	HashTable *myht;
	char*     tmp_str;

	switch (Z_TYPE_PP(struc)) {
		case IS_BOOL:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>%s</font>", BLUE, Z_LVAL_PP(struc) ? "true" : "false"), 1);
			break;

		case IS_NULL:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>null</font>", RED), 1);
			break;

		case IS_LONG:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>%ld</font>", GREEN, Z_LVAL_PP(struc)), 1);
			break;

		case IS_DOUBLE:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>%.*G</font>", BLUE_GREEN, (int) EG(precision), Z_DVAL_PP(struc)), 1);
			break;

		case IS_STRING:
			tmp_str = xmlize(Z_STRVAL_PP(struc));
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>'%s'</font>", PURPLE, tmp_str), 1);
			efree(tmp_str);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			xdebug_str_add(str, xdebug_sprintf("\n%*s", (level - 1) * 2, ""), 1);
			if (myht->nApplyCount < 1) {
				xdebug_str_addl(str, "<b>array</b>\n", 13, 0);
				if (myht->nNumOfElements) {
					zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_array_element_export_fancy, 2, level, str);
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s", level * 2, ""), 1);
					xdebug_str_add(str, xdebug_sprintf("<i><font color='%s'>empty</font></i>\n", LGREY), 1);
				}
			} else {
				xdebug_str_addl(str, "<i>&</i><b>array</b>\n", 21, 0);
			}
			break;

		case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);
			xdebug_str_add(str, xdebug_sprintf("\n%*s", (level - 1) * 2, ""), 1);
			if (myht->nApplyCount < 1) {
				xdebug_str_add(str, xdebug_sprintf("<b>object</b>(<i>%s</i>)", Z_OBJCE_PP(struc)->name), 1);
#ifdef ZEND_ENGINE_2
				xdebug_str_add(str, xdebug_sprintf("[<i>%d</i>]\n", Z_OBJ_HANDLE_PP(struc)), 1);
#else
				xdebug_str_addl(str, "\n", 1, 0);
#endif
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_object_element_export_fancy, 2, level, str);
			} else {
				xdebug_str_addl(str, "<i>&</i><b>object</b>", 21, 0);
#ifdef ZEND_ENGINE_2
				xdebug_str_add(str, xdebug_sprintf("[<i>%d</i>]\n", Z_OBJ_HANDLE_PP(struc)), 1);
#else
				xdebug_str_addl(str, "\n", 1, 0);
#endif
			}
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("<b>resource</b>(<i>%ld</i><font color='%s'>,</font> <i>%s</i>)", Z_LVAL_PP(struc), DGREY, type_name ? type_name : "Unknown"), 1);
			break;
		}

		default:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>null</font>", RED), 0);
			break;
	}
	if (Z_TYPE_PP(struc) != IS_ARRAY && Z_TYPE_PP(struc) != IS_OBJECT) {
		xdebug_str_addl(str, "\n", 1, 0);
	}
}

char* get_zval_value_fancy(char *name, zval *val TSRMLS_DC)
{
	xdebug_str str = {0, 0, NULL};

	xdebug_str_addl(&str, "<pre>", 5, 0);
	xdebug_var_export_fancy(&val, (xdebug_str*) &str, 1 TSRMLS_CC);
	xdebug_str_addl(&str, "</pre>", 6, 0);

	return str.d;
}

/*****************************************************************************
** XML encoding function
*/

char* xmlize(char *string)
{
	int   len = strlen(string);
	char *tmp;
	char *tmp2;

	if (strlen(string)) {
		tmp = php_str_to_str(string, len, "&", 1, "&amp;", 5, &len);

		tmp2 = php_str_to_str(tmp, len, ">", 1, "&gt;", 4, &len);
		efree(tmp);

		tmp = php_str_to_str(tmp2, len, "<", 1, "&lt;", 4, &len);
		efree(tmp2);

		tmp2 = php_str_to_str(tmp, len, "\n", 1, "&#10;", 5, &len);
		efree(tmp);
		return tmp2;
	} else {
		return estrdup(string);
	}
}

/*****************************************************************************
** Function name printing function
*/

char* show_fname(xdebug_func f, int html, int flags TSRMLS_DC)
{
	char *tmp;

	switch (f.type) {
		case XFUNC_NORMAL: {
			zend_function *zfunc;

			if (PG(html_errors) && zend_hash_find(EG(function_table), f.function, strlen(f.function) + 1, (void**) &zfunc) == SUCCESS) {
				if (html && zfunc->type == ZEND_INTERNAL_FUNCTION) {
					return xdebug_sprintf("<a href='%s/%s' target='_new'>%s</a>\n", XG(manual_url), f.function, f.function);
				} else {
					return xdstrdup(f.function);
				}
			} else {
				return xdstrdup(f.function);
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
			tmp = xdmalloc(strlen(f.class) + 4 + 1);
			sprintf(tmp, "new %s", f.class);
			return tmp;
			break;

		case XFUNC_STATIC_MEMBER:
			if (!f.class) {
				f.class = "?";
			}
			if (!f.function) {
				f.function = "?";
			}
			tmp = xdmalloc(strlen(f.function) + strlen(f.class) + 2 + 1);
			sprintf(tmp, "%s::%s", f.class, f.function);
			return tmp;
			break;

		case XFUNC_MEMBER:
			if (!f.class) {
				f.class = "?";
			}
			if (!f.function) {
				f.function = "?";
			}
			tmp = xdmalloc(strlen(f.function) + strlen(f.class) + 2 + 1);
			sprintf(tmp, "%s->%s", f.class, f.function);
			return tmp;
			break;

		case XFUNC_EVAL:
			return xdstrdup("eval");
			break;

		case XFUNC_INCLUDE:
			return xdstrdup("include");
			break;

		case XFUNC_INCLUDE_ONCE:
			return xdstrdup("include_once");
			break;

		case XFUNC_REQUIRE:
			return xdstrdup("require");
			break;

		case XFUNC_REQUIRE_ONCE:
			return xdstrdup("require_once");
			break;

		default:
			return xdstrdup("{unknown}");
	}
}
