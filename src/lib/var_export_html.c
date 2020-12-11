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
#include "lib_private.h"

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
		xdebug_str_add_fmt(str, "%*s", (level * 4) - 2, "");

		if (HASH_KEY_IS_NUMERIC(hash_key)) { /* numeric key */
			xdebug_str_add_fmt(str, XDEBUG_INT_FMT " <font color='%s'>=&gt;</font> ", index_key, COLOR_POINTER);
		} else { /* string key */
			xdebug_str_addc(str, '\'');
			tmp_str = xdebug_xmlize((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key) - 1, &newlen);
			xdebug_str_addl(str, tmp_str, newlen, 0);
			efree(tmp_str);
			xdebug_str_add_fmt(str, "' <font color='%s'>=&gt;</font> ", COLOR_POINTER);
		}
		xdebug_var_export_html(zv, str, level + 1, debug_zval, options);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add_fmt(str, "%*s", (level * 4) - 2, "");
		xdebug_str_add_literal(str, "<i>more elements...</i>\n");
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
		xdebug_str_add_fmt(str, "%*s", (level * 4) - 2, "");

		if (!HASH_KEY_IS_NUMERIC(hash_key)) {
			char       *prop_class_name;
			xdebug_str *property_name;
			xdebug_str *property_type = NULL;
			const char *modifier;

#if PHP_VERSION_ID >= 70400
			property_type = xdebug_get_property_type(object, zv_nptr);
#endif
			property_name = xdebug_get_property_info((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key), &modifier, &prop_class_name);

			xdebug_str_add_fmt(str, "<i>%s</i> ", modifier);
			if (property_type) {
				xdebug_str_add_fmt(str, "<i>%s</i> ", property_type->d);
			}
			xdebug_str_addc(str, '\'');
			xdebug_str_add_str(str, property_name);
			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				xdebug_str_add_fmt(str, "' <font color='%s'>=&gt;</font> ", COLOR_POINTER);
			} else {
				xdebug_str_add_fmt(str, "' <small>(%s)</small> <font color='%s'>=&gt;</font> ", prop_class_name, COLOR_POINTER);
			}

			if (property_type) {
				xdebug_str_free(property_type);
			}
			xdebug_str_free(property_name);
			xdfree(prop_class_name);
		} else {
			xdebug_str_add_fmt(str, "<i>public</i> " XDEBUG_INT_FMT " <font color='%s'>=&gt;</font> ", index_key, COLOR_POINTER);
		}
		xdebug_var_export_html(zv, str, level + 1, debug_zval, options);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add_fmt(str, "%*s", (level * 4) - 2, "");
		xdebug_str_add_literal(str, "<i>more elements...</i>\n");
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
	int   z_type;

	z_type = Z_TYPE_P(*struc);

	if (debug_zval) {
		xdebug_add_variable_attributes(str, *struc, XDEBUG_VAR_ATTR_HTML);
	}
	if (z_type == IS_INDIRECT) {
		tmpz = Z_INDIRECT_P(*struc);
		struc = &tmpz;
		z_type = Z_TYPE_P(*struc);
	}
	if (z_type == IS_REFERENCE) {
		tmpz = &((*struc)->value.ref->val);
		struc = &tmpz;
		z_type = Z_TYPE_P(*struc);
	}

	switch (z_type) {
		case IS_TRUE:
			xdebug_str_add_fmt(str, "<small>boolean</small> <font color='%s'>true</font>", COLOR_BOOL);
			break;

		case IS_FALSE:
			xdebug_str_add_fmt(str, "<small>boolean</small> <font color='%s'>false</font>", COLOR_BOOL);
			break;

		case IS_NULL:
			xdebug_str_add_fmt(str, "<font color='%s'>null</font>", COLOR_NULL);
			break;

		case IS_LONG:
			xdebug_str_add_fmt(str, "<small>int</small> <font color='%s'>" XDEBUG_INT_FMT "</font>", COLOR_LONG, Z_LVAL_P(*struc));
			break;

		case IS_DOUBLE:
			xdebug_str_add_fmt(str, "<small>float</small> <font color='%s'>%.*G</font>", COLOR_DOUBLE, (int) EG(precision), Z_DVAL_P(*struc));
			break;

		case IS_STRING:
			xdebug_str_add_fmt(str, "<small>string</small> <font color='%s'>'", COLOR_STRING);
			if ((size_t) Z_STRLEN_P(*struc) > (size_t) options->max_data) {
				tmp_str = xdebug_xmlize(Z_STRVAL_P(*struc), options->max_data, &newlen);
				xdebug_str_addl(str, tmp_str, newlen, 0);
				efree(tmp_str);
				xdebug_str_add_literal(str, "'...</font>");
			} else {
				tmp_str = xdebug_xmlize(Z_STRVAL_P(*struc), Z_STRLEN_P(*struc), &newlen);
				xdebug_str_addl(str, tmp_str, newlen, 0);
				efree(tmp_str);
				xdebug_str_add_literal(str, "'</font>");
			}
			xdebug_str_add_fmt(str, " <i>(length=%d)</i>", Z_STRLEN_P(*struc));
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);
			xdebug_str_add_fmt(str, "\n%*s", (level - 1) * 4, "");

			if (!xdebug_zend_hash_is_recursive(myht)) {
				xdebug_str_add_fmt(str, "<b>array</b> <i>(size=%d)</i>\n", myht->nNumOfElements);
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
						xdebug_str_add_fmt(str, "%*s", (level * 4) - 2, "");
						xdebug_str_add_fmt(str, "<i><font color='%s'>empty</font></i>\n", COLOR_EMPTY);
					}
				} else {
					xdebug_str_add_fmt(str, "%*s...\n", (level * 4) - 2, "");
				}
			} else {
				xdebug_str_add_literal(str, "<i>&amp;</i><b>array</b>\n");
			}
			break;

		case IS_OBJECT:
