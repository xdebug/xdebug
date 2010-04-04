/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2010 Derick Rethans                               |
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
#include "xdebug_compat.h"
#include "xdebug_private.h"
#include "xdebug_mm.h"
#include "xdebug_var.h"
#include "xdebug_xml.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

char* xdebug_error_type(int type)
{
	switch (type) {
		case E_ERROR:
		case E_CORE_ERROR:
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			return xdstrdup("Fatal error");
			break;
#if PHP_VERSION_ID >= 50200
		case E_RECOVERABLE_ERROR:
			return xdstrdup("Catchable fatal error");
			break;
#endif
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
		case E_STRICT:
			return xdstrdup("Strict standards");
			break;
#if PHP_VERSION_ID >= 50300
		case E_DEPRECATED:
			return xdstrdup("Deprecated");
			break;
#endif
		default:
			return xdstrdup("Unknown error");
			break;
	}
}

/*************************************************************************************************************************************/
#define T(offset) (*(temp_variable *)((char *) Ts + offset))

zval *xdebug_get_zval(zend_execute_data *zdata, znode *node, temp_variable *Ts, int *is_var)
{
	switch (node->op_type) {
		case IS_CONST:
			return &node->u.constant;
			break;

		case IS_TMP_VAR:
			*is_var = 1;
			return &T(node->u.var).tmp_var;
			break;

		case IS_VAR:
			*is_var = 1;
			if (T(node->u.var).var.ptr) {
				return T(node->u.var).var.ptr;
			} else {
				fprintf(stderr, "\nIS_VAR\n");
			}
			break;

		case IS_CV: {
			zval **tmp;
			tmp = zend_get_compiled_variable_value(zdata, node->u.constant.value.lval);
			if (tmp) {
				return *tmp;
			}
			break;
		}

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

static char* xdebug_get_property_info(char *mangled_property, int mangled_len, char **property_name, char **class_name)
{
	char *prop_name, *cls_name;

#if PHP_VERSION_ID >= 50200
	zend_unmangle_property_name(mangled_property, mangled_len - 1, &cls_name, &prop_name);
#else
	zend_unmangle_property_name(mangled_property, &cls_name, &prop_name);
#endif
	*property_name = prop_name;
	*class_name = cls_name;
	if (cls_name) {
		if (cls_name[0] == '*') {
			return "protected";
		} else {
			return "private";
		}
	} else {
		return "public";
	}
}


xdebug_var_export_options* xdebug_var_export_options_from_ini(TSRMLS_D)
{
	xdebug_var_export_options *options;
	options = xdmalloc(sizeof(xdebug_var_export_options));

	options->max_children = XG(display_max_children);
	options->max_data = XG(display_max_data);
	options->max_depth = XG(display_max_depth);
	options->show_hidden = 0;

	if (options->max_children == -1) {
		options->max_children = 1048576;
	} else if (options->max_children < 1) {
		options->max_children = 1;
	}

	if (options->max_data == -1) {
		options->max_data = 1073741824;
	} else if (options->max_data < 1) {
		options->max_data = 1;
	}

	if (options->max_depth == -1) {
		options->max_depth = 4096;
	} else if (options->max_depth < 0) {
		options->max_depth = 0;
	}

	options->runtime = (xdebug_var_runtime_page*) xdmalloc((options->max_depth + 1) * sizeof(xdebug_var_runtime_page));
	options->no_decoration = 0;

	return options;
}

xdebug_var_export_options xdebug_var_nolimit_options = { 1048576, 1048576, 64, 1, NULL, 0 };

xdebug_var_export_options* xdebug_var_get_nolimit_options(TSRMLS_D)
{
	return &xdebug_var_nolimit_options;
}

/*****************************************************************************
** Normal variable printing routines
*/

static int xdebug_array_element_export(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, debug_zval;
	xdebug_str *str;
	xdebug_var_export_options *options;
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	TSRMLS_FETCH();
#endif

	level      = va_arg(args, int);
	str        = va_arg(args, struct xdebug_str*);
	debug_zval = va_arg(args, int);
	options    = va_arg(args, xdebug_var_export_options*);

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		if (hash_key->nKeyLength==0) { /* numeric key */
			xdebug_str_add(str, xdebug_sprintf("%ld => ", hash_key->h), 1);
		} else { /* string key */
			int newlen = 0;
			char *tmp, *tmp2;
			
			tmp = php_str_to_str(hash_key->arKey, hash_key->nKeyLength, "'", 1, "\\'", 2, &newlen);
			tmp2 = php_str_to_str(tmp, newlen - 1, "\0", 1, "\\0", 2, &newlen);
			if (tmp) {
				efree(tmp);
			}
			xdebug_str_addl(str, "'", 1, 0);
			if (tmp2) {
				xdebug_str_addl(str, tmp2, newlen, 0);
				efree(tmp2);
			}
			xdebug_str_add(str, "' => ", 0);
		}
		xdebug_var_export(zv, str, level + 2, debug_zval, options TSRMLS_CC);
		xdebug_str_addl(str, ", ", 2, 0);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_addl(str, "..., ", 5, 0);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, debug_zval;
	xdebug_str *str;
	xdebug_var_export_options *options;
	char *prop_name, *class_name, *modifier, *prop_class_name;
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	TSRMLS_FETCH();
#endif

	level      = va_arg(args, int);
	str        = va_arg(args, struct xdebug_str*);
	debug_zval = va_arg(args, int);
	options    = va_arg(args, xdebug_var_export_options*);
	class_name = va_arg(args, char *);

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		if (hash_key->nKeyLength != 0) {
			modifier = xdebug_get_property_info(hash_key->arKey, hash_key->nKeyLength, &prop_name, &prop_class_name);
			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				xdebug_str_add(str, xdebug_sprintf("%s $%s = ", modifier, prop_name), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("%s ${%s}:%s = ", modifier, prop_class_name, prop_name), 1);
			}
		}
		xdebug_var_export(zv, str, level + 2, debug_zval, options TSRMLS_CC);
		xdebug_str_addl(str, "; ", 2, 0);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_addl(str, "...; ", 5, 0);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

void xdebug_var_export(zval **struc, xdebug_str *str, int level, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	HashTable *myht;
	char*     tmp_str;
	int       tmp_len;

	if (!struc || !(*struc)) {
		return;
	}
	if (debug_zval) {
		xdebug_str_add(str, xdebug_sprintf("(refcount=%d, is_ref=%d)=", (*struc)->XDEBUG_REFCOUNT, (*struc)->XDEBUG_IS_REF), 1);
	}
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
			tmp_str = php_addcslashes(Z_STRVAL_PP(struc), Z_STRLEN_PP(struc), &tmp_len, 0, "'\\\0..\37", 6 TSRMLS_CC);
			if (options->no_decoration) {
				xdebug_str_add(str, tmp_str, 0);
			} else if (options->max_data == 0 || Z_STRLEN_PP(struc) <= options->max_data) {
				xdebug_str_add(str, xdebug_sprintf("'%s'", tmp_str), 1);
			} else {
				xdebug_str_addl(str, "'", 1, 0);
				xdebug_str_addl(str, xdebug_sprintf("%s", tmp_str), options->max_data, 1);
				xdebug_str_addl(str, "...'", 4, 0);
			}
			efree(tmp_str);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			if (myht->nApplyCount < 1) {
				xdebug_str_addl(str, "array (", 7, 0);
				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					zend_hash_apply_with_arguments(myht XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_array_element_export, 4, level, str, debug_zval, options);
					/* Remove the ", " at the end of the string */
					if (myht->nNumOfElements > 0) {
						xdebug_str_chop(str, 2);
					}
				} else {
					xdebug_str_addl(str, "...", 3, 0);
				}
				xdebug_str_addl(str, ")", 1, 0);
			} else {
				xdebug_str_addl(str, "...", 3, 0);
			}
			break;

		case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);
			if (myht->nApplyCount < 1) {
				char *class_name;
				zend_uint class_name_len;

				zend_get_object_classname(*struc, &class_name, &class_name_len TSRMLS_CC);
				xdebug_str_add(str, xdebug_sprintf("class %s { ", class_name), 1);

				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					zend_hash_apply_with_arguments(myht XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export, 5, level, str, debug_zval, options, class_name);
					/* Remove the ", " at the end of the string */
					if (myht->nNumOfElements > 0) {
						xdebug_str_chop(str, 2);
					}
				} else {
					xdebug_str_addl(str, "...", 3, 0);
				}
				xdebug_str_addl(str, " }", 2, 0);
				efree(class_name);
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

char* xdebug_get_zval_value(zval *val, int debug_zval, xdebug_var_export_options *options)
{
	xdebug_str str = {0, 0, NULL};
	int default_options = 0;
	TSRMLS_FETCH();

	if (!options) {
		options = xdebug_var_export_options_from_ini(TSRMLS_C);
		default_options = 1;
	}

	xdebug_var_export(&val, (xdebug_str*) &str, 1, debug_zval, options TSRMLS_CC);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	return str.d;
}

static void xdebug_var_synopsis(zval **struc, xdebug_str *str, int level, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	HashTable *myht;

	if (!struc || !(*struc)) {
		return;
	}
	if (debug_zval) {
		xdebug_str_add(str, xdebug_sprintf("(refcount=%d, is_ref=%d)=", (*struc)->XDEBUG_REFCOUNT, (*struc)->XDEBUG_IS_REF), 1);
	}
	switch (Z_TYPE_PP(struc)) {
		case IS_BOOL:
			xdebug_str_addl(str, "bool", 4, 0);
			break;

		case IS_NULL:
			xdebug_str_addl(str, "null", 4, 0);
			break;

		case IS_LONG:
			xdebug_str_addl(str, "long", 4, 0);
			break;

		case IS_DOUBLE:
			xdebug_str_addl(str, "double", 6, 0);
			break;

		case IS_STRING:
			xdebug_str_add(str, xdebug_sprintf("string(%d)", Z_STRLEN_PP(struc)), 1);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			xdebug_str_add(str, xdebug_sprintf("array(%d)", myht->nNumOfElements), 1);
			break;

		case IS_OBJECT: {
			char *class_name;
			zend_uint class_name_len;

			zend_get_object_classname(*struc, &class_name, &class_name_len TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("class %s", class_name), 1);
			break;
		}

		case IS_RESOURCE: {
			char *type_name;

			type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("resource(%ld) of type (%s)", Z_LVAL_PP(struc), type_name ? type_name : "Unknown"), 1);
			break;
		}
	}
}

