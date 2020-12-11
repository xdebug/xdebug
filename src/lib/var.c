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
   |          Nikita Popov <nikita.ppv@gmail.com>                         |
   +----------------------------------------------------------------------+
 */

#include "php.h"
#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "zend.h"
#include "zend_exceptions.h"
#include "zend_extensions.h"
#include "ext/standard/php_smart_string.h"
#include "zend_smart_str.h"

#include "php_xdebug.h"

#include "lib/compat.h"
#include "lib/lib_private.h"
#include "lib/mm.h"
#include "lib/var.h"
#include "lib/var_export_html.h"
#include "lib/var_export_line.h"
#include "lib/xml.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static inline int object_or_ancestor_is_internal(zval dzval)
{
	zend_class_entry *tmp_ce = Z_OBJCE(dzval);

	do {
		if (tmp_ce->type == ZEND_INTERNAL_CLASS) {
			return 1;
		}
		tmp_ce = tmp_ce->parent;
	} while (tmp_ce);

	return 0;
}
#if PHP_VERSION_ID >= 70400
HashTable *xdebug_objdebug_pp(zval **zval_pp)
#else
HashTable *xdebug_objdebug_pp(zval **zval_pp, int *is_tmp)
#endif
{
	zval dzval = **zval_pp;
	HashTable *tmp;

	if (!XG_BASE(in_debug_info) && object_or_ancestor_is_internal(dzval) && Z_OBJ_HANDLER(dzval, get_debug_info)) {
		void        *original_trace_context;
		zend_object *orig_exception;

		xdebug_tracing_save_trace_context(&original_trace_context);
		XG_BASE(in_debug_info) = 1;
		orig_exception = EG(exception);
		EG(exception) = NULL;

#if PHP_VERSION_ID >= 70400
		tmp = zend_get_properties_for(&dzval, ZEND_PROP_PURPOSE_DEBUG);
#else
		tmp = Z_OBJ_HANDLER(dzval, get_debug_info)(&dzval, is_tmp);
#endif
		XG_BASE(in_debug_info) = 0;
		xdebug_tracing_restore_trace_context(original_trace_context);
		EG(exception) = orig_exception;

		return tmp;
	} else {
#if PHP_VERSION_ID >= 70400
		return zend_get_properties_for(&dzval, ZEND_PROP_PURPOSE_VAR_EXPORT);
#else
		*is_tmp = 0;
		if (Z_OBJ_HANDLER(dzval, get_properties)) {
			return Z_OBJPROP(dzval);
		}
#endif
	}
	return NULL;
}

char* xdebug_error_type_simple(int type)
{
	switch (type) {
		case E_ERROR:
		case E_CORE_ERROR:
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			return xdstrdup("fatal-error");
			break;
		case E_RECOVERABLE_ERROR:
			return xdstrdup("recoverable-fatal-error");
			break;
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
		case E_DEPRECATED:
		case E_USER_DEPRECATED:
			return xdstrdup("deprecated");
			break;
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
		case E_RECOVERABLE_ERROR:
			return xdstrdup("Recoverable fatal error");
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
		case E_STRICT:
			return xdstrdup("Strict standards");
			break;
		case E_DEPRECATED:
		case E_USER_DEPRECATED:
			return xdstrdup("Deprecated");
			break;
		case 0:
			return xdstrdup("Xdebug");
			break;
		default:
			return xdstrdup("Unknown error");
			break;
	}
}

/*************************************************************************************************************************************/
#define T(offset) (*(union _temp_variable *)((char*)zdata->current_execute_data->Ts + offset))

zval *xdebug_get_zval_with_opline(zend_execute_data *zdata, const zend_op *opline, int node_type, const znode_op *node, int *is_var)
{
#if PHP_VERSION_ID >= 80000
	return zend_get_zval_ptr(opline, node_type, node, zdata);
#else
	zend_free_op should_free;

# if PHP_VERSION_ID >= 70300
	return zend_get_zval_ptr(opline, node_type, node, zdata, &should_free, BP_VAR_IS);
# else
	return zend_get_zval_ptr(node_type, node, zdata, &should_free, BP_VAR_IS);
# endif
#endif
}

zval *xdebug_get_zval(zend_execute_data *zdata, int node_type, const znode_op *node, int *is_var)
{
	return xdebug_get_zval_with_opline(zdata, zdata->opline, node_type, node, is_var);
}


/*****************************************************************************
** PHP Variable related utility functions
*/

