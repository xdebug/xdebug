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

#include "php.h"
#include "ext/standard/php_string.h"

#include "php_xdebug.h"

#include "var_export_text.h"

static void xdebug_var_export_text_ansi(zval **struc, xdebug_str *str, int mode, int level, int debug_zval, xdebug_var_export_options *options);
#define xdebug_var_export_text(struc, str, level, debug_zval, options) xdebug_var_export_text_ansi(struc, str, 0, level, debug_zval, options);
#define xdebug_var_export_ansi(struc, str, level, debug_zval, options) xdebug_var_export_text_ansi(struc, str, 1, level, debug_zval, options);

ZEND_EXTERN_MODULE_GLOBALS(xdebug)


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

static int xdebug_array_element_export_text_ansi(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, int mode, xdebug_str *str, int debug_zval, xdebug_var_export_options *options)
{
	zval **zv = &zv_nptr;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 2), ""), 1);

		if (HASH_KEY_IS_NUMERIC(hash_key)) { /* numeric key */
			xdebug_str_add(str, xdebug_sprintf("[" XDEBUG_INT_FMT "] %s=>%s\n", index_key, ANSI_COLOR_POINTER, ANSI_COLOR_RESET), 1);
		} else { /* string key */
			size_t newlen = 0;
			char *tmp, *tmp2;

			tmp = xdebug_str_to_str((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key), "'", 1, "\\'", 2, &newlen);
			tmp2 = xdebug_str_to_str(tmp, newlen - 1, "\0", 1, "\\0", 2, &newlen);
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
		xdebug_var_export_text_ansi(zv, str, mode, level + 1, debug_zval, options);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add(str, xdebug_sprintf("\n%*s(more elements)...\n", (level * 2), ""), 1);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export_text_ansi(zval *object, zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, int mode, xdebug_str *str, int debug_zval, xdebug_var_export_options *options)
{
	zval **zv = &zv_nptr;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 2), ""), 1);

		if (!HASH_KEY_IS_NUMERIC(hash_key)) {
			char       *class_name;
			xdebug_str *property_name;
			const char *modifier;
			xdebug_str *property_type = NULL;

#if PHP_VERSION_ID >= 70400
			property_type = xdebug_get_property_type(object, zv_nptr);
#endif

			property_name = xdebug_get_property_info((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key), &modifier, &class_name);
			xdebug_str_add(str, xdebug_sprintf("%s%s%s%s%s%s%s $",
			               ANSI_COLOR_MODIFIER, ANSI_COLOR_BOLD, modifier, ANSI_COLOR_BOLD_OFF, property_type ? " " : "", property_type ? property_type->d : "", ANSI_COLOR_RESET), 1);
			xdebug_str_add_str(str, property_name);
			xdebug_str_add(str, xdebug_sprintf(" %s=>%s\n",
			               ANSI_COLOR_POINTER, ANSI_COLOR_RESET), 1);

			if (property_type) {
				xdebug_str_free(property_type);
			}
			xdebug_str_free(property_name);
			xdfree(class_name);
		} else {
			xdebug_str_add(str, xdebug_sprintf("%s%spublic%s%s ${" XDEBUG_INT_FMT "} %s=>%s\n",
			               ANSI_COLOR_MODIFIER, ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_RESET,
			               index_key, ANSI_COLOR_POINTER, ANSI_COLOR_RESET), 1);
		}
		xdebug_var_export_text_ansi(zv, str, mode, level + 1, debug_zval, options);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add(str, xdebug_sprintf("\n%*s(more elements)...\n", (level * 2), ""), 1);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static void xdebug_var_export_text_ansi(zval **struc, xdebug_str *str, int mode, int level, int debug_zval, xdebug_var_export_options *options)
{
	HashTable *myht;
	char*     tmp_str;
	int       tmp_len;
#if PHP_VERSION_ID < 70400
	int       is_temp;
#endif
	zend_ulong num;
	zend_string *key;
	zval *val;
	zval *tmpz;

	if (!struc || !(*struc)) {
		return;
	}

	xdebug_str_add(str, xdebug_sprintf("%*s", (level * 2) - 2, ""), 1);

	if (debug_zval) {
		xdebug_add_variable_attributes(str, *struc, XDEBUG_VAR_ATTR_TEXT);
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
			xdebug_str_add(str, xdebug_sprintf("%sbool%s(%s%s%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_BOOL, Z_TYPE_P(*struc) == IS_TRUE ? "true" : "false", ANSI_COLOR_RESET), 1);
			break;

		case IS_NULL:
			xdebug_str_add(str, xdebug_sprintf("%s%sNULL%s%s", ANSI_COLOR_BOLD, ANSI_COLOR_NULL, ANSI_COLOR_RESET, ANSI_COLOR_BOLD_OFF), 1);
			break;

		case IS_LONG:
			xdebug_str_add(str, xdebug_sprintf("%sint%s(%s" XDEBUG_INT_FMT "%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_LONG, Z_LVAL_P(*struc), ANSI_COLOR_RESET), 1);
			break;

		case IS_DOUBLE:
			xdebug_str_add(str, xdebug_sprintf("%sdouble%s(%s%.*G%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_DOUBLE, (int) EG(precision), Z_DVAL_P(*struc), ANSI_COLOR_RESET), 1);
			break;

		case IS_STRING: {
			const char *pattern = (mode == 1) ? "'\\\0..\37" : "\0";
			size_t pattern_len = (mode == 1) ? 7 : 1;

			zend_string *i_string = zend_string_init(Z_STRVAL_P(*struc), Z_STRLEN_P(*struc), 0);
			zend_string *tmp_zstr;

#if PHP_VERSION_ID >= 70300
			tmp_zstr = php_addcslashes(i_string, (char*) pattern, pattern_len);
#else
			tmp_zstr = php_addcslashes(i_string, 0, (char*) pattern, pattern_len);
#endif

			tmp_str = estrndup(tmp_zstr->val, tmp_zstr->len);
			tmp_len = tmp_zstr->len;
			zend_string_release(tmp_zstr);
			zend_string_release(i_string);

			if (options->no_decoration) {
				xdebug_str_addl(str, tmp_str, tmp_len, 0);
			} else if ((size_t) Z_STRLEN_P(*struc) <= (size_t) options->max_data) {
				xdebug_str_add(str, xdebug_sprintf("%sstring%s(%s%ld%s) \"%s%s%s\"",
					ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF,
					ANSI_COLOR_LONG, Z_STRLEN_P(*struc), ANSI_COLOR_RESET,
					ANSI_COLOR_STRING, tmp_str, ANSI_COLOR_RESET), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("%sstring%s(%s%ld%s) \"%s",
					ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF,
					ANSI_COLOR_LONG, Z_STRLEN_P(*struc), ANSI_COLOR_RESET, ANSI_COLOR_STRING), 1);
				xdebug_str_addl(str, tmp_str, options->max_data, 0);
				xdebug_str_add(str, xdebug_sprintf("%s\"...", ANSI_COLOR_RESET), 1);
			}
			efree(tmp_str);
		} break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);

			if (!xdebug_zend_hash_is_recursive(myht)) {
				xdebug_str_add(str, xdebug_sprintf("%sarray%s(%s%d%s) {\n", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_LONG, myht->nNumOfElements, ANSI_COLOR_RESET), 1);
				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					xdebug_zend_hash_apply_protection_begin(myht);

					ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
						xdebug_array_element_export_text_ansi(val, num, key, level, mode, str, debug_zval, options);
					} ZEND_HASH_FOREACH_END();

					xdebug_zend_hash_apply_protection_end(myht);
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 2), ""), 1);
				}
				xdebug_str_add(str, xdebug_sprintf("%*s}", (level * 2) - 2 , ""), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("&%sarray%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF), 1);
			}
			break;

		case IS_OBJECT:
#if PHP_VERSION_ID >= 70400
			myht = xdebug_objdebug_pp(struc);
#else
			myht = xdebug_objdebug_pp(struc, &is_temp);
#endif

			if (!myht || !xdebug_zend_hash_is_recursive(myht)) {
				xdebug_str_add(str, xdebug_sprintf("%sclass%s %s%s%s#%d (%s%d%s) {\n",
					ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF,
					ANSI_COLOR_OBJECT, STR_NAME_VAL(Z_OBJCE_P(*struc)->name), ANSI_COLOR_RESET,
					Z_OBJ_HANDLE_P(*struc),
					ANSI_COLOR_LONG, myht ? myht->nNumOfElements : 0, ANSI_COLOR_RESET), 1);

				if (myht && (level <= options->max_depth)) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					xdebug_zend_hash_apply_protection_begin(myht);

					ZEND_HASH_FOREACH_KEY_VAL(myht, num, key, val) {
						xdebug_object_element_export_text_ansi(*struc, val, num, key, level, mode, str, debug_zval, options);
					} ZEND_HASH_FOREACH_END();

					xdebug_zend_hash_apply_protection_end(myht);
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 2), ""), 1);
				}
				xdebug_str_add(str, xdebug_sprintf("%*s}", (level * 2) - 2, ""), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 2), ""), 1);
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
			xdebug_str_add(str, xdebug_sprintf("%sresource%s(%s%ld%s) of type (%s)",
				ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF,
				ANSI_COLOR_RESOURCE, Z_RES_P(*struc)->handle, ANSI_COLOR_RESET, type_name ? type_name : "Unknown"), 1);
			break;
		}

		case IS_UNDEF:
			xdebug_str_add(str, xdebug_sprintf("%s*uninitialized*%s", ANSI_COLOR_NULL, ANSI_COLOR_RESET), 0);
			break;

		default:
			xdebug_str_add(str, xdebug_sprintf("%sNFC%s", ANSI_COLOR_NULL, ANSI_COLOR_RESET), 0);
			break;
	}

	xdebug_str_addl(str, "\n", 1, 0);
}