char* xdebug_get_zval_synopsis(zval *val, int debug_zval, xdebug_var_export_options *options)
{
	xdebug_str str = {0, 0, NULL};
	int default_options = 0;
	TSRMLS_FETCH();

	if (!options) {
		options = xdebug_var_export_options_from_ini(TSRMLS_C);
		default_options = 1;
	}

	xdebug_var_synopsis(&val, (xdebug_str*) &str, 1, debug_zval, options TSRMLS_CC);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	return str.d;
}

/*****************************************************************************
** XML node printing routines
*/

static int xdebug_array_element_export_xml_node(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_xml_node *parent;
	xdebug_xml_node *node;
	xdebug_var_export_options *options;
	char *name = NULL, *parent_name = NULL, *full_name = NULL;
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	TSRMLS_FETCH();
#endif

	level = va_arg(args, int);
	parent = va_arg(args, xdebug_xml_node*);
	parent_name = va_arg(args, char *);
	options = va_arg(args, xdebug_var_export_options*);

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		node = xdebug_xml_node_init("property");
	
		if (hash_key->nKeyLength != 0) {
			name = xdstrdup(hash_key->arKey);
			if (!parent_name) {
				full_name = NULL;
			} else if (parent_name[0] != '$') {
				full_name = xdebug_sprintf("$%s['%s']", parent_name, name);
			} else {
				full_name = xdebug_sprintf("%s['%s']", parent_name, name);
			}
		} else {
			name = xdebug_sprintf("%ld", hash_key->h);
			if (!parent_name) {
				full_name = NULL;
			} else if (parent_name[0] != '$') {
				full_name = xdebug_sprintf("$%s[%s]", parent_name, name);
			} else {
				full_name = xdebug_sprintf("%s[%s]", parent_name, name);
			}
		}

		xdebug_xml_add_attribute_ex(node, "name", name, 0, 1);
		if (full_name) {
			xdebug_xml_add_attribute_ex(node, "fullname", full_name, 0, 1);
		}
		xdebug_xml_add_attribute_ex(node, "address", xdebug_sprintf("%ld", (long) *zv), 0, 1);

		xdebug_xml_add_child(parent, node);
		xdebug_var_export_xml_node(zv, full_name, node, options, level + 1 TSRMLS_CC);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export_xml_node(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_xml_node *parent;
	xdebug_xml_node *node;
	xdebug_var_export_options *options;
	char *prop_name, *modifier, *class_name, *prop_class_name;
	char *parent_name = NULL, *full_name = NULL;
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	TSRMLS_FETCH();
#endif

	level  = va_arg(args, int);
	parent = va_arg(args, xdebug_xml_node*);
	full_name = parent_name = va_arg(args, char *);
	options = va_arg(args, xdebug_var_export_options*);
	class_name = va_arg(args, char *);

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		if (hash_key->nKeyLength != 0) {
			modifier = xdebug_get_property_info(hash_key->arKey, hash_key->nKeyLength, &prop_name, &prop_class_name);
			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				node = xdebug_xml_node_init("property");
				xdebug_xml_add_attribute_ex(node, "name", xdstrdup(prop_name), 0, 1);
			} else {
				return 0;
			}

			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				if (parent_name) {
					if (parent_name[0] != '$') {
						full_name = xdebug_sprintf("$%s->%s", parent_name, prop_name);
					} else {
						full_name = xdebug_sprintf("%s->%s", parent_name, prop_name);
					}
					xdebug_xml_add_attribute_ex(node, "fullname", full_name, 0, 1);
				}
			}
			xdebug_xml_add_attribute(node, "facet", modifier);

			xdebug_xml_add_attribute_ex(node, "address", xdebug_sprintf("%ld", (long) *zv), 0, 1);

			xdebug_xml_add_child(parent, node);

			xdebug_var_export_xml_node(zv, full_name, node, options, level + 1 TSRMLS_CC);
		}
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