/*****************************************************************************
** Data returning functions
*/
#define XF_ST_ROOT                 0
#define XF_ST_ARRAY_INDEX_NUM      1
#define XF_ST_ARRAY_INDEX_ASSOC    2
#define XF_ST_OBJ_PROPERTY         3
#define XF_ST_STATIC_ROOT          4
#define XF_ST_STATIC_PROPERTY      5
#define XF_ST_ESCAPED_OBJ_PROPERTY 6

inline static HashTable *fetch_ht_from_zval(zval *z)
{
	switch (Z_TYPE_P(z)) {
		case IS_ARRAY:
			return Z_ARRVAL_P(z);
			break;
		case IS_OBJECT:
			return Z_OBJPROP_P(z);
			break;
	}
	return NULL;
}

inline static char *fetch_classname_from_zval(zval *z, int *length, zend_class_entry **ce)
{
	zend_string *class_name;

	if (Z_TYPE_P(z) == IS_INDIRECT) {
		z = z->value.zv;
	}
	if (Z_TYPE_P(z) == IS_REFERENCE) {
		z = &z->value.ref->val;
	}

	if (Z_TYPE_P(z) != IS_OBJECT) {
		return NULL;
	}

	class_name = Z_OBJ_HANDLER_P(z, get_class_name)(Z_OBJ_P(z));

	*ce = Z_OBJCE_P(z);
	*length = class_name->len;

	return estrdup(class_name->val);
}

static char* prepare_search_key(char *name, unsigned int *name_length, const char *prefix, int prefix_length)
{
	char *element;
	int   extra_length = 0;

	if (prefix_length) {
		if (prefix[0] == '*') {
			extra_length = 3;
		} else {
			extra_length = 2 + prefix_length;
		}
	}

	element = malloc(*name_length + 1 + extra_length);
	memset(element, 0, *name_length + 1 + extra_length);
	if (extra_length) {
		memcpy(element + 1, prefix, extra_length - 2);
	}
	memcpy(element + extra_length, name, *name_length);
	*name_length += extra_length;

	return element;
}

#if PHP_VERSION_ID >= 70400
static zval *get_arrayobject_storage(zval *parent, HashTable **properties)
{
	*properties = zend_get_properties_for(parent, ZEND_PROP_PURPOSE_DEBUG);
	return zend_hash_str_find(*properties, "\0ArrayObject\0storage", sizeof("*ArrayObject*storage") - 1);
}

static zval *get_splobjectstorage_storage(zval *parent, HashTable **properties)
{
	*properties = zend_get_properties_for(parent, ZEND_PROP_PURPOSE_DEBUG);
	return zend_hash_str_find(*properties, "\0SplObjectStorage\0storage", sizeof("*SplObjectStorage*storage") - 1);
}

static zval *get_arrayiterator_storage(zval *parent, HashTable **properties)
{
	*properties = zend_get_properties_for(parent, ZEND_PROP_PURPOSE_DEBUG);
	return zend_hash_str_find(*properties, "\0ArrayIterator\0storage", sizeof("*ArrayIterator*storage") - 1);
}
#else
static zval *get_arrayobject_storage(zval *parent, HashTable **properties, int *is_temp)
{
	*properties = Z_OBJDEBUG_P(parent, *is_temp);
	return zend_hash_str_find(*properties, "\0ArrayObject\0storage", sizeof("*ArrayObject*storage") - 1);
}

static zval *get_splobjectstorage_storage(zval *parent, HashTable **properties, int *is_temp)
{
	*properties = Z_OBJDEBUG_P(parent, *is_temp);
	return zend_hash_str_find(*properties, "\0SplObjectStorage\0storage", sizeof("*SplObjectStorage*storage") - 1);
}

static zval *get_arrayiterator_storage(zval *parent, HashTable **properties, int *is_temp)
{
	*properties = Z_OBJDEBUG_P(parent, *is_temp);
	return zend_hash_str_find(*properties, "\0ArrayIterator\0storage", sizeof("*ArrayIterator*storage") - 1);
}
#endif

#if PHP_VERSION_ID < 70400
void xdebug_var_maybe_destroy_ht(HashTable *ht, int is_temp)
{
	if (ht && is_temp) {
		zend_hash_destroy(ht);
		FREE_HASHTABLE(ht);
	}
}
#endif

