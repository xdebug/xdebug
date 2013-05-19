/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
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

char* xdebug_error_type_simple(int type)
{
	switch (type) {
		case E_ERROR:
		case E_CORE_ERROR:
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			return xdstrdup("fatal-error");
			break;
#if PHP_VERSION_ID >= 50200
		case E_RECOVERABLE_ERROR:
			return xdstrdup("catchable-fatal-error");
			break;
#endif
		case E_WARNING:
		case E_CORE_WARNING:
		case E_COMPILE_WARNING:
		case E_USER_WARNING:
			return xdstrdup("warning");
			break;
		case E_PARSE:
			return xdstrdup("parse-error");
			break;
		case E_NOTICE:
		case E_USER_NOTICE:
			return xdstrdup("notice");
			break;
		case E_STRICT:
			return xdstrdup("strict-standards");
			break;
#if PHP_VERSION_ID >= 50300
		case E_DEPRECATED:
		case E_USER_DEPRECATED:
			return xdstrdup("deprecated");
			break;
#endif
		case 0:
			return xdstrdup("xdebug");
			break;
		default:
			return xdstrdup("unknown-error");
			break;
	}
}

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
		case E_USER_DEPRECATED:
			return xdstrdup("Deprecated");
			break;
#endif
		case 0:
			return xdstrdup("Xdebug");
			break;
		default:
			return xdstrdup("Unknown error");
			break;
	}
}

/*************************************************************************************************************************************/
#if PHP_VERSION_ID >= 50500
# define T(offset) (*EX_TMP_VAR(zdata, offset))
#else
# define T(offset) (*(temp_variable *)((char*)zdata->Ts + offset))
#endif

