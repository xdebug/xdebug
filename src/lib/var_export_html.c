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

#include "var_export_html.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

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

static int xdebug_array_element_export_html(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, xdebug_str *str, int debug_zval, xdebug_var_export_options *options)
{
	zval **zv = &zv_nptr;
	size_t newlen;
	char *tmp_str;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 4) - 2, ""), 1);

		if (HASH_KEY_IS_NUMERIC(hash_key)) { /* numeric key */
			xdebug_str_add(str, xdebug_sprintf(XDEBUG_INT_FMT " <font color='%s'>=&gt;</font> ", index_key, COLOR_POINTER), 1);
		} else { /* string key */
			xdebug_str_addl(str, "'", 1, 0);
			tmp_str = xdebug_xmlize((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key) - 1, &newlen);
			xdebug_str_addl(str, tmp_str, newlen, 0);
			efree(tmp_str);
			xdebug_str_add(str, xdebug_sprintf("' <font color='%s'>=&gt;</font> ", COLOR_POINTER), 1);
		}
		xdebug_var_export_html(zv, str, level + 1, debug_zval, options);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 4) - 2, ""), 1);
		xdebug_str_addl(str, "<i>more elements...</i>\n", 24, 0);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export_html(zval *object, zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, xdebug_str *str, int debug_zval, xdebug_var_export_options *options, char *class_name)
{
	zval **zv = &zv_nptr;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 4) - 2, ""), 1);

		if (!HASH_KEY_IS_NUMERIC(hash_key)) {
			char       *prop_class_name;
			xdebug_str *property_name;
			xdebug_str *property_type = NULL;
			const char *modifier;

#if PHP_VERSION_ID >= 70400
			property_type = xdebug_get_property_type(object, zv_nptr);
#endif
			property_name = xdebug_get_property_info((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key), &modifier, &prop_class_name);

			xdebug_str_add(str, xdebug_sprintf("<i>%s</i> ", modifier), 1);
			if (property_type) {
				xdebug_str_add(str, xdebug_sprintf("<i>%s</i> ", property_type->d), 1);
			}
			xdebug_str_addc(str, '\'');
			xdebug_str_add_str(str, property_name);
			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				xdebug_str_add(str, xdebug_sprintf("' <font color='%s'>=&gt;</font> ", COLOR_POINTER), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("' <small>(%s)</small> <font color='%s'>=&gt;</font> ", prop_class_name, COLOR_POINTER), 1);
			}

			if (property_type) {
				xdebug_str_free(property_type);
			}
			xdebug_str_free(property_name);
			xdfree(prop_class_name);
		} else {
			xdebug_str_add(str, xdebug_sprintf("<i>public</i> " XDEBUG_INT_FMT " <font color='%s'>=&gt;</font> ", index_key, COLOR_POINTER), 1);
		}
		xdebug_var_export_html(zv, str, level + 1, debug_zval, options);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 4) - 2, ""), 1);
		xdebug_str_addl(str, "<i>more elements...</i>\n", 24, 0);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