static void fetch_zval_from_symbol_table(
		zval *value_in, char *name, unsigned int name_length,
		int type, char* ccn, int ccnl, zend_class_entry *cce)
{
	HashTable *ht = NULL;
	char  *element = NULL;
	unsigned int element_length = name_length;
	zend_property_info *zpp;
#if PHP_VERSION_ID < 70400
	int is_temp = 0;
#endif
	int free_duplicated_name = 0;
	HashTable *myht = NULL;
	zval *orig_value_in = value_in;
	zval tmp_retval;

	ZVAL_UNDEF(&tmp_retval);

	if (Z_TYPE_P(value_in) == IS_INDIRECT) {
		value_in = Z_INDIRECT_P(value_in);
	}
	ZVAL_DEREF(value_in);

	ht = fetch_ht_from_zval(value_in);

	switch (type) {
		case XF_ST_STATIC_ROOT:
		case XF_ST_STATIC_PROPERTY:
			/* First we try a public,private,protected property */
#if PHP_VERSION_ID >= 70400
			if (cce && (cce->type == ZEND_INTERNAL_CLASS || (cce->ce_flags & ZEND_ACC_IMMUTABLE))) {
				zend_class_init_statics(cce);
			}
#endif
			element = prepare_search_key(name, &element_length, "", 0);
			if (cce && ((zpp = zend_hash_str_find_ptr(&cce->properties_info, element, element_length)) != NULL) && CE_STATIC_MEMBERS(cce)) {
				ZVAL_COPY(&tmp_retval, &CE_STATIC_MEMBERS(cce)[zpp->offset]);
				goto cleanup;
			}
			element_length = name_length;

			/* Then we try to see whether the first char is * and use the part between * and * as class name for the private property */
			if (name[0] == '*') {
				char *secondStar;

				secondStar = strstr(name + 1, "*");
				if (secondStar) {
					free(element);
					element_length = name_length - (secondStar + 1 - name);
					element = prepare_search_key(secondStar + 1, &element_length, "", 0);
					if (cce && ((zpp = zend_hash_str_find_ptr(&cce->properties_info, element, element_length)) != NULL)) {
						ZVAL_COPY(&tmp_retval, &CE_STATIC_MEMBERS(cce)[zpp->offset]);
						goto cleanup;
					}
				}
			}

			break;

		case XF_ST_ROOT:
			/* Check for compiled vars */
			element = prepare_search_key(name, &element_length, "", 0);
			if (xdebug_lib_has_active_data() && xdebug_lib_has_active_function()) {
				int i = 0;
				zend_ulong hash_value = zend_inline_hash_func(element, element_length);
				zend_op_array *opa = xdebug_lib_get_active_func_oparray();
				zval **CV;

				while (i < opa->last_var) {
					if (ZSTR_H(opa->vars[i]) == hash_value &&
						ZSTR_LEN(opa->vars[i]) == element_length &&
						strncmp(STR_NAME_VAL(opa->vars[i]), element, element_length) == 0)
					{
						zval *CV_z = ZEND_CALL_VAR_NUM(xdebug_lib_get_active_data(), i);
						CV = &CV_z;
						if (CV) {
							ZVAL_COPY(&tmp_retval, *CV);
							goto cleanup;
						}
					}
					i++;
				}
			}
			free(element);
			ht = xdebug_lib_get_active_symbol_table();

			XDEBUG_BREAK_INTENTIONALLY_MISSING

		case XF_ST_ARRAY_INDEX_ASSOC:
			element = prepare_search_key(name, &name_length, "", 0);
			xdebug_stripcslashes(element, (int *) &name_length);

			/* Handle "this" in a different way */
			if (type == XF_ST_ROOT && strcmp("this", element) == 0) {
				if (xdebug_lib_has_active_object()) {
					ZVAL_COPY(&tmp_retval, xdebug_lib_get_active_object());
				} else {
					ZVAL_NULL(&tmp_retval);
				}
				goto cleanup;
			}

			if (ht) {
				zval *tmp = zend_hash_str_find(ht, element, name_length);
				if (tmp != NULL) {
					ZVAL_COPY(&tmp_retval, tmp);
					goto cleanup;
				}
			}
			break;

		case XF_ST_ARRAY_INDEX_NUM:
			element = prepare_search_key(name, &name_length, "", 0);
			if (ht) {
				zval *tmp = zend_hash_index_find(ht, strtoull(element, NULL, 10));
				if (tmp != NULL) {
					ZVAL_COPY(&tmp_retval, tmp);
					goto cleanup;
				}
			}
			break;

		case XF_ST_ESCAPED_OBJ_PROPERTY:
			name = xdstrndup(name, name_length);
			free_duplicated_name = 1;
			xdebug_stripcslashes(name, (int *) &name_length);

			XDEBUG_BREAK_INTENTIONALLY_MISSING

		case XF_ST_OBJ_PROPERTY:
			/* Let's see if there is a debug handler */
			if (value_in && Z_TYPE_P(value_in) == IS_OBJECT) {
#if PHP_VERSION_ID >= 70400
				myht = xdebug_objdebug_pp(&value_in);
#else
				myht = xdebug_objdebug_pp(&value_in, &is_temp);
#endif
				if (myht) {
					zval *tmp = zend_symtable_str_find(myht, name, name_length);
					if (tmp != NULL) {
						ZVAL_COPY(&tmp_retval, tmp);
#if PHP_VERSION_ID >= 70400
						zend_release_properties(myht);
#else
						xdebug_var_maybe_destroy_ht(myht, is_temp);
#endif
						goto cleanup;
					}
#if PHP_VERSION_ID >= 70400
					zend_release_properties(myht);
#else
					xdebug_var_maybe_destroy_ht(myht, is_temp);
#endif
				}
			}
			/* First we try an object handler */
			if (cce) {
				zval *tmp_val;
#if PHP_VERSION_ID >= 80000
				tmp_val = zend_read_property(cce, Z_OBJ_P(value_in), name, name_length, 1, &tmp_retval);
#else
				tmp_val = zend_read_property(cce, value_in, name, name_length, 1, &tmp_retval);
#endif
				if (tmp_val != &tmp_retval && tmp_val != &EG(uninitialized_zval)) {
					ZVAL_COPY(&tmp_retval, tmp_val);
					goto cleanup;
				}

				if (EG(exception)) {
					zend_clear_exception();
				}
			}

			/* Then we try a public property */
			element = prepare_search_key(name, &element_length, "", 0);
			if (ht) {
				zval *tmp = zend_symtable_str_find(ht, element, element_length);
				if (tmp != NULL) {
					ZVAL_COPY(&tmp_retval, tmp);
					goto cleanup;
				}
			}
			element_length = name_length;

			/* Then we try it again as protected property */
			free(element);
			element = prepare_search_key(name, &element_length, "*", 1);
			if (ht) {
				zval *tmp = zend_hash_str_find(ht, element, element_length);
				if (tmp != NULL) {
					ZVAL_COPY(&tmp_retval, tmp);
					goto cleanup;
				}
			}
			element_length = name_length;

			/* Then we try it again as private property */
			free(element);
			element = prepare_search_key(name, &element_length, ccn, ccnl);
			if (ht) {
				zval *tmp = zend_hash_str_find(ht, element, element_length);
				if (tmp != NULL) {
					ZVAL_COPY(&tmp_retval, tmp);
					goto cleanup;
				}
			}
			element_length = name_length;

			/* All right, time for a mega hack. It's SplObjectStorage access time! */
			if (strncmp(ccn, "SplObjectStorage", ccnl) == 0 && strncmp(name, "storage", name_length) == 0) {
#if PHP_VERSION_ID >= 70400
				zval *tmp = get_splobjectstorage_storage(value_in, &myht);
#else
				zval *tmp = get_splobjectstorage_storage(value_in, &myht, &is_temp);
#endif
				element = NULL;
				if (tmp != NULL) {
					ZVAL_COPY(&tmp_retval, tmp);
#if PHP_VERSION_ID >= 70400
					zend_release_properties(myht);
#else
					xdebug_var_maybe_destroy_ht(myht, is_temp);
#endif
					goto cleanup;
				}
#if PHP_VERSION_ID >= 70400
				zend_release_properties(myht);
#else
				xdebug_var_maybe_destroy_ht(myht, is_temp);
#endif
			}

			/* Then we try to see whether the first char is * and use the part between * and * as class name for the private property */
			if (name[0] == '*') {
				char *secondStar;

				secondStar = strstr(name + 1, "*");
				if (secondStar) {
					free(element);
					element_length = name_length - (secondStar + 1 - name);

					/* All right, time for a mega hack. It's ArrayObject access time! */
					if (strncmp(name + 1, "ArrayObject", secondStar - name - 1) == 0 && strncmp(secondStar + 1, "storage", element_length) == 0) {
#if PHP_VERSION_ID >= 70400
						zval *tmp = get_arrayobject_storage(value_in, &myht);
#else
						zval *tmp = get_arrayobject_storage(value_in, &myht, &is_temp);
#endif
						element = NULL;
						if (tmp != NULL) {
							ZVAL_COPY(&tmp_retval, tmp);
#if PHP_VERSION_ID >= 70400
							zend_release_properties(myht);
#else
							xdebug_var_maybe_destroy_ht(myht, is_temp);
#endif
							goto cleanup;
						}
#if PHP_VERSION_ID >= 70400
						zend_release_properties(myht);
#else
						xdebug_var_maybe_destroy_ht(myht, is_temp);
#endif
					}
					/* All right, time for a mega hack. It's ArrayIterator access time! */
					if (strncmp(name + 1, "ArrayIterator", secondStar - name - 1) == 0 && strncmp(secondStar + 1, "storage", element_length) == 0) {
#if PHP_VERSION_ID >= 70400
						zval *tmp = get_arrayiterator_storage(value_in, &myht);
#else
						zval *tmp = get_arrayiterator_storage(value_in, &myht, &is_temp);
#endif
						element = NULL;
						if (tmp != NULL) {
							ZVAL_COPY(&tmp_retval, tmp);
#if PHP_VERSION_ID >= 70400
							zend_release_properties(myht);
#else
							xdebug_var_maybe_destroy_ht(myht, is_temp);
#endif
							goto cleanup;
						}
#if PHP_VERSION_ID >= 70400
						zend_release_properties(myht);
#else
						xdebug_var_maybe_destroy_ht(myht, is_temp);
#endif

					}

					/* The normal one */
					element = prepare_search_key(secondStar + 1, &element_length, name + 1, secondStar - name - 1);
					if (ht) {
						zval *tmp = zend_hash_str_find(ht, element, element_length);
						if (tmp != NULL) {
							ZVAL_COPY(&tmp_retval, tmp);
							goto cleanup;
						}
					}
				}
			}

			break;
	}

cleanup:
	if (element) {
		free(element);
	}
	if (free_duplicated_name && name) {
		xdfree(name);
	}

	zval_ptr_dtor_nogc(orig_value_in);
	ZVAL_COPY_VALUE(orig_value_in, &tmp_retval);
}