zval *xdebug_get_zval(zend_execute_data *zdata, int node_type, XDEBUG_ZNODE *node, int *is_var)
{
	switch (node_type) {
		case IS_CONST:
#if PHP_VERSION_ID >= 50399
			return node->zv;
#else
			return &node->u.constant;
#endif
			break;

		case IS_TMP_VAR:
			*is_var = 1;
#if PHP_VERSION_ID >= 50399
			return &T(node->var).tmp_var;
#else
			return &T(node->u.var).tmp_var;
#endif
			break;

		case IS_VAR:
			*is_var = 1;
#if PHP_VERSION_ID >= 50399
			if (T(node->var).var.ptr) {
				return T(node->var).var.ptr;
#else
			if (T(node->u.var).var.ptr) {
				return T(node->u.var).var.ptr;
#endif
			}
			break;

		case IS_CV: {
			zval **tmp;
#if PHP_VERSION_ID >= 50399
			tmp = zend_get_compiled_variable_value(zdata, node->constant);
#else
			tmp = zend_get_compiled_variable_value(zdata, node->u.constant.value.lval);
#endif
			if (tmp) {
				return *tmp;
			}
			break;
		}

		case IS_UNUSED:
			fprintf(stderr, "\nIS_UNUSED\n");
			break;

		default:
			fprintf(stderr, "\ndefault %d\n", node_type);
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

char* xdebug_get_property_info(char *mangled_property, int mangled_len, char **property_name, char **class_name)
{
	const char *prop_name, *cls_name;

#if PHP_VERSION_ID >= 50200
	zend_unmangle_property_name(mangled_property, mangled_len - 1, &cls_name, &prop_name);
#else
	zend_unmangle_property_name(mangled_property, &cls_name, &prop_name);
#endif
	*property_name = (char *) prop_name;
	*class_name = (char *) cls_name;
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

#define XDEBUG_MAX_INT 2147483647

xdebug_var_export_options* xdebug_var_export_options_from_ini(TSRMLS_D)
{
	xdebug_var_export_options *options;
	options = xdmalloc(sizeof(xdebug_var_export_options));

	options->max_children = XG(display_max_children);
	options->max_data = XG(display_max_data);
	options->max_depth = XG(display_max_depth);
	options->show_hidden = 0;

	if (options->max_children == -1 || options->max_children > XDEBUG_MAX_INT) {
		options->max_children = XDEBUG_MAX_INT;
	} else if (options->max_children < 1) {
		options->max_children = 0;
	}

	if (options->max_data == -1 || options->max_data > XDEBUG_MAX_INT) {
		options->max_data = XDEBUG_MAX_INT;
	} else if (options->max_data < 1) {
		options->max_data = 0;
	}

	if (options->max_depth == -1 || options->max_depth > 1023) {
		options->max_depth = 1023;
	} else if (options->max_depth < 1) {
		options->max_depth = 0;
	}

	options->runtime = (xdebug_var_runtime_page*) xdmalloc((options->max_depth + 1) * sizeof(xdebug_var_runtime_page));
	options->no_decoration = 0;

	return options;
}

xdebug_var_export_options xdebug_var_nolimit_options = { XDEBUG_MAX_INT, XDEBUG_MAX_INT, 1023, 1, NULL, 0 };

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
			
			tmp = php_str_to_str((char *) hash_key->arKey, hash_key->nKeyLength, "'", 1, "\\'", 2, &newlen);
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
			modifier = xdebug_get_property_info((char *) hash_key->arKey, hash_key->nKeyLength, &prop_name, &prop_class_name);
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
			tmp_str = php_addcslashes(Z_STRVAL_PP(struc), Z_STRLEN_PP(struc), &tmp_len, 0, "'\\\0..\37", 7 TSRMLS_CC);
			if (options->no_decoration) {
				xdebug_str_add(str, tmp_str, 0);
			} else if (Z_STRLEN_PP(struc) <= options->max_data) {
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

				zend_get_object_classname(*struc, (const char **) &class_name, &class_name_len TSRMLS_CC);
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

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
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

			zend_get_object_classname(*struc, (const char **) &class_name, &class_name_len TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("class %s", class_name), 1);
			efree(class_name);
			break;
		}

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
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
** Plain text/ANSI coloured variable printing routines
*/

#define ANSI_COLOR_POINTER       (mode == 1 ? "[0m" : "")
#define ANSI_COLOR_BOOL          (mode == 1 ? "[35m" : "")
#define ANSI_COLOR_LONG          (mode == 1 ? "[32m" : "")
#define ANSI_COLOR_NULL          (mode == 1 ? "[34m" : "")
#define ANSI_COLOR_DOUBLE        (mode == 1 ? "[33m" : "")
#define ANSI_COLOR_STRING        (mode == 1 ? "[31m" : "")
#define ANSI_COLOR_EMPTY         (mode == 1 ? "[30m" : "")
#define ANSI_COLOR_ARRAY         (mode == 1 ? "[33m" : "")
#define ANSI_COLOR_OBJECT        (mode == 1 ? "[31m" : "")
#define ANSI_COLOR_RESOURCE      (mode == 1 ? "[36m" : "")
#define ANSI_COLOR_MODIFIER      (mode == 1 ? "[32m" : "")
#define ANSI_COLOR_RESET         (mode == 1 ? "[0m" : "")
#define ANSI_COLOR_BOLD          (mode == 1 ? "[1m" : "")
#define ANSI_COLOR_BOLD_OFF      (mode == 1 ? "[22m" : "")

static int xdebug_array_element_export_text_ansi(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, mode, debug_zval;
	xdebug_str *str;
	xdebug_var_export_options *options;
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	TSRMLS_FETCH();
#endif

	level      = va_arg(args, int);
	mode       = va_arg(args, int);
	str        = va_arg(args, struct xdebug_str*);
	debug_zval = va_arg(args, int);
	options    = va_arg(args, xdebug_var_export_options*);

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 2), ""), 1);

		if (hash_key->nKeyLength==0) { /* numeric key */
			xdebug_str_add(str, xdebug_sprintf("[%ld] %s=>%s\n", hash_key->h, ANSI_COLOR_POINTER, ANSI_COLOR_RESET), 1);
		} else { /* string key */
			int newlen = 0;
			char *tmp, *tmp2;
			
			tmp = php_str_to_str((char *) hash_key->arKey, hash_key->nKeyLength, "'", 1, "\\'", 2, &newlen);
			tmp2 = php_str_to_str(tmp, newlen - 1, "\0", 1, "\\0", 2, &newlen);
			if (tmp) {
				efree(tmp);
			}
			xdebug_str_addl(str, "'", 1, 0);
			if (tmp2) {
				xdebug_str_addl(str, tmp2, newlen, 0);
				efree(tmp2);
			}
			xdebug_str_add(str, "' =>\n", 0);
		}
		xdebug_var_export_text_ansi(zv, str, mode, level + 1, debug_zval, options TSRMLS_CC);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add(str, xdebug_sprintf("\n%*s(more elements)...\n", (level * 2), ""), 1);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export_text_ansi(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, mode, debug_zval;
	xdebug_str *str;
	xdebug_var_export_options *options;
	char *prop_name, *class_name, *modifier;
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	TSRMLS_FETCH();
#endif

	level      = va_arg(args, int);
	mode       = va_arg(args, int);
	str        = va_arg(args, struct xdebug_str*);
	debug_zval = va_arg(args, int);
	options    = va_arg(args, xdebug_var_export_options*);

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 2), ""), 1);

		if (hash_key->nKeyLength != 0) {
			modifier = xdebug_get_property_info((char *) hash_key->arKey, hash_key->nKeyLength, &prop_name, &class_name);
			xdebug_str_add(str, xdebug_sprintf("%s%s%s%s%s $%s %s=>%s\n",
			               ANSI_COLOR_MODIFIER, ANSI_COLOR_BOLD, modifier, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_RESET, 
			               prop_name, ANSI_COLOR_POINTER, ANSI_COLOR_RESET), 1);
		}
		xdebug_var_export_text_ansi(zv, str, mode, level + 1, debug_zval, options TSRMLS_CC);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add(str, xdebug_sprintf("\n%*s(more elements)...\n", (level * 2), ""), 1);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