void xdebug_var_export_html(zval **struc, xdebug_str *str, int level, int debug_zval, xdebug_var_export_options *options)
{
	HashTable *myht;
	char*     tmp_str;
	size_t    newlen;
#if PHP_VERSION_ID < 70400
	int       is_temp;
#endif
	zend_ulong num;
	zend_string *key;
	zval *val;
	zval *tmpz;

	if (debug_zval) {
		xdebug_add_variable_attributes(str, *struc, XDEBUG_VAR_ATTR_HTML);
	}
	if (Z_TYPE_P(*struc) == IS_INDIRECT) {
		tmpz = Z_INDIRECT_P(*struc);
		struc = &tmpz;
	}
	if (Z_TYPE_P(*struc) == IS_REFERENCE) {
		tmpz = &((*struc)->value.ref->val);
		struc = &tmpz;
	}

	switch (Z_TYPE_P(*struc)) {
		case IS_TRUE:
		case IS_FALSE:
			xdebug_str_add(str, xdebug_sprintf("<small>boolean</small> <font color='%s'>%s</font>", COLOR_BOOL, Z_TYPE_P(*struc) == IS_TRUE ? "true" : "false"), 1);
			break;

		case IS_NULL:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>null</font>", COLOR_NULL), 1);
			break;

		case IS_LONG:
			xdebug_str_add(str, xdebug_sprintf("<small>int</small> <font color='%s'>" XDEBUG_INT_FMT "</font>", COLOR_LONG, Z_LVAL_P(*struc)), 1);
			break;

		case IS_DOUBLE:
			xdebug_str_add(str, xdebug_sprintf("<small>float</small> <font color='%s'>%.*G</font>", COLOR_DOUBLE, (int) EG(precision), Z_DVAL_P(*struc)), 1);
			break;

		case IS_STRING:
			xdebug_str_add(str, xdebug_sprintf("<small>string</small> <font color='%s'>'", COLOR_STRING), 1);
			if ((size_t) Z_STRLEN_P(*struc) > (size_t) options->max_data) {
				tmp_str = xdebug_xmlize(Z_STRVAL_P(*struc), options->max_data, &newlen);
				xdebug_str_addl(str, tmp_str, newlen, 0);
				efree(tmp_str);
				xdebug_str_addl(str, "'...</font>", 11, 0);
			} else {
				tmp_str = xdebug_xmlize(Z_STRVAL_P(*struc), Z_STRLEN_P(*struc), &newlen);
				xdebug_str_addl(str, tmp_str, newlen, 0);
				efree(tmp_str);
				xdebug_str_addl(str, "'</font>", 8, 0);
			}
			xdebug_str_add(str, xdebug_sprintf(" <i>(length=%d)</i>", Z_STRLEN_P(*struc)), 1);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);
			xdebug_str_add(str, xdebug_sprintf("\n%*s", (level - 1) * 4, ""), 1);

			if (!xdebug_zend_hash_is_recursive(myht)) {
				xdebug_str_add(str, xdebug_sprintf("<b>array</b> <i>(size=%d)</i>\n", myht->nNumOfElements), 1);
				if (level <= options->max_depth) {
					if (myht->nNumOfElements) {
						options->runtime[level].current_element_nr = 0;
						options->runtime[level].start_element_nr = 0;
						options->runtime[level].end_element_nr = options->max_children;

						xdebug_zend_hash_apply_protection_begin(myht);

						ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
							xdebug_array_element_export_html(val, num, key, level, str, debug_zval, options);
						} ZEND_HASH_FOREACH_END();

						xdebug_zend_hash_apply_protection_end(myht);
					} else {
						xdebug_str_add(str, xdebug_sprintf("%*s", (level * 4) - 2, ""), 1);
						xdebug_str_add(str, xdebug_sprintf("<i><font color='%s'>empty</font></i>\n", COLOR_EMPTY), 1);
					}
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 4) - 2, ""), 1);
				}
			} else {
				xdebug_str_addl(str, "<i>&amp;</i><b>array</b>\n", 21, 0);
			}
			break;

		case IS_OBJECT:
#if PHP_VERSION_ID >= 70400
			myht = xdebug_objdebug_pp(struc);
#else
			myht = xdebug_objdebug_pp(struc, &is_temp);
#endif
			xdebug_str_add(str, xdebug_sprintf("\n%*s", (level - 1) * 4, ""), 1);

			if (!myht || !xdebug_zend_hash_is_recursive(myht)) {
				char *class_name = (char*) STR_NAME_VAL(Z_OBJCE_P(*struc)->name);
				xdebug_str_add(str, xdebug_sprintf("<b>object</b>(<i>%s</i>)", class_name), 1);
				xdebug_str_add(str, xdebug_sprintf("[<i>%d</i>]\n", Z_OBJ_HANDLE_P(*struc)), 1);

				if (myht && (level <= options->max_depth)) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					xdebug_zend_hash_apply_protection_begin(myht);

					ZEND_HASH_FOREACH_KEY_VAL(myht, num, key, val) {
						xdebug_object_element_export_html(*struc, val, num, key, level, str, debug_zval, options, class_name);
					} ZEND_HASH_FOREACH_END();

					xdebug_zend_hash_apply_protection_end(myht);
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 4) - 2, ""), 1);
				}
			} else {
				xdebug_str_add(str, xdebug_sprintf("<i>&amp;</i><b>object</b>(<i>%s</i>)", STR_NAME_VAL(Z_OBJCE_P(*struc)->name)), 1);
				xdebug_str_add(str, xdebug_sprintf("[<i>%d</i>]\n", Z_OBJ_HANDLE_P(*struc)), 1);
			}
