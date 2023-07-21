/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2022 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
 */

#include "lib/php-header.h"
#include "ext/standard/php_string.h"
#include "Zend/zend_closures.h"
#if PHP_VERSION_ID >= 80100
# include "zend_enum.h"
#endif


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
		xdebug_str_add_fmt(str, "%*s", (level * 2), "");

		if (HASH_KEY_IS_NUMERIC(hash_key)) { /* numeric key */
			xdebug_str_add_fmt(str, "[" XDEBUG_INT_FMT "] %s=>%s\n", index_key, ANSI_COLOR_POINTER, ANSI_COLOR_RESET);
		} else { /* string key */
			zend_string *tmp, *tmp2;

			tmp = php_str_to_str(ZSTR_VAL(hash_key), ZSTR_LEN(hash_key), (char*) "'", 1, (char*) "\\'", 2);
			tmp2 = php_str_to_str(ZSTR_VAL(tmp), ZSTR_LEN(tmp), (char*) "\0", 1, (char*) "\\0", 2);
			if (tmp) {
				zend_string_release(tmp);
			}
			xdebug_str_addc(str, '\'');
			if (tmp2) {
				xdebug_str_add_zstr(str, tmp2);
				zend_string_release(tmp2);
			}
			xdebug_str_add_literal(str, "' =>\n");
		}
		xdebug_var_export_text_ansi(zv, str, mode, level + 1, debug_zval, options);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add_fmt(str, "\n%*s(more elements)...\n", (level * 2), "");
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
		xdebug_str_add_fmt(str, "%*s", (level * 2), "");

		if (!HASH_KEY_IS_NUMERIC(hash_key)) {
			char       *class_name;
			xdebug_str *property_name;
			const char *modifier;
			xdebug_str *property_type = NULL;

			property_type = xdebug_get_property_type(object, zv_nptr);

			property_name = xdebug_get_property_info((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key), &modifier, &class_name);
			xdebug_str_add_fmt(
				str, "%s%s%s%s%s%s%s $",
				ANSI_COLOR_MODIFIER, ANSI_COLOR_BOLD, modifier, ANSI_COLOR_BOLD_OFF, property_type ? " " : "", property_type ? property_type->d : "", ANSI_COLOR_RESET
			);
			xdebug_str_add_str(str, property_name);
			xdebug_str_add_fmt(str, " %s=>%s\n", ANSI_COLOR_POINTER, ANSI_COLOR_RESET);

			if (property_type) {
				xdebug_str_free(property_type);
			}
			xdebug_str_free(property_name);
			xdfree(class_name);
		} else {
			xdebug_str_add_fmt(str, "%s%spublic%s%s ${" XDEBUG_INT_FMT "} %s=>%s\n",
				ANSI_COLOR_MODIFIER, ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_RESET,
				index_key, ANSI_COLOR_POINTER, ANSI_COLOR_RESET
			);
		}
		xdebug_var_export_text_ansi(zv, str, mode, level + 1, debug_zval, options);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add_fmt(str, "\n%*s(more elements)...\n", (level * 2), "");
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

#if PHP_VERSION_ID < 80200
static void handle_closure(xdebug_str *str, zval *obj, int level, int mode)
{
	const zend_function *closure_function;

	if (Z_TYPE_P(obj) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(obj), zend_ce_closure)) {
		return;
	}

	closure_function = zend_get_closure_method_def(Z_OBJ_P(obj));

	xdebug_str_add_fmt(
		str, "%*s%s%svirtual%s $closure =>\n%*s\"",
		(level * 4) - 2, "",
		ANSI_COLOR_MODIFIER, ANSI_COLOR_BOLD, ANSI_COLOR_RESET,
		(level * 4) - 2, ""
	);

	if (closure_function->common.scope) {
		if (closure_function->common.fn_flags & ZEND_ACC_STATIC) {
			xdebug_str_add_fmt(str, "%s", ANSI_COLOR_OBJECT);
			xdebug_str_add_zstr(str, closure_function->common.scope->name);
			xdebug_str_add_fmt(str, "%s::", ANSI_COLOR_RESET);
		} else {
			xdebug_str_add_fmt(str, "%s$this%s->", ANSI_COLOR_OBJECT, ANSI_COLOR_RESET);
		}
	}

	xdebug_str_add_fmt(str, "%s", ANSI_COLOR_STRING);
	xdebug_str_add_zstr(str, closure_function->common.function_name);
	xdebug_str_add_fmt(str, "%s\"\n", ANSI_COLOR_RESET);
}
#endif

