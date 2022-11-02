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

#include "var_export_line.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static int xdebug_array_element_export(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, xdebug_str *str, int debug_zval, xdebug_var_export_options *options)
{
	zval **zv = &zv_nptr;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		if (HASH_KEY_IS_NUMERIC(hash_key)) { /* numeric key */
			xdebug_str_add_fmt(str, XDEBUG_INT_FMT " => ", index_key);
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
			xdebug_str_add_literal(str, "' => ");
		}
		xdebug_var_export_line(zv, str, level + 2, debug_zval, options);
		xdebug_str_add_literal(str, ", ");
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add_literal(str, "..., ");
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export(zval *object, zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, xdebug_str *str, int debug_zval, xdebug_var_export_options *options, char *class_name)
{
	zval **zv = &zv_nptr;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		if (!HASH_KEY_IS_NUMERIC(hash_key)) {
			xdebug_str *property_name;
			xdebug_str *property_type = NULL;
			char       *prop_class_name;
			const char *modifier;

			property_type = xdebug_get_property_type(object, zv_nptr);
			property_name = xdebug_get_property_info((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key), &modifier, &prop_class_name);
			xdebug_str_add(str, modifier, 0);
			if (property_type) {
				xdebug_str_addc(str, ' ');
				xdebug_str_add_str(str, property_type);
			}
			xdebug_str_add_literal(str, " $");
			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				xdebug_str_add_str(str, property_name);
				xdebug_str_add_literal(str, " = ");
			} else {
				xdebug_str_addc(str, '{');
				xdebug_str_add(str, prop_class_name, 0);
				xdebug_str_addc(str, '}');
				xdebug_str_add_str(str, property_name);
				xdebug_str_add_literal(str, " = ");
			}

			if (property_type) {
				xdebug_str_free(property_type);
			}
			xdebug_str_free(property_name);
			xdfree(prop_class_name);
		} else {
			xdebug_str_add_fmt(str, "public $%d = ", index_key);
		}
		xdebug_var_export_line(zv, str, level + 2, debug_zval, options);
		xdebug_str_add_literal(str, "; ");
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add_literal(str, "...; ");
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

#if PHP_VERSION_ID < 80200
static void handle_closure(xdebug_str *str, zval *obj)
{
	const zend_function *closure_function;

	if (Z_TYPE_P(obj) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(obj), zend_ce_closure)) {
		return;
	}

	closure_function = zend_get_closure_method_def(Z_OBJ_P(obj));

	xdebug_str_add_literal(str, "virtual $closure = \"");

	if (closure_function->common.scope) {
		if (closure_function->common.fn_flags & ZEND_ACC_STATIC) {
			xdebug_str_add(str, ZSTR_VAL(closure_function->common.scope->name), 0);
			xdebug_str_add_literal(str, "::");
		} else {
			xdebug_str_add_literal(str, "$this->");
		}
	}
	xdebug_str_add_zstr(str, closure_function->common.function_name);

	xdebug_str_add_literal(str, "\", ");
}
#endif