void xdebug_var_export_xml_node(zval **struc, char *name, xdebug_xml_node *node, xdebug_var_export_options *options, int level TSRMLS_DC)
{
	HashTable *myht;
	char *class_name;
	zend_uint class_name_len;

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
			if (options->max_data == 0 || Z_STRLEN_PP(struc) <= options->max_data) {
				xdebug_xml_add_text_encodel(node, xdstrndup(Z_STRVAL_PP(struc), Z_STRLEN_PP(struc)), Z_STRLEN_PP(struc));
			} else {
				xdebug_xml_add_text_encodel(node, xdstrndup(Z_STRVAL_PP(struc), options->max_data), options->max_data);
			}
			xdebug_xml_add_attribute_ex(node, "size", xdebug_sprintf("%d", Z_STRLEN_PP(struc)), 0, 1);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			xdebug_xml_add_attribute(node, "type", "array");
			xdebug_xml_add_attribute(node, "children", myht->nNumOfElements > 0?"1":"0");
			if (myht->nApplyCount < 1) {
				xdebug_xml_add_attribute_ex(node, "numchildren", xdebug_sprintf("%d", myht->nNumOfElements), 0, 1);
				if (level < options->max_depth) {
					xdebug_xml_add_attribute_ex(node, "page", xdebug_sprintf("%d", options->runtime[level].page), 0, 1);
					xdebug_xml_add_attribute_ex(node, "pagesize", xdebug_sprintf("%d", options->max_children), 0, 1);
					options->runtime[level].current_element_nr = 0;
					if (level == 0) {
						options->runtime[level].start_element_nr = options->max_children * options->runtime[level].page;
						options->runtime[level].end_element_nr = options->max_children * (options->runtime[level].page + 1);
					} else {
						options->runtime[level].start_element_nr = 0;
						options->runtime[level].end_element_nr = options->max_children;
					}
					zend_hash_apply_with_arguments(myht XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_array_element_export_xml_node, 4, level, node, name, options);
				}
			} else {
				xdebug_xml_add_attribute(node, "recursive", "1");
			}
			break;

		case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);

			xdebug_xml_add_attribute(node, "type", "object");
			zend_get_object_classname(*struc, &class_name, &class_name_len TSRMLS_CC);
			xdebug_xml_add_attribute_ex(node, "classname", xdstrdup(class_name), 0, 1);
			xdebug_xml_add_attribute(node, "children", (myht && zend_hash_num_elements(myht))?"1":"0");

			/** Temporary additional property **/
			{
				xdebug_xml_node *pnode;

				pnode = xdebug_xml_node_init("property");
				xdebug_xml_add_attribute(pnode, "name", "CLASSNAME");
				xdebug_xml_add_attribute(pnode, "type", "string");
				xdebug_xml_add_text(pnode, xdstrdup(class_name));
				xdebug_xml_add_child(node, pnode);
			}

			if (myht) {
				if (myht->nApplyCount < 1) {
					xdebug_xml_add_attribute_ex(node, "numchildren", xdebug_sprintf("%d", zend_hash_num_elements(myht)), 0, 1);
					if (level < options->max_depth) {
						xdebug_xml_add_attribute_ex(node, "page", xdebug_sprintf("%d", options->runtime[level].page), 0, 1);
						xdebug_xml_add_attribute_ex(node, "pagesize", xdebug_sprintf("%d", options->max_children), 0, 1);
						options->runtime[level].current_element_nr = 0;
						if (level == 0) {
							options->runtime[level].start_element_nr = options->max_children * options->runtime[level].page;
							options->runtime[level].end_element_nr = options->max_children * (options->runtime[level].page + 1);
						} else {
							options->runtime[level].start_element_nr = 0;
							options->runtime[level].end_element_nr = options->max_children;
						}
						zend_hash_apply_with_arguments(myht XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export_xml_node, 5, level, node, name, options, class_name);
					}
				} else {
					xdebug_xml_add_attribute(node, "recursive", "1");
				}
			}
			efree(class_name);
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