xdebug_str* xdebug_get_zval_value_text_ansi(zval *val, int mode, int debug_zval, xdebug_var_export_options *options)
{
	xdebug_str *str = xdebug_str_new();
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini();
		default_options = 1;
	}

	if (options->show_location && !debug_zval) {
		char *formatted_filename;

		xdebug_format_filename(&formatted_filename, XINI_BASE(filename_format), "%f", zend_get_executed_filename());
		xdebug_str_add(str, xdebug_sprintf("%s%s%s:%s%d%s:\n", ANSI_COLOR_BOLD, formatted_filename, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_BOLD, zend_get_executed_lineno(), ANSI_COLOR_BOLD_OFF), 1);
		xdfree(formatted_filename);
	}

	xdebug_var_export_text_ansi(&val, str, mode, 1, debug_zval, options);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	return str;
}

static void xdebug_var_synopsis_text_ansi(zval **struc, xdebug_str *str, int mode, int level, int debug_zval, xdebug_var_export_options *options)
{
	HashTable *myht;
	zval *tmpz;

	if (!struc || !(*struc)) {
		return;
	}

	if (debug_zval) {
		xdebug_add_variable_attributes(str, *struc, XDEBUG_VAR_ATTR_TEXT);
	}
	if (Z_TYPE_P(*struc) == IS_REFERENCE) {
		tmpz = &((*struc)->value.ref->val);
		struc = &tmpz;
	}

	switch (Z_TYPE_P(*struc)) {
		case IS_TRUE:
			xdebug_str_add(str, xdebug_sprintf("%strue%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF), 1);
			break;

		case IS_FALSE:
			xdebug_str_add(str, xdebug_sprintf("%sfalse%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF), 1);
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
			xdebug_str_add(str, xdebug_sprintf("%sstring%s(%s%d%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_LONG, Z_STRLEN_P(*struc), ANSI_COLOR_RESET), 1);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);
			xdebug_str_add(str, xdebug_sprintf("array(%s%d%s)", ANSI_COLOR_LONG, myht->nNumOfElements, ANSI_COLOR_RESET), 1);
			break;

		case IS_OBJECT: {
			xdebug_str_add(str, xdebug_sprintf("class %s", STR_NAME_VAL(Z_OBJCE_P(*struc)->name)), 1);
			break;
		}

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc));
			xdebug_str_add(str, xdebug_sprintf("resource(%s%ld%s) of type (%s)", ANSI_COLOR_LONG, Z_RES_P(*struc)->handle, ANSI_COLOR_RESET, type_name ? type_name : "Unknown"), 1);
			break;
		}

		case IS_UNDEF:
			xdebug_str_add(str, xdebug_sprintf("%s*uninitialized*%s", ANSI_COLOR_NULL, ANSI_COLOR_RESET), 0);
			break;

		default:
			xdebug_str_add(str, xdebug_sprintf("%sNFC%s", ANSI_COLOR_NULL, ANSI_COLOR_RESET), 0);
			break;
	}
}

xdebug_str* xdebug_get_zval_synopsis_text_ansi(zval *val, int mode, int debug_zval, xdebug_var_export_options *options)
{
	xdebug_str *str = xdebug_str_new();
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini();
		default_options = 1;
	}

	if (options->show_location && !debug_zval) {
		xdebug_str_add(str, xdebug_sprintf("%s%s: %d%s\n", ANSI_COLOR_BOLD, zend_get_executed_filename(), zend_get_executed_lineno(), ANSI_COLOR_BOLD_OFF), 1);
	}

	xdebug_var_synopsis_text_ansi(&val, str, mode, 1, debug_zval, options);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	return str;
}