void xdebug_var_export_line(zval **struc, xdebug_str *str, int level, int debug_zval, xdebug_var_export_options *options)
{
	HashTable *myht;
	zend_ulong num;
	zend_string *key;
	zval *val;
	zval *tmpz;
	int   z_type;

	if (!struc || !(*struc)) {
		return;
	}

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
			xdebug_str_add_literal(str, "TRUE");
			break;

		case IS_FALSE:
			xdebug_str_add_literal(str, "FALSE");
			break;

		case IS_NULL:
			xdebug_str_add_literal(str, "NULL");
			break;

		case IS_LONG:
			xdebug_str_add_fmt(str, XDEBUG_INT_FMT, Z_LVAL_P(*struc));
			break;

		case IS_DOUBLE:
			xdebug_str_add_fmt(str, "%.*H", (int) EG(precision), Z_DVAL_P(*struc));
			break;

		case IS_STRING: {
			if (options->no_decoration) {
				xdebug_str_add_zstr(str, Z_STR_P(*struc));
			} else if ((size_t) Z_STRLEN_P(*struc) <= (size_t) options->max_data) {
				zend_string *tmp_zstr;

				tmp_zstr = php_addcslashes(Z_STR_P(*struc), (char*) "'\\\0..\37", 7);

				xdebug_str_addc(str, '\'');
				xdebug_str_add_zstr(str, tmp_zstr);
				xdebug_str_addc(str, '\'');

				zend_string_release(tmp_zstr);
			} else {
				zend_string *tmp_zstr = php_addcslashes_str(ZSTR_VAL(Z_STR_P(*struc)), options->max_data, (char*) "'\\\0..\37", 7);

				xdebug_str_addc(str, '\'');
				xdebug_str_add_zstr(str, tmp_zstr);
				xdebug_str_add_literal(str, "\'...");

				zend_string_release(tmp_zstr);
			}
		} break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);

			if (!xdebug_zend_hash_is_recursive(myht)) {
				if (debug_zval) {
					xdebug_str_add_literal(str, "array (");
				} else {
					xdebug_str_addc(str, '[');
				}
				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					xdebug_zend_hash_apply_protection_begin(myht);

					ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
						xdebug_array_element_export(val, num, key, level, str, debug_zval, options);
					} ZEND_HASH_FOREACH_END();

					xdebug_zend_hash_apply_protection_end(myht);

					/* Remove the ", " at the end of the string */
					if (myht->nNumOfElements > 0) {
						xdebug_str_chop(str, 2);
					}
				} else {
					xdebug_str_add_literal(str, "...");
				}
				xdebug_str_addc(str, debug_zval ? ')' : ']');
			} else {
				xdebug_str_add_literal(str, "...");
			}
			break;

		case IS_OBJECT: {
#if PHP_VERSION_ID >= 80100
			zend_class_entry *ce = Z_OBJCE_P(*struc);
			if (ce->ce_flags & ZEND_ACC_ENUM) {
				zval *case_name_zval = zend_enum_fetch_case_name(Z_OBJ_P(*struc));
				xdebug_str_add_fmt(str, "enum %s::%s", ZSTR_VAL(ce->name), Z_STRVAL_P(case_name_zval));

				if (ce->enum_backing_type != IS_UNDEF) {
					zval *value = zend_enum_fetch_case_value(Z_OBJ_P(*struc));

					if (ce->enum_backing_type == IS_LONG) {
						xdebug_str_add_fmt(str, "(" XDEBUG_INT_FMT ")", Z_LVAL_P(value));
					}

					if (ce->enum_backing_type == IS_STRING) {
						zend_string *tmp_zstr = php_addcslashes(Z_STR_P(value), (char*) "'\\\0..\37", 7);

						xdebug_str_add_literal(str, "('");
						xdebug_str_add_zstr(str, tmp_zstr);
						xdebug_str_add_literal(str, "')");

						zend_string_release(tmp_zstr);
					}
				}

				break;
			}
#endif

			myht = xdebug_objdebug_pp(struc, XDEBUG_VAR_OBJDEBUG_USE_DEBUGINFO);

			if (!myht || !xdebug_zend_hash_is_recursive(myht)) {
				xdebug_str_add_literal(str, "class ");
				xdebug_str_add(str, ZSTR_VAL(Z_OBJCE_P(*struc)->name), 0);
				xdebug_str_add_literal(str, " { ");

#if PHP_VERSION_ID < 80200
				handle_closure(str, *struc);
#endif

				if (myht && (level <= options->max_depth)) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					xdebug_zend_hash_apply_protection_begin(myht);

					ZEND_HASH_FOREACH_KEY_VAL(myht, num, key, val) {
						xdebug_object_element_export(*struc, val, num, key, level, str, debug_zval, options, ZSTR_VAL(Z_OBJCE_P(*struc)->name));
					} ZEND_HASH_FOREACH_END();

					xdebug_zend_hash_apply_protection_end(myht);
				} else {
					xdebug_str_add_literal(str, "...");
				}
				/* Remove the ", " and "; " at the end of the string */
				if (
					str->l > 2 &&
					((str->d[str->l - 2] == ',') || (str->d[str->l - 2] == ';' )) &&
					(str->d[str->l - 1] == ' ')
				) {
					xdebug_str_chop(str, 2);
				}

				xdebug_str_add_literal(str, " }");
			} else {
				xdebug_str_add_literal(str, "...");
			}
			zend_release_properties(myht);
			break;
		}

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc));
			xdebug_str_add_fmt(str, "resource(%ld) of type (%s)", Z_RES_P(*struc)->handle, type_name ? type_name : "Unknown");
			break;
		}

		case IS_UNDEF:
			xdebug_str_add_literal(str, "*uninitialized*");
			break;

		default:
			xdebug_str_add_literal(str, "NFC");
			break;
	}
}

xdebug_str* xdebug_get_zval_value_line(zval *val, int debug_zval, xdebug_var_export_options *options)
{
	xdebug_str *str = xdebug_str_new();
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini();
		default_options = 1;
	}

	xdebug_var_export_line(&val, str, 1, debug_zval, options);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	return str;
}

static void xdebug_var_synopsis(zval **struc, xdebug_str *str, int level, int debug_zval, xdebug_var_export_options *options)
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
			xdebug_str_add_literal(str, "true");
			break;

		case IS_FALSE:
			xdebug_str_add_literal(str, "false");
			break;

		case IS_NULL:
			xdebug_str_add_literal(str, "null");
			break;

		case IS_LONG:
			xdebug_str_add_literal(str, "long");
			break;

		case IS_DOUBLE:
			xdebug_str_add_literal(str, "double");
			break;

		case IS_STRING:
			xdebug_str_add_fmt(str, "string(%d)", Z_STRLEN_P(*struc));
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);
			xdebug_str_add_fmt(str, "array(%d)", myht->nNumOfElements);
			break;

		case IS_OBJECT: {
			xdebug_str_add_literal(str, "class ");
			xdebug_str_add(str, ZSTR_VAL(Z_OBJCE_P(*struc)->name), 0);
			break;
		}

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc));
			xdebug_str_add_fmt(str, "resource(%ld) of type (%s)", Z_RES_P(*struc)->handle, type_name ? type_name : "Unknown");
			break;
		}

		case IS_UNDEF:
			xdebug_str_add_literal(str, "*uninitialized*");
			break;

		default:
			xdebug_str_add_literal(str, "NFC");
			break;

	}
}

xdebug_str* xdebug_get_zval_synopsis_line(zval *val, int debug_zval, xdebug_var_export_options *options)
{
	xdebug_str *str = xdebug_str_new();
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini();
		default_options = 1;
	}

	xdebug_var_synopsis(&val, str, 1, debug_zval, options);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	return str;
}