inline static int is_objectish(zval *value)
{
	switch (Z_TYPE_P(value)) {
		case IS_OBJECT:
			return 1;

		case IS_INDIRECT:
			if (Z_TYPE_P(Z_INDIRECT_P(value)) == IS_OBJECT) {
				return 1;
			}
			break;

		case IS_REFERENCE:
			if (Z_TYPE_P(Z_REFVAL_P(value)) == IS_OBJECT) {
				return 1;
			}
			break;
	}

	return 0;
}

void xdebug_get_php_symbol(zval *retval, xdebug_str* name)
{
	int        found = -1;
	int        state = 0;
	char      *ptr = name->d;
	size_t     ctr = 0;
	char      *keyword = NULL, *keyword_end = NULL;
	int        type = XF_ST_ROOT;
	char      *current_classname = NULL;
	zend_class_entry *current_ce = NULL;
	int        cc_length = 0;
	char       quotechar = 0;

	ZVAL_UNDEF(retval);

	do {
		if (ctr == name->l) {
			found = 0;
		} else {
			switch (state) {
				case 0:
					if (ptr[ctr] == '$') {
						keyword = &ptr[ctr] + 1;
						break;
					}
					if (ptr[ctr] == ':') { /* special tricks */
						keyword = &ptr[ctr];
						state = 7;
						break;
					}
					keyword = &ptr[ctr];
					state = 1;

					XDEBUG_BREAK_INTENTIONALLY_MISSING

				case 1:
					if (ptr[ctr] == '[') {
						keyword_end = &ptr[ctr];
						if (keyword) {
							fetch_zval_from_symbol_table(retval, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce);
							if (current_classname) {
								efree(current_classname);
							}
							current_classname = NULL;
							cc_length = 0;
							current_ce = NULL;
							keyword = NULL;
						}
						state = 3;
					} else if (ptr[ctr] == '-') {
						keyword_end = &ptr[ctr];
						if (keyword) {
							fetch_zval_from_symbol_table(retval, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce);
							if (current_classname) {
								efree(current_classname);
							}
							current_classname = NULL;
							cc_length = 0;
							current_ce = NULL;
							if (is_objectish(retval)) {
								current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce);
							}
							keyword = NULL;
						}
						state = 2;
						type = XF_ST_OBJ_PROPERTY;
					} else if (ptr[ctr] == ':') {
						keyword_end = &ptr[ctr];
						if (keyword) {
							fetch_zval_from_symbol_table(retval, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce);
							if (current_classname) {
								efree(current_classname);
							}
							current_classname = NULL;
							cc_length = 0;
							if (is_objectish(retval)) {
								current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce);
							}
							keyword = NULL;
						}
						state = 8;
						type = XF_ST_STATIC_PROPERTY;
					}
					break;
				case 2:
					if (ptr[ctr] != '>') {
						if (ptr[ctr] == '{') {
							state = 11;
						} else {
							keyword = &ptr[ctr];
							state = 1;
						}
					}
					break;
				case 8:
					if (ptr[ctr] != ':') {
						keyword = &ptr[ctr];
						state = 1;
					}
					break;
				case 3: /* Parsing in [...] */
					/* Associative arrays */
					if (ptr[ctr] == '\'' || ptr[ctr] == '"') {
						state = 4;
						keyword = &ptr[ctr] + 1;
						quotechar = ptr[ctr];
						type = XF_ST_ARRAY_INDEX_ASSOC;
					}
					/* Numerical index */
					if (ptr[ctr] >= '0' && ptr[ctr] <= '9') {
						cc_length = 0;
						state = 6;
						keyword = &ptr[ctr];
						type = XF_ST_ARRAY_INDEX_NUM;
					}
					/* Numerical index starting with a - */
					if (ptr[ctr] == '-') {
						state = 9;
						keyword = &ptr[ctr];
					}
					break;
				case 9:
					/* Numerical index starting with a - */
					if (ptr[ctr] >= '0' && ptr[ctr] <= '9') {
						state = 6;
						type = XF_ST_ARRAY_INDEX_NUM;
					}
					break;
				case 4:
					if (ptr[ctr] == '\\') {
						state = 10; /* Escaped character */
					} else if (ptr[ctr] == quotechar) {
						quotechar = 0;
						state = 5;
						keyword_end = &ptr[ctr];
						fetch_zval_from_symbol_table(retval, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce);
						if (current_classname) {
							efree(current_classname);
						}
						current_classname = NULL;
						cc_length = 0;
						if (is_objectish(retval)) {
							current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce);
						}
						keyword = NULL;
					}
					break;
				case 10: /* Escaped character */
					state = 4;
					break;
				case 5:
					if (ptr[ctr] == ']') {
						state = 1;
					}
					break;
				case 6:
					if (ptr[ctr] == ']') {
						state = 1;
						keyword_end = &ptr[ctr];
						fetch_zval_from_symbol_table(retval, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce);
						if (current_classname) {
							efree(current_classname);
						}
						current_classname = NULL;
						cc_length = 0;
						if (is_objectish(retval)) {
							current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce);
						}
						keyword = NULL;
					}
					break;
				case 7: /* special cases, started with a ":" */
					if (ptr[ctr] == ':') {
						function_stack_entry *active_fse = xdebug_lib_get_active_stack_entry();
						state = 1;
						keyword_end = &ptr[ctr];

						if (strncmp(keyword, "::", 2) == 0 && active_fse->function.class_name) { /* static class properties */
							zend_class_entry *ce = zend_fetch_class(active_fse->function.class_name, ZEND_FETCH_CLASS_SELF);

							current_classname = estrdup(STR_NAME_VAL(ce->name));
							cc_length = strlen(STR_NAME_VAL(ce->name));
							current_ce = ce;
							keyword = &ptr[ctr] + 1;

							type = XF_ST_STATIC_ROOT;
						} else {
							keyword = NULL;
						}
					}
					break;

				case 11:
					if (ptr[ctr] == '\'' || ptr[ctr] == '"') {
						state = 12;
						keyword = &ptr[ctr] + 1;
						quotechar = ptr[ctr];
						type = XF_ST_ESCAPED_OBJ_PROPERTY;
					}
					break;

				case 12: /* Inside {" */
					if (ptr[ctr] == '\\') {
						state = 13; /* Escaped character */
					} else if (ptr[ctr] == quotechar) {
						quotechar = 0;
						state = 14;
						keyword_end = &ptr[ctr];
						fetch_zval_from_symbol_table(retval, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce);
						if (current_classname) {
							efree(current_classname);
						}
						current_classname = NULL;
						cc_length = 0;
						if (is_objectish(retval)) {
							current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce);
						}
						keyword = NULL;
					}
					break;
				case 13: /* Escaped character */
					state = 12;
					break;
				case 14:
					if (ptr[ctr] == '}') {
						state = 1;
					}
					break;
			}
			ctr++;
		}
	} while (found < 0);
	if (keyword != NULL) {
		fetch_zval_from_symbol_table(retval, keyword, &ptr[ctr] - keyword, type, current_classname, cc_length, current_ce);
	}
	if (current_classname) {
		efree(current_classname);
	}
}