void xdebug_var_export_text_ansi(zval **struc, xdebug_str *str, int mode, int level, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	HashTable *myht;
	char*     tmp_str;
	int       tmp_len;
#if PHP_VERSION_ID >= 50300
	int       is_temp;
#endif

	if (!struc || !(*struc)) {
		return;
	}
	if (debug_zval) {
		xdebug_str_add(str, xdebug_sprintf("(refcount=%d, is_ref=%d)=", (*struc)->XDEBUG_REFCOUNT, (*struc)->XDEBUG_IS_REF), 1);
	}
	
	xdebug_str_add(str, xdebug_sprintf("%*s", (level * 2) - 2, ""), 1);

	switch (Z_TYPE_PP(struc)) {
		case IS_BOOL:
			xdebug_str_add(str, xdebug_sprintf("%sbool%s(%s%s%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_BOOL, Z_LVAL_PP(struc) ? "true" : "false", ANSI_COLOR_RESET), 1);
			break;

		case IS_NULL:
			xdebug_str_add(str, xdebug_sprintf("%s%sNULL%s%s", ANSI_COLOR_BOLD, ANSI_COLOR_NULL, ANSI_COLOR_RESET, ANSI_COLOR_BOLD_OFF), 1);
			break;

		case IS_LONG:
			xdebug_str_add(str, xdebug_sprintf("%sint%s(%s%ld%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_LONG, Z_LVAL_PP(struc), ANSI_COLOR_RESET), 1);
			break;

		case IS_DOUBLE:
			xdebug_str_add(str, xdebug_sprintf("%sdouble%s(%s%.*G%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_DOUBLE, (int) EG(precision), Z_DVAL_PP(struc), ANSI_COLOR_RESET), 1);
			break;

		case IS_STRING:
			if (mode == 1) {
				tmp_str = php_addcslashes(Z_STRVAL_PP(struc), Z_STRLEN_PP(struc), &tmp_len, 0, "'\\\0..\37", 7 TSRMLS_CC);
			} else {
				tmp_str = php_addcslashes(Z_STRVAL_PP(struc), Z_STRLEN_PP(struc), &tmp_len, 0, "\0", 1 TSRMLS_CC);
			}
			if (options->no_decoration) {
				xdebug_str_addl(str, tmp_str, tmp_len, 0);
			} else if (Z_STRLEN_PP(struc) <= options->max_data) {
				xdebug_str_add(str, xdebug_sprintf("%sstring%s(%s%ld%s) \"%s%s%s\"", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, 
							ANSI_COLOR_LONG, Z_STRLEN_PP(struc), ANSI_COLOR_RESET,
							ANSI_COLOR_STRING, tmp_str, ANSI_COLOR_RESET), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("%sstring%s(%s%ld%s) \"%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, 
							ANSI_COLOR_LONG, Z_STRLEN_PP(struc), ANSI_COLOR_RESET, ANSI_COLOR_STRING), 1);
				xdebug_str_addl(str, tmp_str, options->max_data, 0);
				xdebug_str_add(str, xdebug_sprintf("%s\"...", ANSI_COLOR_RESET), 1);
			}
			efree(tmp_str);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			if (myht->nApplyCount < 1) {
				xdebug_str_add(str, xdebug_sprintf("%sarray%s(%s%d%s) {\n", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_LONG, myht->nNumOfElements, ANSI_COLOR_RESET), 1);
				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					zend_hash_apply_with_arguments(myht XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_array_element_export_text_ansi, 5, level, mode, str, debug_zval, options);
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 2), ""), 1);
				}
				xdebug_str_add(str, xdebug_sprintf("%*s}", (level * 2) - 2 , ""), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("&%sarray%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF), 1);
			}
			break;

		case IS_OBJECT:
#if PHP_VERSION_ID >= 50300
			myht = Z_OBJDEBUG_PP(struc, is_temp);
#else
			myht = Z_OBJPROP_PP(struc);
#endif
			if (myht && myht->nApplyCount < 1) {
				char *class_name;
				zend_uint class_name_len;

				zend_get_object_classname(*struc, (const char **) &class_name, &class_name_len TSRMLS_CC);
				xdebug_str_add(str, xdebug_sprintf("%sclass%s %s%s%s#%d (%s%d%s) {\n", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_OBJECT, class_name, ANSI_COLOR_RESET,
							Z_OBJ_HANDLE_PP(struc),
							ANSI_COLOR_LONG, myht->nNumOfElements, ANSI_COLOR_RESET), 1);
				efree(class_name);

				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					zend_hash_apply_with_arguments(myht XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export_text_ansi, 5, level, mode, str, debug_zval, options);
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 2), ""), 1);
				}
				xdebug_str_add(str, xdebug_sprintf("%*s}", (level * 2) - 2, ""), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 2), ""), 1);
			}