xdebug_xml_node* xdebug_get_zval_value_xml_node(char *name, zval *val, xdebug_var_export_options *options)
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
		xdebug_xml_add_attribute_ex(node, "fullname", full_name, 0, 1);
	}
	xdebug_xml_add_attribute_ex(node, "address", xdebug_sprintf("%ld", (long) val), 0, 1);
	xdebug_var_export_xml_node(&val, name, node, options, 0 TSRMLS_CC);

	return node;
}

/*****************************************************************************
** Fancy variable printing routines
*/

#define COLOR_POINTER   "#888a85"
#define COLOR_BOOL      "#75507b"
#define COLOR_LONG      "#4e9a06"
#define COLOR_NULL      "#3465a4"
#define COLOR_DOUBLE    "#f57900"
#define COLOR_STRING    "#cc0000"
#define COLOR_EMPTY     "#888a85"
#define COLOR_ARRAY     "#ce5c00"
#define COLOR_OBJECT    "#8f5902"
#define COLOR_RESOURCE  "#2e3436"

static int xdebug_array_element_export_fancy(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, debug_zval, newlen;
	char *tmp_str;
	xdebug_str *str;
	xdebug_var_export_options *options;
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	TSRMLS_FETCH();
#endif

	level      = va_arg(args, int);
	str        = va_arg(args, struct xdebug_str*);
	debug_zval = va_arg(args, int);
	options    = va_arg(args, xdebug_var_export_options*);

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 4) - 2, ""), 1);

		if (hash_key->nKeyLength==0) { /* numeric key */
			xdebug_str_add(str, xdebug_sprintf("%ld <font color='%s'>=&gt;</font> ", hash_key->h, COLOR_POINTER), 1);
		} else { /* string key */
			xdebug_str_addl(str, "'", 1, 0);
			tmp_str = xdebug_xmlize(hash_key->arKey, hash_key->nKeyLength - 1, &newlen);
			xdebug_str_addl(str, tmp_str, newlen, 0);
			efree(tmp_str);
			xdebug_str_add(str, xdebug_sprintf("' <font color='%s'>=&gt;</font> ", COLOR_POINTER), 1);
		}
		xdebug_var_export_fancy(zv, str, level + 1, debug_zval, options TSRMLS_CC);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 4) - 2, ""), 1);
		xdebug_str_addl(str, "<i>more elements...</i>\n", 24, 0);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export_fancy(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, debug_zval;
	xdebug_str *str;
	xdebug_var_export_options *options;
	char *key;
	char *prop_name, *class_name, *modifier, *prop_class_name;
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	TSRMLS_FETCH();
#endif

	level      = va_arg(args, int);
	str        = va_arg(args, struct xdebug_str*);
	debug_zval = va_arg(args, int);
	options    = va_arg(args, xdebug_var_export_options*);
	class_name = va_arg(args, char *);

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 4) - 2, ""), 1);

		key = hash_key->arKey;
		if (hash_key->nKeyLength != 0) {
			modifier = xdebug_get_property_info(hash_key->arKey, hash_key->nKeyLength, &prop_name, &prop_class_name);
			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				xdebug_str_add(str, xdebug_sprintf("<i>%s</i> '%s' <font color='%s'>=&gt;</font> ", modifier, prop_name, COLOR_POINTER), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("<i>%s</i> '%s' <small>(%s)</small> <font color='%s'>=&gt;</font> ", modifier, prop_name, prop_class_name, COLOR_POINTER), 1);
			}
		}
		xdebug_var_export_fancy(zv, str, level + 1, debug_zval, options TSRMLS_CC);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 4) - 2, ""), 1);
		xdebug_str_addl(str, "<i>more elements...</i>\n", 24, 0);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