#if PHP_VERSION_ID >= 70400
xdebug_str* xdebug_get_property_type(zval* object, zval *val)
{
	xdebug_str         *type_str = NULL;
	zend_property_info *info;

	if (Z_TYPE_P(val) != IS_INDIRECT) {
		return NULL;
	}
	val = Z_INDIRECT_P(val);

	info = zend_get_typed_property_info_for_slot(Z_OBJ_P(object), val);

	if (info) {
#if PHP_VERSION_ID >= 80000
		zend_string *type_info = zend_type_to_string(info->type);
		type_str = xdebug_str_create(ZSTR_VAL(type_info), ZSTR_LEN(type_info));
		zend_string_release(type_info);
#else
		type_str = xdebug_str_new();

		if (ZEND_TYPE_ALLOW_NULL(info->type)) {
			xdebug_str_addc(type_str, '?');
		}
		if (ZEND_TYPE_IS_CLASS(info->type)) {
			xdebug_str_add(
				type_str,
				ZSTR_VAL(
					ZEND_TYPE_IS_CE(info->type) ? ZEND_TYPE_CE(info->type)->name : ZEND_TYPE_NAME(info->type)
				),
				0
			);
		} else {
			xdebug_str_add(type_str, zend_get_type_by_const(ZEND_TYPE_CODE(info->type)), 0);
		}
#endif
	}

	return type_str;
}
#endif