#if PHP_VERSION_ID >= 50300
			if (is_temp) {
				zend_hash_destroy(myht);
				efree(myht);
			}
#endif
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("%sresource%s(%s%ld%s) of type (%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, 
			               ANSI_COLOR_RESOURCE, Z_LVAL_PP(struc), ANSI_COLOR_RESET, type_name ? type_name : "Unknown"), 1);
			break;
		}

		default:
			xdebug_str_add(str, xdebug_sprintf("%sNULL%s", ANSI_COLOR_NULL, ANSI_COLOR_RESET), 0);
			break;
	}

	xdebug_str_addl(str, "\n", 1, 0);
}

char* xdebug_get_zval_value_text_ansi(zval *val, int mode, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	xdebug_str str = {0, 0, NULL};
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini(TSRMLS_C);
		default_options = 1;
	}

	xdebug_var_export_text_ansi(&val, (xdebug_str*) &str, mode, 1, debug_zval, options TSRMLS_CC);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	return str.d;
}

static void xdebug_var_synopsis_text_ansi(zval **struc, xdebug_str *str, int mode, int level, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
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
			xdebug_str_add(str, xdebug_sprintf("%sbool%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF), 1);
			break;

		case IS_NULL:
			xdebug_str_add(str, xdebug_sprintf("%snull%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF), 1);
			break;

		case IS_LONG:
			xdebug_str_add(str, xdebug_sprintf("%sint%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF), 1);
			break;

		case IS_DOUBLE:
			xdebug_str_add(str, xdebug_sprintf("%sdouble%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF), 1);
			break;

		case IS_STRING:
			xdebug_str_add(str, xdebug_sprintf("%sstring%s(%s%d%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_LONG, Z_STRLEN_PP(struc), ANSI_COLOR_RESET), 1);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			xdebug_str_add(str, xdebug_sprintf("array(%s%d%s)", ANSI_COLOR_LONG, myht->nNumOfElements, ANSI_COLOR_RESET), 1);
			break;

		case IS_OBJECT: {
			char *class_name;
			zend_uint class_name_len;

			zend_get_object_classname(*struc, (const char **) &class_name, &class_name_len TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("class %s", class_name), 1);
			break;
		}

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("resource(%s%ld%s) of type (%s)", ANSI_COLOR_LONG, Z_LVAL_PP(struc), ANSI_COLOR_RESET, type_name ? type_name : "Unknown"), 1);
			break;
		}
	}
}

char* xdebug_get_zval_synopsis_text_ansi(zval *val, int mode, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	xdebug_str str = {0, 0, NULL};
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini(TSRMLS_C);
		default_options = 1;
	}

	xdebug_var_synopsis_text_ansi(&val, (xdebug_str*) &str, mode, 1, debug_zval, options TSRMLS_CC);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	return str.d;
}


/*****************************************************************************
** XML node printing routines
*/

#define XDEBUG_OBJECT_ITEM_TYPE_PROPERTY        1
#define XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY 2

typedef struct
{
	char  type;
	char *name;
	int   name_len;
	zval *zv;
} xdebug_object_item;

static int object_item_add_to_merged_hash(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	HashTable          *merged;
	int                 object_type;
	xdebug_object_item *item;

	merged = va_arg(args, HashTable*);
	object_type = va_arg(args, int);

	item = xdmalloc(sizeof(xdebug_object_item));
	item->type = object_type;
	item->zv   = *zv;
	item->name = (char *) hash_key->arKey;
	item->name_len = hash_key->nKeyLength;

	zend_hash_next_index_insert(merged, &item, sizeof(xdebug_object_item*), NULL);

	return 0;
}

#if PHP_VERSION_ID >= 50400
static int object_item_add_zend_prop_to_merged_hash(zend_property_info *zpp XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	HashTable          *merged;
	int                 object_type;
	xdebug_object_item *item;
	zend_class_entry   *ce;

	if ((zpp->flags & ZEND_ACC_STATIC) == 0) {
		return 0;
	}
	merged = va_arg(args, HashTable*);
	object_type = va_arg(args, int);
	ce = va_arg(args, zend_class_entry*);

	item = xdmalloc(sizeof(xdebug_object_item));
	item->type = object_type;
#if ZTS
	if (ce->type == 1) {
		item->zv   = CG(static_members_table)[(zend_intptr_t) ce->static_members_table][zpp->offset];
	} else {
		item->zv   = ce->static_members_table[zpp->offset];
	}
#else
	item->zv   = ce->static_members_table[zpp->offset];
#endif
	item->name = (char *) zpp->name;
	item->name_len = zpp->name_length;

	zend_hash_next_index_insert(merged, &item, sizeof(xdebug_object_item*), NULL);

	return 0;
}
#endif

static int xdebug_array_element_export_xml_node(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_xml_node *parent;
	xdebug_xml_node *node;
	xdebug_var_export_options *options;
	char *name = NULL, *parent_name = NULL;
	int   name_len = 0;
	xdebug_str full_name = { 0, 0, NULL };
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
			name = xdstrndup(hash_key->arKey, hash_key->nKeyLength);
			name_len = hash_key->nKeyLength - 1;
			if (parent_name) {
				xdebug_str_add(&full_name, parent_name, 0);
				xdebug_str_addl(&full_name, "['", 2, 0);
				xdebug_str_addl(&full_name, name, name_len, 0);
				xdebug_str_addl(&full_name, "']", 2, 0);
			}
		} else {
			name = xdebug_sprintf("%ld", hash_key->h);
			name_len = strlen(name);
			if (parent_name) {
				xdebug_str_add(&full_name, xdebug_sprintf("%s[%s]", parent_name, name), 1);
			}
		}

		xdebug_xml_add_attribute_exl(node, "name", 4, name, name_len, 0, 1);
		if (full_name.l) {
			xdebug_xml_add_attribute_exl(node, "fullname", 8, full_name.d, full_name.l, 0, 1);
		}
		xdebug_xml_add_attribute_ex(node, "address", xdebug_sprintf("%ld", (long) *zv), 0, 1);

		xdebug_xml_add_child(parent, node);
		xdebug_var_export_xml_node(zv, full_name.d, node, options, level + 1 TSRMLS_CC);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export_xml_node(xdebug_object_item **item XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
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
		if ((*item)->name_len != 0) {
			modifier = xdebug_get_property_info((*item)->name, (*item)->name_len, &prop_name, &prop_class_name);
			node = xdebug_xml_node_init("property");
			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				xdebug_xml_add_attribute_ex(node, "name", xdstrdup(prop_name), 0, 1);
			} else {
				xdebug_xml_add_attribute_ex(node, "name", xdebug_sprintf("*%s*%s", prop_class_name, prop_name), 0, 1);
			}

			if (parent_name) {
				if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
					full_name = xdebug_sprintf("%s%s%s", parent_name, (*item)->type == XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY ? "::" : "->", prop_name);
				} else {
					full_name = xdebug_sprintf("%s%s*%s*%s", parent_name, (*item)->type == XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY ? "::" : "->", prop_class_name, prop_name);
				}
				xdebug_xml_add_attribute_ex(node, "fullname", full_name, 0, 1);
			}
			xdebug_xml_add_attribute_ex(node, "facet", xdebug_sprintf("%s%s", (*item)->type == XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY ? "static " : "", modifier), 0, 1);

			xdebug_xml_add_attribute_ex(node, "address", xdebug_sprintf("%ld", (long) (*item)->zv), 0, 1);

			xdebug_xml_add_child(parent, node);

			xdebug_var_export_xml_node(&((*item)->zv), full_name, node, options, level + 1 TSRMLS_CC);
		}
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static char *prepare_variable_name(char *name)
{
	char *tmp_name;

	tmp_name = xdebug_sprintf("%s%s", (name[0] == '$' || name[0] == ':') ? "" : "$", name);
	if (tmp_name[strlen(tmp_name) - 2] == ':' && tmp_name[strlen(tmp_name) - 1] == ':') {
		tmp_name[strlen(tmp_name) - 2] = '\0';
	}
	return tmp_name;
}

void xdebug_attach_uninitialized_var(xdebug_xml_node *node, char *name)
{
	xdebug_xml_node *contents = NULL;
	char            *tmp_name;

	contents = xdebug_xml_node_init("property");

	tmp_name = prepare_variable_name(name);
	xdebug_xml_add_attribute_ex(contents, "name", xdstrdup(tmp_name), 0, 1);
	xdebug_xml_add_attribute_ex(contents, "fullname", xdstrdup(tmp_name), 0, 1);
	xdfree(tmp_name);

	xdebug_xml_add_attribute(contents, "type", "uninitialized");
	xdebug_xml_add_child(node, contents);
}

#if PHP_VERSION_ID >= 50400
void xdebug_attach_property_with_contents(zend_property_info *prop_info XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	xdebug_xml_node    *node;
	char               *modifier;
	xdebug_xml_node    *contents = NULL;
	char               *class_name;
	zend_class_entry   *class_entry;
	char               *prop_name, *prop_class_name;
	xdebug_var_export_options *options;
	int                *children_count;

	node = va_arg(args, xdebug_xml_node *);
	options = va_arg(args, xdebug_var_export_options *);
	class_entry = va_arg(args, zend_class_entry *);
	class_name = va_arg(args, char *);
	children_count = va_arg(args, int *);

	if ((prop_info->flags & ZEND_ACC_STATIC) == 0) {
		return;
	}

	(*children_count)++;
	modifier = xdebug_get_property_info((char *) prop_info->name, prop_info->name_length, &prop_name, &prop_class_name);

	if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
		contents = xdebug_get_zval_value_xml_node_ex(prop_name, class_entry->static_members_table[prop_info->offset], XDEBUG_VAR_TYPE_STATIC, options TSRMLS_CC);
	} else{
		char *priv_name = xdebug_sprintf("*%s*%s", prop_class_name, prop_name);
		contents = xdebug_get_zval_value_xml_node_ex(priv_name, class_entry->static_members_table[prop_info->offset], XDEBUG_VAR_TYPE_STATIC, options TSRMLS_CC);
		xdfree(priv_name);
	}

	if (contents) {
		xdebug_xml_add_attribute_ex(contents, "facet", xdebug_sprintf("static %s", modifier), 0, 1);
		xdebug_xml_add_child(node, contents);
	} else {
		xdebug_attach_uninitialized_var(node, (char *) prop_info->name);
	}
}
#else
void xdebug_attach_static_var_with_contents(zval **zv XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	xdebug_xml_node    *node;
	char               *name = (char *) hash_key->arKey;
	char               *modifier;
	xdebug_xml_node    *contents = NULL;
	char               *class_name;
	char               *prop_name, *prop_class_name;
	xdebug_var_export_options *options;
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	TSRMLS_FETCH();
#endif

	node = va_arg(args, xdebug_xml_node *);
	options = va_arg(args, xdebug_var_export_options *);
	class_name = va_arg(args, char *);

	modifier = xdebug_get_property_info(name, hash_key->nKeyLength, &prop_name, &prop_class_name);

	if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
		contents = xdebug_get_zval_value_xml_node_ex(prop_name, *zv, XDEBUG_VAR_TYPE_STATIC, options TSRMLS_CC);
	} else{
		char *priv_name = xdebug_sprintf("*%s*%s", prop_class_name, prop_name);
		contents = xdebug_get_zval_value_xml_node_ex(priv_name, *zv, XDEBUG_VAR_TYPE_STATIC, options TSRMLS_CC);
		xdfree(priv_name);
	}

	if (contents) {
		xdebug_xml_add_attribute_ex(contents, "facet", xdebug_sprintf("static %s", modifier), 0, 1);
		xdebug_xml_add_child(node, contents);
	} else {
		xdebug_attach_uninitialized_var(node, name);
	}
}
#endif

void xdebug_attach_static_vars(xdebug_xml_node *node, xdebug_var_export_options *options, zend_class_entry *ce TSRMLS_DC)
{
#if PHP_VERSION_ID >= 50400
	HashTable        *static_members = &ce->properties_info;
	int               children = 0;
#else
	HashTable        *static_members = CE_STATIC_MEMBERS(ce);
#endif
	xdebug_xml_node  *static_container;

	static_container = xdebug_xml_node_init("property");
	xdebug_xml_add_attribute(static_container, "name", "::");
	xdebug_xml_add_attribute(static_container, "fullname", "::");
	xdebug_xml_add_attribute(static_container, "type", "object");
	xdebug_xml_add_attribute_ex(static_container, "classname", xdstrdup(ce->name), 0, 1);

#if PHP_VERSION_ID >= 50400
	zend_hash_apply_with_arguments(static_members XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_attach_property_with_contents, 5, static_container, options, ce, ce->name, &children); 
	xdebug_xml_add_attribute(static_container, "children", children > 0 ? "1" : "0");
	xdebug_xml_add_attribute_ex(static_container, "numchildren", xdebug_sprintf("%d", children), 0, 1);
#else
	zend_hash_apply_with_arguments(static_members XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_attach_static_var_with_contents, 3, static_container, options, ce->name); 
	xdebug_xml_add_attribute(static_container, "children", static_members->nNumOfElements > 0 ? "1" : "0");
	xdebug_xml_add_attribute_ex(static_container, "numchildren", xdebug_sprintf("%d", zend_hash_num_elements(static_members)), 0, 1);
#endif
	xdebug_xml_add_child(node, static_container);
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
			if (Z_STRLEN_PP(struc) <= options->max_data) {
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

		case IS_OBJECT: {
			HashTable *merged_hash;
			zend_class_entry *ce;
#if PHP_VERSION_ID >= 50400
			int is_temp;
#endif

			ALLOC_HASHTABLE(merged_hash);
			zend_hash_init(merged_hash, 128, NULL, NULL, 0);

			zend_get_object_classname(*struc, (const char **) &class_name, &class_name_len TSRMLS_CC);
			ce = zend_fetch_class(class_name, strlen(class_name), ZEND_FETCH_CLASS_DEFAULT TSRMLS_CC);

#if PHP_VERSION_ID >= 50400
			/* Adding static properties */
			if (&ce->properties_info) {
				zend_hash_apply_with_arguments(&ce->properties_info XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) object_item_add_zend_prop_to_merged_hash, 3, merged_hash, (int) XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY, ce);
			}

			/* Adding normal properties */
			zend_hash_apply_with_arguments(Z_OBJDEBUG_PP(struc, is_temp) XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) object_item_add_to_merged_hash, 2, merged_hash, (int) XDEBUG_OBJECT_ITEM_TYPE_PROPERTY);
#else
			/* Adding static properties */
			if (CE_STATIC_MEMBERS(ce)) {
				zend_hash_apply_with_arguments(CE_STATIC_MEMBERS(ce) XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) object_item_add_to_merged_hash, 2, merged_hash, (int) XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY);
			}

			/* Adding normal properties */
			myht = Z_OBJPROP_PP(struc);
			if (myht) {
				zend_hash_apply_with_arguments(myht XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) object_item_add_to_merged_hash, 2, merged_hash, (int) XDEBUG_OBJECT_ITEM_TYPE_PROPERTY);
			}
#endif

			xdebug_xml_add_attribute(node, "type", "object");
			xdebug_xml_add_attribute_ex(node, "classname", xdstrdup(class_name), 0, 1);
			xdebug_xml_add_attribute(node, "children", merged_hash->nNumOfElements ? "1" : "0");

			if (merged_hash->nApplyCount < 1) {
				xdebug_xml_add_attribute_ex(node, "numchildren", xdebug_sprintf("%d", zend_hash_num_elements(merged_hash)), 0, 1);
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
					zend_hash_apply_with_arguments(merged_hash XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export_xml_node, 5, level, node, name, options, class_name);
				}
			}
			efree(class_name);
			break;
		}

		case IS_RESOURCE: {
			char *type_name;

			xdebug_xml_add_attribute(node, "type", "resource");
			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			xdebug_xml_add_text(node, xdebug_sprintf("resource id='%ld' type='%s'", Z_LVAL_PP(struc), type_name ? type_name : "Unknown"));
			break;
		}

		default:
			xdebug_xml_add_attribute(node, "type", "null");
			break;
	}
}