static void xdebug_var_export_text_ansi(zval **struc, xdebug_str *str, int mode, int level, int debug_zval, xdebug_var_export_options *options)
{
	HashTable *myht;
	char*     tmp_str;
	int       tmp_len;
	zend_ulong num;
	zend_string *key;
	zval *val;
	zval *tmpz;
	int   z_type;

	if (!struc || !(*struc)) {
		return;
	}

	xdebug_str_add_fmt(str, "%*s", (level * 2) - 2, "");

	z_type = Z_TYPE_P(*struc);

	if (debug_zval) {
		xdebug_add_variable_attributes(str, *struc, XDEBUG_VAR_ATTR_TEXT);
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
			xdebug_str_add_fmt(str, "%sbool%s(%s%s%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_BOOL, "true", ANSI_COLOR_RESET);
			break;

		case IS_FALSE:
			xdebug_str_add_fmt(str, "%sbool%s(%s%s%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_BOOL, "false", ANSI_COLOR_RESET);
			break;

		case IS_NULL:
			xdebug_str_add_fmt(str, "%s%sNULL%s%s", ANSI_COLOR_BOLD, ANSI_COLOR_NULL, ANSI_COLOR_RESET, ANSI_COLOR_BOLD_OFF);
			break;

		case IS_LONG:
			xdebug_str_add_fmt(str, "%sint%s(%s" XDEBUG_INT_FMT "%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_LONG, Z_LVAL_P(*struc), ANSI_COLOR_RESET);
			break;

		case IS_DOUBLE:
			xdebug_str_add_fmt(str, "%sdouble%s(%s%.*H%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_DOUBLE, (int) EG(precision), Z_DVAL_P(*struc), ANSI_COLOR_RESET);
			break;

		case IS_STRING: {
			const char *pattern = (mode == 1) ? "'\\\0..\37" : "\0";
			size_t pattern_len = (mode == 1) ? 7 : 1;

			zend_string *i_string = zend_string_init(Z_STRVAL_P(*struc), Z_STRLEN_P(*struc), 0);
			zend_string *tmp_zstr;

			tmp_zstr = php_addcslashes(i_string, (char*) pattern, pattern_len);

			tmp_str = estrndup(tmp_zstr->val, tmp_zstr->len);
			tmp_len = tmp_zstr->len;
			zend_string_release(tmp_zstr);
			zend_string_release(i_string);

			if (options->no_decoration) {
				xdebug_str_addl(str, tmp_str, tmp_len, 0);
			} else if ((size_t) Z_STRLEN_P(*struc) <= (size_t) options->max_data) {
				xdebug_str_add_fmt(
					str, "%sstring%s(%s%ld%s) \"%s%s%s\"",
					ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF,
					ANSI_COLOR_LONG, Z_STRLEN_P(*struc), ANSI_COLOR_RESET,
					ANSI_COLOR_STRING, tmp_str, ANSI_COLOR_RESET
				);
			} else {
				xdebug_str_add_fmt(
					str, "%sstring%s(%s%ld%s) \"%s",
					ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF,
					ANSI_COLOR_LONG, Z_STRLEN_P(*struc), ANSI_COLOR_RESET, ANSI_COLOR_STRING
				);
				xdebug_str_addl(str, tmp_str, options->max_data, 0);
				xdebug_str_add_fmt(str, "%s\"...", ANSI_COLOR_RESET);
			}
			efree(tmp_str);
		} break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);

			if (!xdebug_zend_hash_is_recursive(myht)) {
				xdebug_str_add_fmt(str, "%sarray%s(%s%d%s) {\n", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_LONG, myht->nNumOfElements, ANSI_COLOR_RESET);
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
					xdebug_str_add_fmt(str, "%*s...\n", (level * 2), "");
				}
				xdebug_str_add_fmt(str, "%*s}", (level * 2) - 2 , "");
			} else {
				xdebug_str_add_fmt(str, "&%sarray%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF);
			}
			break;

		case IS_OBJECT: {
#if PHP_VERSION_ID >= 80100
			zend_class_entry *ce = Z_OBJCE_P(*struc);
			if (ce->ce_flags & ZEND_ACC_ENUM) {
				zval *case_name_zval = zend_enum_fetch_case_name(Z_OBJ_P(*struc));
				xdebug_str_add_fmt(
					str, "%senum%s %s%s%s::%s%s%s",
					ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF,
					ANSI_COLOR_OBJECT, ZSTR_VAL(ce->name), ANSI_COLOR_RESET,
					ANSI_COLOR_STRING, Z_STRVAL_P(case_name_zval), ANSI_COLOR_RESET
				);
				if (ce->enum_backing_type != IS_UNDEF) {
					zval *value = zend_enum_fetch_case_value(Z_OBJ_P(*struc));

					if (ce->enum_backing_type == IS_LONG) {
						xdebug_str_add_fmt(
							str, " : %s%s%s(%s%d%s)",
							ANSI_COLOR_BOLD, "int", ANSI_COLOR_RESET,
							ANSI_COLOR_LONG, Z_LVAL_P(value), ANSI_COLOR_RESET
						);
					}

					if (ce->enum_backing_type == IS_STRING) {
						xdebug_str_add_fmt(
							str, " : %s%s%s(%s\"%s\"%s)",
							ANSI_COLOR_BOLD, "string", ANSI_COLOR_RESET,
							ANSI_COLOR_STRING, Z_STRVAL_P(value), ANSI_COLOR_RESET
						);
					}
				}
				xdebug_str_addc(str, ';');
				break;
			}
#endif

			myht = xdebug_objdebug_pp(struc, XDEBUG_VAR_OBJDEBUG_USE_DEBUGINFO);

			if (!myht || !xdebug_zend_hash_is_recursive(myht)) {
				xdebug_str_add_fmt(
					str, "%sclass%s %s%s%s#%d (%s%d%s) {\n",
					ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF,
					ANSI_COLOR_OBJECT, STR_NAME_VAL(Z_OBJCE_P(*struc)->name), ANSI_COLOR_RESET,
					Z_OBJ_HANDLE_P(*struc),
					ANSI_COLOR_LONG, myht ? myht->nNumOfElements : 0, ANSI_COLOR_RESET
				);

#if PHP_VERSION_ID < 80200
				handle_closure(str, *struc, level, mode);
#endif

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
					xdebug_str_add_fmt(str, "%*s...\n", (level * 2), "");
				}
				xdebug_str_add_fmt(str, "%*s}", (level * 2) - 2, "");
			} else {
				xdebug_str_add_fmt(str, "%*s...\n", (level * 2), "");
			}
			zend_release_properties(myht);
			break;
		}

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc));
			xdebug_str_add_fmt(
				str, "%sresource%s(%s%ld%s) of type (%s)",
				ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF,
				ANSI_COLOR_RESOURCE, Z_RES_P(*struc)->handle, ANSI_COLOR_RESET, type_name ? type_name : "Unknown"
			);
			break;
		}

		case IS_UNDEF:
			xdebug_str_add_fmt(str, "%s*uninitialized*%s", ANSI_COLOR_NULL, ANSI_COLOR_RESET);
			break;

		default:
			xdebug_str_add_fmt(str, "%sNFC%s", ANSI_COLOR_NULL, ANSI_COLOR_RESET);
			break;
	}

	xdebug_str_addc(str, '\n');
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

		xdebug_format_filename(&formatted_filename, "%f", zend_get_executed_filename_ex());
		xdebug_str_add_fmt(str, "%s%s%s:%s%d%s:\n", ANSI_COLOR_BOLD, formatted_filename, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_BOLD, zend_get_executed_lineno(), ANSI_COLOR_BOLD_OFF);
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
	int   z_type;

	if (!struc || !(*struc)) {
		return;
	}

	z_type = Z_TYPE_P(*struc);

	if (debug_zval) {
		xdebug_add_variable_attributes(str, *struc, XDEBUG_VAR_ATTR_TEXT);
	}
	if (z_type == IS_REFERENCE) {
		tmpz = &((*struc)->value.ref->val);
		struc = &tmpz;
		z_type = Z_TYPE_P(*struc);
	}

	switch (z_type) {
		case IS_TRUE:
			xdebug_str_add_fmt(str, "%strue%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF);
			break;

		case IS_FALSE:
			xdebug_str_add_fmt(str, "%sfalse%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF);
			break;

		case IS_NULL:
			xdebug_str_add_fmt(str, "%snull%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF);
			break;

		case IS_LONG:
			xdebug_str_add_fmt(str, "%sint%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF);
			break;

		case IS_DOUBLE:
			xdebug_str_add_fmt(str, "%sdouble%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF);
			break;

		case IS_STRING:
			xdebug_str_add_fmt(str, "%sstring%s(%s%d%s)", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_LONG, Z_STRLEN_P(*struc), ANSI_COLOR_RESET);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);
			xdebug_str_add_fmt(str, "array(%s%d%s)", ANSI_COLOR_LONG, myht->nNumOfElements, ANSI_COLOR_RESET);
			break;

		case IS_OBJECT:
			xdebug_str_add_fmt(str, "class %s", ZSTR_VAL(Z_OBJCE_P(*struc)->name));
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc));
			xdebug_str_add_fmt(str, "resource(%s%ld%s) of type (%s)", ANSI_COLOR_LONG, Z_RES_P(*struc)->handle, ANSI_COLOR_RESET, type_name ? type_name : "Unknown");
			break;
		}

		case IS_UNDEF:
			xdebug_str_add_fmt(str, "%s*uninitialized*%s", ANSI_COLOR_NULL, ANSI_COLOR_RESET);
			break;

		default:
			xdebug_str_add_fmt(str, "%sNFC%s", ANSI_COLOR_NULL, ANSI_COLOR_RESET);
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
		xdebug_str_add_fmt(str, "%s%s: %d%s\n", ANSI_COLOR_BOLD, zend_get_executed_filename(), zend_get_executed_lineno(), ANSI_COLOR_BOLD_OFF);
	}

	xdebug_var_synopsis_text_ansi(&val, str, mode, 1, debug_zval, options);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	return str;
}