void xdebug_var_export_fancy(zval **struc, xdebug_str *str, int level, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	HashTable *myht;
	char*     tmp_str;
	int       newlen;

	if (debug_zval) {
		xdebug_str_add(str, xdebug_sprintf("<i>(refcount=%d, is_ref=%d)</i>,", (*struc)->XDEBUG_REFCOUNT, (*struc)->XDEBUG_IS_REF), 1);
	} else {
		if ((*struc)->XDEBUG_IS_REF) {
			xdebug_str_add(str, "&amp;", 0);
		}
	}
	switch (Z_TYPE_PP(struc)) {
		case IS_BOOL:
			xdebug_str_add(str, xdebug_sprintf("<small>boolean</small> <font color='%s'>%s</font>", COLOR_BOOL, Z_LVAL_PP(struc) ? "true" : "false"), 1);
			break;

		case IS_NULL:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>null</font>", COLOR_NULL), 1);
			break;

		case IS_LONG:
			xdebug_str_add(str, xdebug_sprintf("<small>int</small> <font color='%s'>%ld</font>", COLOR_LONG, Z_LVAL_PP(struc)), 1);
			break;

		case IS_DOUBLE:
			xdebug_str_add(str, xdebug_sprintf("<small>float</small> <font color='%s'>%.*G</font>", COLOR_DOUBLE, (int) EG(precision), Z_DVAL_PP(struc)), 1);
			break;

		case IS_STRING:
			xdebug_str_add(str, xdebug_sprintf("<small>string</small> <font color='%s'>'", COLOR_STRING), 1);
			if (Z_STRLEN_PP(struc) > options->max_data) {
				tmp_str = xdebug_xmlize(Z_STRVAL_PP(struc), options->max_data, &newlen);
				xdebug_str_addl(str, tmp_str, newlen, 0);
				efree(tmp_str);
				xdebug_str_addl(str, "'...</font>", 11, 0);
			} else {
				tmp_str = xdebug_xmlize(Z_STRVAL_PP(struc), Z_STRLEN_PP(struc), &newlen);
				xdebug_str_addl(str, tmp_str, newlen, 0);
				efree(tmp_str);
				xdebug_str_addl(str, "'</font>", 8, 0);
			}
			xdebug_str_add(str, xdebug_sprintf(" <i>(length=%d)</i>", Z_STRLEN_PP(struc)), 1);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			xdebug_str_add(str, xdebug_sprintf("\n%*s", (level - 1) * 4, ""), 1);
			if (myht->nApplyCount < 1) {
				xdebug_str_addl(str, "<b>array</b>\n", 13, 0);
				if (level <= options->max_depth) {
					if (myht->nNumOfElements) {
						options->runtime[level].current_element_nr = 0;
						options->runtime[level].start_element_nr = 0;
						options->runtime[level].end_element_nr = options->max_children;

						zend_hash_apply_with_arguments(myht XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_array_element_export_fancy, 4, level, str, debug_zval, options);
					} else {
						xdebug_str_add(str, xdebug_sprintf("%*s", (level * 4) - 2, ""), 1);
						xdebug_str_add(str, xdebug_sprintf("<i><font color='%s'>empty</font></i>\n", COLOR_EMPTY), 1);
					}
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 4) - 2, ""), 1);
				}
			} else {
				xdebug_str_addl(str, "<i>&</i><b>array</b>\n", 21, 0);
			}
			break;

		case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);
			xdebug_str_add(str, xdebug_sprintf("\n%*s", (level - 1) * 4, ""), 1);
			if (myht->nApplyCount < 1) {
				xdebug_str_add(str, xdebug_sprintf("<b>object</b>(<i>%s</i>)", Z_OBJCE_PP(struc)->name), 1);
				xdebug_str_add(str, xdebug_sprintf("[<i>%d</i>]\n", Z_OBJ_HANDLE_PP(struc)), 1);
				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					zend_hash_apply_with_arguments(myht XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export_fancy, 5, level, str, debug_zval, options, Z_OBJCE_PP(struc)->name);
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 4) - 2, ""), 1);
				}
			} else {
				xdebug_str_add(str, xdebug_sprintf("<i>&</i><b>object</b>(<i>%s</i>)", Z_OBJCE_PP(struc)->name), 1);
				xdebug_str_add(str, xdebug_sprintf("[<i>%d</i>]\n", Z_OBJ_HANDLE_PP(struc)), 1);
			}
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("<b>resource</b>(<i>%ld</i><font color='%s'>,</font> <i>%s</i>)", Z_LVAL_PP(struc), COLOR_RESOURCE, type_name ? type_name : "Unknown"), 1);
			break;
		}

		default:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>null</font>", COLOR_NULL), 0);
			break;
	}
	if (Z_TYPE_PP(struc) != IS_ARRAY && Z_TYPE_PP(struc) != IS_OBJECT) {
		xdebug_str_addl(str, "\n", 1, 0);
	}
}