#if PHP_VERSION_ID >= 70400
			myht = xdebug_objdebug_pp(struc);
#else
			myht = xdebug_objdebug_pp(struc, &is_temp);
#endif
			xdebug_str_add_fmt(str, "\n%*s", (level - 1) * 4, "");

			if (!myht || !xdebug_zend_hash_is_recursive(myht)) {
				xdebug_str_add_literal(str, "<b>object</b>(<i>");
				xdebug_str_add(str, ZSTR_VAL(Z_OBJCE_P(*struc)->name), 0);
				xdebug_str_add_literal(str, "</i>)");
				xdebug_str_add_fmt(str, "[<i>%d</i>]\n", Z_OBJ_HANDLE_P(*struc));

				if (myht && (level <= options->max_depth)) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					xdebug_zend_hash_apply_protection_begin(myht);

					ZEND_HASH_FOREACH_KEY_VAL(myht, num, key, val) {
						xdebug_object_element_export_html(*struc, val, num, key, level, str, debug_zval, options, ZSTR_VAL(Z_OBJCE_P(*struc)->name));
					} ZEND_HASH_FOREACH_END();

					xdebug_zend_hash_apply_protection_end(myht);
				} else {
					xdebug_str_add_fmt(str, "%*s...\n", (level * 4) - 2, "");
				}
			} else {
				xdebug_str_add_literal(str, "<i>&amp;</i><b>object</b>(<i>");
				xdebug_str_add(str, ZSTR_VAL(Z_OBJCE_P(*struc)->name), 0);
				xdebug_str_add_literal(str, "</i>)");
				xdebug_str_add_fmt(str, "[<i>%d</i>]\n", Z_OBJ_HANDLE_P(*struc));
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
			xdebug_str_add_fmt(str, "<b>resource</b>(<i>%ld</i><font color='%s'>,</font> <i>%s</i>)", Z_RES_P(*struc)->handle, COLOR_RESOURCE, type_name ? type_name : "Unknown");
			break;
		}

		case IS_UNDEF:
			xdebug_str_add_fmt(str, "<font color='%s'>*uninitialized*</font>", COLOR_NULL);
			break;

		default:
			xdebug_str_add_fmt(str, "<font color='%s'>NFC</font>", COLOR_NULL);
			break;
	}
	if (z_type != IS_ARRAY && z_type != IS_OBJECT) {
		xdebug_str_addc(str, '\n');
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

	xdebug_str_add_literal(str, "<pre class='xdebug-var-dump' dir='ltr'>");
	if (options->show_location && !debug_zval) {
		char *formatted_filename;
		xdebug_format_filename(&formatted_filename, "%f", zend_get_executed_filename_ex());

		if (strlen(XINI_LIB(file_link_format)) > 0 && strcmp(zend_get_executed_filename(), "Unknown") != 0) {
			char *file_link;

			xdebug_format_file_link(&file_link, zend_get_executed_filename(), zend_get_executed_lineno());
			xdebug_str_add_fmt(str, "\n<small><a href='%s'>%s:%d</a>:</small>", file_link, formatted_filename, zend_get_executed_lineno());
			xdfree(file_link);
		} else {
			xdebug_str_add_fmt(str, "\n<small>%s:%d:</small>", formatted_filename, zend_get_executed_lineno());
		}

		xdfree(formatted_filename);
	}
	xdebug_var_export_html(&val, str, 1, debug_zval, options);
	xdebug_str_add_literal(str, "</pre>");

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
	int   z_type;

	z_type = Z_TYPE_P(*struc);

	if (debug_zval) {
		xdebug_add_variable_attributes(str, *struc, XDEBUG_VAR_ATTR_HTML);
	}
	if (z_type == IS_REFERENCE) {
		tmpz = &((*struc)->value.ref->val);
		struc = &tmpz;
	}

	switch (z_type) {
		case IS_TRUE:
			xdebug_str_add_fmt(str, "<font color='%s'>true</font>", COLOR_BOOL);
			break;

		case IS_FALSE:
			xdebug_str_add_fmt(str, "<font color='%s'>false</font>", COLOR_BOOL);
			break;

		case IS_NULL:
			xdebug_str_add_fmt(str, "<font color='%s'>null</font>", COLOR_NULL);
			break;

		case IS_LONG:
			xdebug_str_add_fmt(str, "<font color='%s'>long</font>", COLOR_LONG);
			break;

		case IS_DOUBLE:
			xdebug_str_add_fmt(str, "<font color='%s'>double</font>", COLOR_DOUBLE);
			break;

		case IS_STRING:
			xdebug_str_add_fmt(str, "<font color='%s'>string(%d)</font>", COLOR_STRING, Z_STRLEN_P(*struc));
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);
			xdebug_str_add_fmt(str, "<font color='%s'>array(%d)</font>", COLOR_ARRAY, myht->nNumOfElements);
			break;

		case IS_OBJECT:
			xdebug_str_add_fmt(str, "<font color='%s'>object(%s)[%d]</font>", COLOR_OBJECT, ZSTR_VAL(Z_OBJCE_P(*struc)->name), Z_OBJ_HANDLE_P(*struc));
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc));
			xdebug_str_add_fmt(str, "<font color='%s'>resource(%ld, %s)</font>", COLOR_RESOURCE, Z_RES_P(*struc)->handle, type_name ? type_name : "Unknown");
			break;
		}

		case IS_UNDEF:
			xdebug_str_add_fmt(str, "<font color='%s'>*uninitialized*</font>", COLOR_NULL);
			break;

		default:
			xdebug_str_add_fmt(str, "<font color='%s'>NFC</font>", COLOR_NULL);
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