#if PHP_VERSION_ID >= 70400
			zend_release_properties(myht);
#else
			xdebug_var_maybe_destroy_ht(myht, is_temp);
#endif
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc));
			xdebug_str_add(str, xdebug_sprintf("<b>resource</b>(<i>%ld</i><font color='%s'>,</font> <i>%s</i>)", Z_RES_P(*struc)->handle, COLOR_RESOURCE, type_name ? type_name : "Unknown"), 1);
			break;
		}

		case IS_UNDEF:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>*uninitialized*</font>", COLOR_NULL), 0);
			break;

		default:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>NFC</font>", COLOR_NULL), 0);
			break;
	}
	if (Z_TYPE_P(*struc) != IS_ARRAY && Z_TYPE_P(*struc) != IS_OBJECT) {
		xdebug_str_addl(str, "\n", 1, 0);
	}
}

xdebug_str* xdebug_get_zval_value_html(char *name, zval *val, int debug_zval, xdebug_var_export_options *options)
{
	xdebug_str *str = xdebug_str_new();
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini();
		default_options = 1;
	}

	xdebug_str_addl(str, "<pre class='xdebug-var-dump' dir='ltr'>", 39, 0);
	if (options->show_location && !debug_zval) {
		char *formatted_filename;
		xdebug_format_filename(&formatted_filename, XINI_BASE(filename_format), "%f", zend_get_executed_filename());

		if (strlen(XINI_BASE(file_link_format)) > 0) {
			char *file_link;

			xdebug_format_file_link(&file_link, zend_get_executed_filename(), zend_get_executed_lineno());
			xdebug_str_add(str, xdebug_sprintf("\n<small><a href='%s'>%s:%d</a>:</small>", file_link, formatted_filename, zend_get_executed_lineno()), 1);
			xdfree(file_link);
		} else {
			xdebug_str_add(str, xdebug_sprintf("\n<small>%s:%d:</small>", formatted_filename, zend_get_executed_lineno()), 1);
		}

		xdfree(formatted_filename);
	}
	xdebug_var_export_html(&val, str, 1, debug_zval, options);
	xdebug_str_addl(str, "</pre>", 6, 0);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	return str;
}

static void xdebug_var_synopsis_html(zval **struc, xdebug_str *str, int level, int debug_zval, xdebug_var_export_options *options)
{
	HashTable *myht;
	zval *tmpz;

	if (debug_zval) {
		xdebug_add_variable_attributes(str, *struc, XDEBUG_VAR_ATTR_HTML);
	}
	if (Z_TYPE_P(*struc) == IS_REFERENCE) {
		tmpz = &((*struc)->value.ref->val);
		struc = &tmpz;
	}

	switch (Z_TYPE_P(*struc)) {
		case IS_TRUE:
		case IS_FALSE:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>%s</font>", COLOR_BOOL, Z_TYPE_P(*struc) == IS_TRUE ? "true" : "false"), 1);
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
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>string(%d)</font>", COLOR_STRING, Z_STRLEN_P(*struc)), 1);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>array(%d)</font>", COLOR_ARRAY, myht->nNumOfElements), 1);
			break;

		case IS_OBJECT:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>object(%s)", COLOR_OBJECT, STR_NAME_VAL(Z_OBJCE_P(*struc)->name)), 1);
			xdebug_str_add(str, xdebug_sprintf("[%d]", Z_OBJ_HANDLE_P(*struc)), 1);
			xdebug_str_addl(str, "</font>", 7, 0);
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc));
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>resource(%ld, %s)</font>", COLOR_RESOURCE, Z_RES_P(*struc)->handle, type_name ? type_name : "Unknown"), 1);
			break;
		}

		case IS_UNDEF:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>*uninitialized*</font>", COLOR_NULL), 0);
			break;

		default:
			xdebug_str_add(str, xdebug_sprintf("<font color='%s'>NFC</font>", COLOR_NULL), 0);
			break;
	}
}

xdebug_str* xdebug_get_zval_synopsis_html(const char *name, zval *val, int debug_zval, xdebug_var_export_options *options)
{
	xdebug_str *str = xdebug_str_new();
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini();
		default_options = 1;
	}

	xdebug_var_synopsis_html(&val, str, 1, debug_zval, options);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	return str;
}