char* xdebug_get_zval_value_fancy(char *name, zval *val, int *len, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	xdebug_str str = {0, 0, NULL};
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini(TSRMLS_C);
		default_options = 1;
	}

	xdebug_str_addl(&str, "<pre class='xdebug-var-dump' dir='ltr'>", 39, 0);
	xdebug_var_export_fancy(&val, (xdebug_str*) &str, 1, debug_zval, options TSRMLS_CC);
	xdebug_str_addl(&str, "</pre>", 6, 0);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	*len = str.l;
	return str.d;
}

static void xdebug_var_synopsis_fancy(zval **struc, xdebug_str *str, int level, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	HashTable *myht;

	if (debug_zval) {
		xdebug_str_add(str, xdebug_sprintf("<i>(refcount=%d, is_ref=%d)</i>,", (*struc)->XDEBUG_REFCOUNT, (*struc)->XDEBUG_IS_REF), 1);
	}
	switch (Z_TYPE_PP(struc)) {
		case IS_BOOL:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>bool</font>", COLOR_BOOL), 1);
			break;

		case IS_NULL:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>null</font>", COLOR_NULL), 1);
			break;

		case IS_LONG:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>long</font>", COLOR_LONG), 1);
			break;

		case IS_DOUBLE:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>double</font>", COLOR_DOUBLE), 1);
			break;

		case IS_STRING:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>string(%d)</font>", COLOR_STRING, Z_STRLEN_PP(struc)), 1);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>array(%d)</font>", COLOR_ARRAY, myht->nNumOfElements), 1);
			break;

		case IS_OBJECT:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>object(%s)", COLOR_OBJECT, Z_OBJCE_PP(struc)->name), 1);
			xdebug_str_add(str, xdebug_sprintf("[%d]", Z_OBJ_HANDLE_PP(struc)), 1);
			xdebug_str_addl(str, "</font>", 7, 0);
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>resource(%ld, %s)</font>", COLOR_RESOURCE, Z_LVAL_PP(struc), type_name ? type_name : "Unknown"), 1);
			break;
		}
	}
}