xdebug_str* xdebug_get_property_info(char *mangled_property, int mangled_len, const char **modifier, char **class_name)
{
	const char *cls_name, *tmp_prop_name;
	size_t      tmp_prop_name_len;
	xdebug_str *property_name;

	zend_string *i_mangled = zend_string_init(mangled_property, mangled_len - 1, 0);
	zend_unmangle_property_name_ex(i_mangled, &cls_name, &tmp_prop_name, &tmp_prop_name_len);
	property_name = xdebug_str_create((char*) tmp_prop_name, tmp_prop_name_len);
	*class_name = cls_name ? xdstrdup(cls_name) : NULL;
	zend_string_release(i_mangled);

	if (*class_name) {
		if (*class_name[0] == '*') {
			*modifier = "protected";
		} else {
			*modifier = "private";
		}
	} else {
		*modifier = "public";
	}

	return property_name;
}

#define XDEBUG_MAX_INT 2147483647

xdebug_var_export_options* xdebug_var_export_options_from_ini(void)
{
	xdebug_var_export_options *options;
	options = xdmalloc(sizeof(xdebug_var_export_options));

	options->max_children = XINI_LIB(display_max_children);
	options->max_data = XINI_LIB(display_max_data);
	options->max_depth = XINI_LIB(display_max_depth);
	options->show_hidden = 0;
	options->show_location = 1;
	options->extended_properties = 0;
	options->encode_as_extended_property = 0;

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

xdebug_var_export_options xdebug_var_nolimit_options = { XDEBUG_MAX_INT, XDEBUG_MAX_INT, 1023, 1, 0, 0, 0, NULL, 0 };

xdebug_var_export_options* xdebug_var_get_nolimit_options(void)
{
	return &xdebug_var_nolimit_options;
}

/*****************************************************************************
** Normal variable printing routines
*/

#define XDEBUG_VAR_ATTR_TEXT 0
#define XDEBUG_VAR_ATTR_FANCY 1

void xdebug_add_variable_attributes(xdebug_str *str, zval *struc, zend_bool html)
{
	if (html) {
		xdebug_str_add_literal(str, "<i>(");
	} else {
		xdebug_str_add_literal(str, "(");
	}

	if (Z_TYPE_P(struc) >= IS_STRING && Z_TYPE_P(struc) != IS_INDIRECT) {
		if (Z_TYPE_P(struc) == IS_STRING && ZSTR_IS_INTERNED(Z_STR_P(struc))) {
			xdebug_str_add_literal(str, "interned");
		} else if (Z_TYPE_P(struc) == IS_ARRAY && (GC_FLAGS(Z_ARRVAL_P(struc)) & IS_ARRAY_IMMUTABLE)) {
			xdebug_str_add_literal(str, "immutable");
		} else {
			xdebug_str_add_fmt(str, "refcount=%d", Z_REFCOUNT_P(struc));
		}
		xdebug_str_add_fmt(str, ", is_ref=%d", Z_TYPE_P(struc) == IS_REFERENCE);
	} else {
		xdebug_str_add_literal(str, "refcount=0, is_ref=0");
	}

	if (html) {
		xdebug_str_add_literal(str, ")</i>");
	} else {
		xdebug_str_add_literal(str, ")=");
	}
}


/*****************************************************************************
** XML encoding function
*/
static const char xml_encode_count[256] = {
	4, 1, 1, 1,  1, 1, 1, 1,  1, 1, 5, 1,  1, 5, 1, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
	1, 1, 6, 1,  1, 1, 5, 5,  1, 1, 1, 1,  1, 1, 1, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  4, 1, 4, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
	1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
};

static const char* xml_encode_map[64] = {
	"&#0;", NULL, NULL,     NULL,  NULL, NULL, NULL,    NULL,     NULL, NULL, "&#10;", NULL,  NULL,   "&#13;", NULL,   NULL,
	NULL,   NULL, NULL,     NULL,  NULL, NULL, NULL,    NULL,     NULL, NULL, NULL,    NULL,  NULL,   NULL,    NULL,   NULL,
	NULL,   NULL, "&quot;", NULL,  NULL, NULL, "&amp;", "&#39;",  NULL, NULL, NULL,    NULL,  NULL,   NULL,    NULL,   NULL,
	NULL,   NULL, NULL,     NULL,  NULL, NULL, NULL,    NULL,     NULL, NULL, NULL,    NULL,  "&lt;", NULL,    "&gt;", NULL,
};

char* xdebug_xmlize(char *s_string, size_t len, size_t *newlen)
{
	int i, w_pos;
	int encoded_string_length = 0;
	char *new_string;
	const unsigned char *string = (unsigned char*) s_string;

	/* Quick bailout for empty strings */
	if (!len) {
		*newlen = 0;
		return estrdup("");
	}

	/* Calculate new memory requirement */
	for (i = 0; i < len; i++) {
		encoded_string_length += xml_encode_count[string[i]];
	}

	/* No characters need to be encoded, so just duplicate and return */
	if (encoded_string_length == len) {
		*newlen = len;
		return estrdup(s_string);
	}

	new_string = emalloc(encoded_string_length + 1);
	w_pos = 0;
	for (i = 0; i < len; i++) {
		int replacement_length = xml_encode_count[string[i]];

		if (replacement_length != 1) {
			int j;

			for (j = 0; j < replacement_length; j++) {
				new_string[w_pos] = xml_encode_map[string[i]][j];
				w_pos++;
			}
			continue;
		}

		new_string[w_pos] = string[i];
		w_pos++;
	}

	new_string[w_pos] = '\0';
	*newlen = encoded_string_length; /* remove one for null byte */

	return new_string;
}

/*****************************************************************************
** Function name printing function
*/
static char* xdebug_create_doc_link(xdebug_func f)
{
	char *tmp_target = NULL, *p, *retval;

	switch (f.type) {
		case XFUNC_NORMAL: {
			tmp_target = xdebug_sprintf("function.%s", f.function);
			break;
		}

		case XFUNC_STATIC_MEMBER:
		case XFUNC_MEMBER: {
			if (strcmp(f.function, "__construct") == 0) {
				tmp_target = xdebug_sprintf("%s.construct", ZSTR_VAL(f.class_name));
			} else {
				tmp_target = xdebug_sprintf("%s.%s", ZSTR_VAL(f.class_name), f.function);
			}
			break;
		}
	}

	while ((p = strchr(tmp_target, '_')) != NULL) {
		*p = '-';
	}

	retval = xdebug_sprintf("<a href='%s%s%s' target='_new'>%s</a>",
		(PG(docref_root) && PG(docref_root)[0]) ? PG(docref_root) : "http://www.php.net/",
		tmp_target, PG(docref_ext), f.function);

	xdfree(tmp_target);

	return retval;
}

char* xdebug_show_fname(xdebug_func f, int html, int flags)
{
	switch (f.type) {
		case XFUNC_NORMAL: {
			if (PG(html_errors) && html && f.internal) {
				return xdebug_create_doc_link(f);
			} else {
				return xdstrdup(f.function);
			}
			break;
		}

		case XFUNC_STATIC_MEMBER:
		case XFUNC_MEMBER: {
			if (PG(html_errors) && html && f.internal) {
				return xdebug_create_doc_link(f);
			} else {
				return xdebug_sprintf("%s%s%s",
					f.class_name ? ZSTR_VAL(f.class_name) : "?",
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

		case XFUNC_MAIN:
			return xdstrdup("{main}");
			break;

		case XFUNC_ZEND_PASS:
			return xdstrdup("{zend_pass}");
			break;

		default:
			return xdstrdup("{unknown}");
	}
}