xdebug_xml_node* xdebug_get_zval_value_xml_node_ex(char *name, zval *val, int var_type, xdebug_var_export_options *options TSRMLS_DC)
{
	xdebug_xml_node *node;
	char *short_name = NULL;
	char *full_name = NULL;

	node = xdebug_xml_node_init("property");
	if (name) {
		switch (var_type) {
			case XDEBUG_VAR_TYPE_NORMAL: {
				char *tmp_name;

				tmp_name = prepare_variable_name(name);
				short_name = xdstrdup(tmp_name);
				full_name = xdstrdup(tmp_name);
				xdfree(tmp_name);
			} break;
			case XDEBUG_VAR_TYPE_STATIC:
				short_name = xdebug_sprintf("::%s", name);
				full_name =  xdebug_sprintf("::%s", name);
				break;
		}

		xdebug_xml_add_attribute_ex(node, "name", short_name, 0, 1);
		xdebug_xml_add_attribute_ex(node, "fullname", full_name, 0, 1);
	}
	xdebug_xml_add_attribute_ex(node, "address", xdebug_sprintf("%ld", (long) val), 0, 1);
	xdebug_var_export_xml_node(&val, full_name, node, options, 0 TSRMLS_CC);

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
			tmp_str = xdebug_xmlize((char *) hash_key->arKey, hash_key->nKeyLength - 1, &newlen);
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

		if (hash_key->nKeyLength != 0) {
			modifier = xdebug_get_property_info((char *) hash_key->arKey, hash_key->nKeyLength, &prop_name, &prop_class_name);
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
				xdebug_str_add(str, xdebug_sprintf("<b>array</b> <i>(size=%d)</i>\n", myht->nNumOfElements), 1);
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

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
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

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
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

		tmp = php_str_to_str(tmp2, len, "'", 1, "&#39;", 5, &len);
		efree(tmp2);

		tmp2 = php_str_to_str(tmp, len, "\n", 1, "&#10;", 5, &len);
		efree(tmp);

		tmp = php_str_to_str(tmp2, len, "\r", 1, "&#13;", 5, &len);
		efree(tmp2);

		tmp2 = php_str_to_str(tmp, len, "\0", 1, "&#0;", 4, newlen);
		efree(tmp);
		return tmp2;
	} else {
		*newlen = len;
		return estrdup(string);
	}
}