char* xdebug_get_zval_synopsis_fancy(char *name, zval *val, int *len, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	xdebug_str str = {0, 0, NULL};
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini(TSRMLS_C);
		default_options = 1;
	}

	xdebug_var_synopsis_fancy(&val, (xdebug_str*) &str, 1, debug_zval, options TSRMLS_CC);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	*len = str.l;
	return str.d;
}

/*****************************************************************************
** XML encoding function
*/

char* xdebug_xmlize(char *string, int len, int *newlen)
{
	char *tmp;
	char *tmp2;

	if (len) {
		tmp = php_str_to_str(string, len, "&", 1, "&amp;", 5, &len);

		tmp2 = php_str_to_str(tmp, len, ">", 1, "&gt;", 4, &len);
		efree(tmp);

		tmp = php_str_to_str(tmp2, len, "<", 1, "&lt;", 4, &len);
		efree(tmp2);

		tmp2 = php_str_to_str(tmp, len, "\"", 1, "&quot;", 6, &len);
		efree(tmp);

		tmp = php_str_to_str(tmp2, len, "'", 1, "&apos;", 6, &len);
		efree(tmp2);

		tmp2 = php_str_to_str(tmp, len, "\n", 1, "&#10;", 5, &len);
		efree(tmp);

		tmp = php_str_to_str(tmp2, len, "\0", 1, "&#0;", 4, newlen);
		efree(tmp2);
		return tmp;
	} else {
		*newlen = len;
		return estrdup(string);
	}
}

/*****************************************************************************
** Function name printing function
*/

char* xdebug_show_fname(xdebug_func f, int html, int flags TSRMLS_DC)
{
	char *tmp;

	switch (f.type) {
		case XFUNC_NORMAL: {
			zend_function *zfunc;

			if (PG(html_errors) && EG(function_table) && zend_hash_find(EG(function_table), f.function, strlen(f.function) + 1, (void**) &zfunc) == SUCCESS) {
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