/*****************************************************************************
** Function name printing function
*/
static char* xdebug_create_doc_link(xdebug_func f TSRMLS_DC)
{
	char *tmp_target, *p, *retval;

	switch (f.type) {
		case XFUNC_NORMAL: {
			tmp_target = xdebug_sprintf("function.%s", f.function);
			break;
		}

		case XFUNC_STATIC_MEMBER:
		case XFUNC_MEMBER: {
			if (strcmp(f.function, "__construct") == 0) {
				tmp_target = xdebug_sprintf("%s.construct", f.class);
			} else {
				tmp_target = xdebug_sprintf("%s.%s", f.class, f.function);
			}
			break;
		}
	}

	while ((p = strchr(tmp_target, '_')) != NULL) {
		*p = '-';
	}

	retval = xdebug_sprintf("<a href='%s%s%s' target='_new'>%s</a>\n",
		(PG(docref_root) && PG(docref_root)[0]) ? PG(docref_root) : "http://www.php.net/",
		tmp_target, PG(docref_ext), f.function);

	xdfree(tmp_target);

	return retval;
}

char* xdebug_show_fname(xdebug_func f, int html, int flags TSRMLS_DC)
{
	char *tmp;

	switch (f.type) {
		case XFUNC_NORMAL: {
			if (PG(html_errors) && html && f.internal) {
				return xdebug_create_doc_link(f TSRMLS_CC);
			} else {
				return xdstrdup(f.function);
			}
			break;
		}

		case XFUNC_STATIC_MEMBER:
		case XFUNC_MEMBER: {
			if (PG(html_errors) && html && f.internal) {
				return xdebug_create_doc_link(f TSRMLS_CC);
			} else {
				return xdebug_sprintf("%s%s%s",
					f.class ? f.class : "?",
					f.type == XFUNC_STATIC_MEMBER ? "::" : "->",
					f.function ? f.function : "?"
				);
			}
			break;
		}	

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

