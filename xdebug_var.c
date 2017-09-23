/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2017 Derick Rethans                               |
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
#include "ext/standard/php_smart_string.h"
#include "zend_smart_str.h"

#include "php_xdebug.h"
#include "xdebug_compat.h"
#include "xdebug_private.h"
#include "xdebug_mm.h"
#include "xdebug_var.h"
#include "xdebug_xml.h"

/* Set correct int format to use */
#include "Zend/zend_long.h"
#if SIZEOF_ZEND_LONG == 4
# define XDEBUG_INT_FMT "%ld"
#else
# define XDEBUG_INT_FMT "%lld"
#endif

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

HashTable *xdebug_objdebug_pp(zval **zval_pp, int *is_tmp TSRMLS_DC)
{
	zval dzval = **zval_pp;
	HashTable *tmp;

	if (!XG(in_debug_info) && Z_OBJ_HANDLER(dzval, get_debug_info)) {
		zend_bool old_trace = XG(do_trace);
		zend_object *orig_exception;

		XG(do_trace) = 0;
		XG(in_debug_info) = 1;
		orig_exception = EG(exception);
		EG(exception) = NULL;

		tmp = Z_OBJ_HANDLER(dzval, get_debug_info)(&dzval, is_tmp TSRMLS_CC);

		XG(in_debug_info) = 0;
		XG(do_trace) = old_trace;
		EG(exception) = orig_exception;
		return tmp;
	} else {
		*is_tmp = 0;
		if (Z_OBJ_HANDLER(dzval, get_properties)) {
			return Z_OBJPROP(dzval);
		}
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
			return xdstrdup("catchable-fatal-error");
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
			return xdstrdup("Catchable fatal error");
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

zval *xdebug_get_zval(zend_execute_data *zdata, int node_type, const znode_op *node, int *is_var)
{
	zend_free_op should_free;

	return zend_get_zval_ptr(node_type, node, zdata, &should_free, BP_VAR_IS);
}

/*****************************************************************************
** PHP Variable related utility functions
*/

/*****************************************************************************
** Data returning functions
*/
#define XF_ST_ROOT               0
#define XF_ST_ARRAY_INDEX_NUM    1
#define XF_ST_ARRAY_INDEX_ASSOC  2
#define XF_ST_OBJ_PROPERTY       3
#define XF_ST_STATIC_ROOT        4
#define XF_ST_STATIC_PROPERTY    5

inline static HashTable *fetch_ht_from_zval(zval *z TSRMLS_DC)
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

inline static char *fetch_classname_from_zval(zval *z, int *length, zend_class_entry **ce TSRMLS_DC)
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

static char* prepare_search_key(char *name, unsigned int *name_length, char *prefix, int prefix_length)
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

static zval *get_arrayobject_storage(zval *parent TSRMLS_DC)
{
	int is_temp;
	HashTable *properties = Z_OBJDEBUG_P(parent, is_temp);

	zval *tmp = NULL;

	if ((tmp = zend_hash_str_find(properties, "\0ArrayObject\0storage", sizeof("*ArrayObject*storage") - 1)) != NULL) {
		return tmp;
	}

	return NULL;
}

static zval *get_splobjectstorage_storage(zval *parent TSRMLS_DC)
{
	int is_temp;
	HashTable *properties = Z_OBJDEBUG_P(parent, is_temp);

	zval *tmp = NULL;

	if ((tmp = zend_hash_str_find(properties, "\0SplObjectStorage\0storage", sizeof("*SplObjectStorage*storage") - 1)) != NULL) {
		return tmp;
	}

	return NULL;
}

static zval *get_arrayiterator_storage(zval *parent TSRMLS_DC)
{
	int is_temp;
	HashTable *properties = Z_OBJDEBUG_P(parent, is_temp);

	zval *tmp = NULL;

	if ((tmp = zend_hash_str_find(properties, "\0ArrayIterator\0storage", sizeof("*ArrayIterator*storage") - 1)) != NULL) {
		return tmp;
	}

	return NULL;
}

static void fetch_zval_from_symbol_table(zval *value_in, char* orig_name, unsigned int orig_name_length, int type, char* ccn, int ccnl, zend_class_entry *cce, xdebug_ptr_collection *hashes_to_free TSRMLS_DC)
{
	HashTable *ht = NULL;
	char  *element = NULL;
	unsigned int name_length;
	unsigned int element_length;
	zend_property_info *zpp;
	int is_temp = 0;
	int found = 0;
	HashTable *myht = NULL;
	char *name = NULL;

	if (value_in && Z_TYPE_P(value_in) != IS_UNDEF) {
		if (Z_TYPE_P(value_in) == IS_INDIRECT) {
			ZVAL_COPY(value_in, value_in->value.zv);
		}
		if (Z_TYPE_P(value_in) == IS_REFERENCE) {
			ZVAL_COPY(value_in, &value_in->value.ref->val);
		}

		ht = fetch_ht_from_zval(value_in TSRMLS_CC);
	}

	/* We need to strip the slashes for the " and / here */
	name = xdmalloc(orig_name_length + 1);
	memcpy(name, orig_name, orig_name_length);
	name[orig_name_length] = '\0';
	name_length = orig_name_length;
	element_length = name_length;

	xdebug_stripcslashes(name, (int *) &name_length);

	switch (type) {
		case XF_ST_STATIC_ROOT:
		case XF_ST_STATIC_PROPERTY:
			/* First we try a public,private,protected property */
			element = prepare_search_key(name, &element_length, "", 0);
			if (cce && &cce->properties_info && ((zpp = zend_hash_str_find_ptr(&cce->properties_info, element, element_length)) != NULL) && cce->static_members_table) {
				ZVAL_COPY_VALUE(value_in, &cce->static_members_table[zpp->offset]);
				found = 1;
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
					if (cce && &cce->properties_info && ((zpp = zend_hash_str_find_ptr(&cce->properties_info, element, element_length)) != NULL)) {
						ZVAL_COPY_VALUE(value_in, &cce->static_members_table[zpp->offset]);
						found = 1;
						goto cleanup;
					}
				}
			}

			break;

		case XF_ST_ROOT:
			/* Check for compiled vars */
			element = prepare_search_key(name, &element_length, "", 0);
			if (XG(active_execute_data) && XG(active_execute_data)->func) {
				int i = 0;
				zend_ulong hash_value = zend_inline_hash_func(element, element_length);
				zend_op_array *opa = &XG(active_execute_data)->func->op_array;
				zval **CV;

				while (i < opa->last_var) {
					if (ZSTR_H(opa->vars[i]) == hash_value &&
						ZSTR_LEN(opa->vars[i]) == element_length &&
						strncmp(STR_NAME_VAL(opa->vars[i]), element, element_length) == 0)
					{
						zval *CV_z = ZEND_CALL_VAR_NUM(XG(active_execute_data), i);
						CV = &CV_z;
						if (CV) {
							ZVAL_COPY_VALUE(value_in, *CV);
							found = 1;
							goto cleanup;
						}
					}
					i++;
				}
			}
			free(element);
			ht = XG(active_symbol_table);
			/* break intentionally missing */

		case XF_ST_ARRAY_INDEX_ASSOC:
			element = prepare_search_key(name, &name_length, "", 0);

			/* Handle "this" in a different way */
			if (type == XF_ST_ROOT && strcmp("this", element) == 0) {
				if (XG(This)) {
					ZVAL_COPY_VALUE(value_in, XG(This));
				} else {
					ZVAL_NULL(value_in);
				}
				found = 1;
				goto cleanup;
			}

			if (ht) {
				zval *tmp = zend_hash_str_find(ht, element, name_length);
				if (tmp != NULL) {
					ZVAL_COPY_VALUE(value_in, tmp);
					found = 1;
					goto cleanup;
				}
			}
			break;

		case XF_ST_ARRAY_INDEX_NUM:
			element = prepare_search_key(name, &name_length, "", 0);
			if (ht) {
				zval *tmp = zend_hash_index_find(ht, strtoull(element, NULL, 10));
				if (tmp != NULL) {
					ZVAL_COPY_VALUE(value_in, tmp);
					found = 1;
					goto cleanup;
				}
			}
			break;

		case XF_ST_OBJ_PROPERTY:
			/* Let's see if there is a debug handler */
			if (value_in && Z_TYPE_P(value_in) == IS_OBJECT) {
				myht = xdebug_objdebug_pp(&value_in, &is_temp TSRMLS_CC);
				if (myht) {
					zval *tmp = zend_hash_str_find(myht, name, name_length);
					if (tmp != NULL) {
						ZVAL_COPY_VALUE(value_in, tmp);
						found = 1;
						goto cleanup;
					}
				}
			}
			/* First we try an object handler */
			if (cce) {
				zval *tmp_val;

				zval dummy;
				tmp_val = zend_read_property(cce, value_in, name, name_length, 0, &dummy);
				if (tmp_val && tmp_val != &EG(uninitialized_zval)) {
					ZVAL_COPY_VALUE(value_in, tmp_val);
					found = 1;
					goto cleanup;
				}

				if (EG(exception)) {
					zend_clear_exception(TSRMLS_C);
				}
			}

			/* Then we try a public property */
			element = prepare_search_key(name, &element_length, "", 0);
			if (ht) {
				zval *tmp = zend_symtable_str_find(ht, element, element_length);
				if (tmp != NULL) {
					ZVAL_COPY_VALUE(value_in, tmp);
					found = 1;
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
					ZVAL_COPY_VALUE(value_in, tmp);
					found = 1;
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
					ZVAL_COPY_VALUE(value_in, tmp);
					found = 1;
					goto cleanup;
				}
			}
			element_length = name_length;

			/* All right, time for a mega hack. It's SplObjectStorage access time! */
			if (strncmp(ccn, "SplObjectStorage", ccnl) == 0 && strncmp(name, "storage", name_length) == 0) {
				zval *tmp = get_splobjectstorage_storage(value_in TSRMLS_CC);
				element = NULL;
				if (tmp != NULL) {
					ZVAL_COPY_VALUE(value_in, tmp);
					found = 1;
					goto cleanup;
				}
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
						zval *tmp = get_arrayobject_storage(value_in TSRMLS_CC);
						element = NULL;
						if (tmp != NULL) {
							ZVAL_COPY_VALUE(value_in, tmp);
							found = 1;
							goto cleanup;
						}
					}
					/* All right, time for a mega hack. It's ArrayIterator access time! */
					if (strncmp(name + 1, "ArrayIterator", secondStar - name - 1) == 0 && strncmp(secondStar + 1, "storage", element_length) == 0) {
						zval *tmp = get_arrayiterator_storage(value_in TSRMLS_CC);
						element = NULL;
						if (tmp != NULL) {
							ZVAL_COPY_VALUE(value_in, tmp);
							found = 1;
							goto cleanup;
						}
					}

					/* The normal one */
					element = prepare_search_key(secondStar + 1, &element_length, name + 1, secondStar - name - 1);
					if (ht) {
						zval *tmp = zend_hash_str_find(ht, element, element_length);
						if (tmp != NULL) {
							ZVAL_COPY_VALUE(value_in, tmp);
							found = 1;
							goto cleanup;
						}
					}
				}
			}

			break;
	}

cleanup:
	if (is_temp) {
		xdebug_ptr_collection_add(hashes_to_free, myht);
	}

	if (element) {
		free(element);
	}
	if (name) {
		xdfree(name);
	}

	if (!found) {
		ZVAL_UNDEF(value_in);
	}
}

inline static int is_objectish(zval value)
{
	switch (Z_TYPE(value)) {
		case IS_OBJECT:
			return 1;

		case IS_INDIRECT:
			if (Z_TYPE_P(value.value.zv) == IS_OBJECT) {
				return 1;
			}

		case IS_REFERENCE:
			if (Z_TYPE(value.value.ref->val) == IS_OBJECT) {
				return 1;
			}
	}

	return 0;
}

zval* xdebug_get_php_symbol(char* name, xdebug_ptr_collection *hashes_to_free TSRMLS_DC)
{
	int        found = -1;
	int        state = 0;
	char     **p = &name;
	char      *keyword = NULL, *keyword_end = NULL;
	int        type = XF_ST_ROOT;
	char      *current_classname = NULL;
	zend_class_entry *current_ce = NULL;
	int        cc_length = 0;
	char       quotechar = 0;
	zval       temp_value_in = {{0}};
	zval      *return_retval;

	do {
		if (*p[0] == '\0') {
			found = 0;
		} else {
			switch (state) {
				case 0:
					if (*p[0] == '$') {
						keyword = *p + 1;
						break;
					}
					if (*p[0] == ':') { /* special tricks */
						keyword = *p;
						state = 7;
						break;
					}
					keyword = *p;
					state = 1;
					/* break intentionally missing */
				case 1:
					if (*p[0] == '[') {
						keyword_end = *p;
						if (keyword) {
							fetch_zval_from_symbol_table(&temp_value_in, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce, hashes_to_free TSRMLS_CC);
							if (current_classname) {
								efree(current_classname);
							}
							current_classname = NULL;
							cc_length = 0;
							current_ce = NULL;
							keyword = NULL;
						}
						state = 3;
					} else if (*p[0] == '-') {
						keyword_end = *p;
						if (keyword) {
							fetch_zval_from_symbol_table(&temp_value_in, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce, hashes_to_free TSRMLS_CC);
							if (current_classname) {
								efree(current_classname);
							}
							current_classname = NULL;
							cc_length = 0;
							current_ce = NULL;
							if (is_objectish(temp_value_in)) {
								current_classname = fetch_classname_from_zval(&temp_value_in, &cc_length, &current_ce TSRMLS_CC);
							}
							keyword = NULL;
						}
						state = 2;
						type = XF_ST_OBJ_PROPERTY;
					} else if (*p[0] == ':') {
						keyword_end = *p;
						if (keyword) {
							fetch_zval_from_symbol_table(&temp_value_in, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce, hashes_to_free TSRMLS_CC);
							if (current_classname) {
								efree(current_classname);
							}
							current_classname = NULL;
							cc_length = 0;
							if (is_objectish(temp_value_in)) {
								current_classname = fetch_classname_from_zval(&temp_value_in, &cc_length, &current_ce TSRMLS_CC);
							}
							keyword = NULL;
						}
						state = 8;
						type = XF_ST_STATIC_PROPERTY;
					}
					break;
				case 2:
					if (*p[0] != '>') {
						keyword = *p;
						state = 1;
					}
					break;
				case 8:
					if (*p[0] != ':') {
						keyword = *p;
						state = 1;
					}
					break;
				case 3: /* Parsing in [...] */
					/* Associative arrays */
					if (*p[0] == '\'' || *p[0] == '"') {
						state = 4;
						keyword = *p + 1;
						quotechar = *p[0];
						type = XF_ST_ARRAY_INDEX_ASSOC;
					}
					/* Numerical index */
					if (*p[0] >= '0' && *p[0] <= '9') {
						cc_length = 0;
						state = 6;
						keyword = *p;
						type = XF_ST_ARRAY_INDEX_NUM;
					}
					/* Numerical index starting with a - */
					if (*p[0] == '-') {
						state = 9;
						keyword = *p;
					}
					break;
				case 9:
					/* Numerical index starting with a - */
					if (*p[0] >= '0' && *p[0] <= '9') {
						state = 6;
						type = XF_ST_ARRAY_INDEX_NUM;
					}
					break;
				case 4:
					if (*p[0] == '\\') {
						state = 10; /* Escaped character */
					} else if (*p[0] == quotechar) {
						quotechar = 0;
						state = 5;
						keyword_end = *p;
						fetch_zval_from_symbol_table(&temp_value_in, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce, hashes_to_free TSRMLS_CC);
						if (current_classname) {
							efree(current_classname);
						}
						current_classname = NULL;
						cc_length = 0;
						if (is_objectish(temp_value_in)) {
							current_classname = fetch_classname_from_zval(&temp_value_in, &cc_length, &current_ce TSRMLS_CC);
						}
						keyword = NULL;
					}
					break;
				case 10: /* Escaped character */
					state = 4;
					break;
				case 5:
					if (*p[0] == ']') {
						state = 1;
					}
					break;
				case 6:
					if (*p[0] == ']') {
						state = 1;
						keyword_end = *p;
						fetch_zval_from_symbol_table(&temp_value_in, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce, hashes_to_free TSRMLS_CC);
						if (current_classname) {
							efree(current_classname);
						}
						current_classname = NULL;
						cc_length = 0;
						if (is_objectish(temp_value_in)) {
							current_classname = fetch_classname_from_zval(&temp_value_in, &cc_length, &current_ce TSRMLS_CC);
						}
						keyword = NULL;
					}
					break;
				case 7: /* special cases, started with a ":" */
					if (*p[0] == ':') {
						state = 1;
						keyword_end = *p;

						if (strncmp(keyword, "::", 2) == 0 && XG(active_fse)->function.class) { /* static class properties */
							zend_class_entry *ce = xdebug_fetch_class(XG(active_fse)->function.class, strlen(XG(active_fse)->function.class), ZEND_FETCH_CLASS_SELF TSRMLS_CC);

							current_classname = estrdup(STR_NAME_VAL(ce->name));
							cc_length = strlen(STR_NAME_VAL(ce->name));
							current_ce = ce;
							keyword = *p + 1;

							type = XF_ST_STATIC_ROOT;
						} else {
							keyword = NULL;
						}
					}
					break;
			}
			(*p)++;
		}
	} while (found < 0);
	if (keyword != NULL) {
		fetch_zval_from_symbol_table(&temp_value_in, keyword, *p - keyword, type, current_classname, cc_length, current_ce, hashes_to_free TSRMLS_CC);
	}
	if (current_classname) {
		efree(current_classname);
	}

	/* Allocate and copy the zval value */
	XDEBUG_MAKE_STD_ZVAL(return_retval);
	ZVAL_COPY_VALUE(return_retval, &temp_value_in);

	return return_retval;
}

char* xdebug_get_property_info(char *mangled_property, int mangled_len, char **property_name, char **class_name)
{
	const char *prop_name, *cls_name;

	zend_string *i_mangled = zend_string_init(mangled_property, mangled_len - 1, 0);
	zend_unmangle_property_name(i_mangled, &cls_name, &prop_name);
	*property_name = (char *) xdstrdup(prop_name);
	*class_name = cls_name ? xdstrdup(cls_name) : NULL;
	zend_string_release(i_mangled);

	if (*class_name) {
		if (*class_name[0] == '*') {
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
	options->show_location = XG(overload_var_dump) > 1;
	options->extended_properties = 0;
	options->force_extended = 0;

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

xdebug_var_export_options* xdebug_var_get_nolimit_options(TSRMLS_D)
{
	return &xdebug_var_nolimit_options;
}

/*****************************************************************************
** Normal variable printing routines
*/

static int xdebug_array_element_export(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, xdebug_str *str, int debug_zval, xdebug_var_export_options *options)
{
	zval **zv = &zv_nptr;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		if (HASH_KEY_IS_NUMERIC(hash_key)) { /* numeric key */
			xdebug_str_add(str, xdebug_sprintf(XDEBUG_INT_FMT " => ", index_key), 1);
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

static int xdebug_object_element_export(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, xdebug_str *str, int debug_zval, xdebug_var_export_options *options, char *class_name)
{
	zval **zv = &zv_nptr;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		if (!HASH_KEY_IS_NUMERIC(hash_key)) {
			char *prop_name, *modifier, *prop_class_name;

			modifier = xdebug_get_property_info((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key), &prop_name, &prop_class_name);
			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				xdebug_str_add(str, xdebug_sprintf("%s $%s = ", modifier, prop_name), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("%s ${%s}:%s = ", modifier, prop_class_name, prop_name), 1);
			}

			xdfree(prop_name);
			xdfree(prop_class_name);
		} else {
			xdebug_str_add(str, xdebug_sprintf("public $%d = ", index_key), 1);
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
	int       is_temp;
	zend_ulong num;
	zend_string *key;
	zval *val;
	zval *tmpz;

	if (!struc || !(*struc)) {
		return;
	}

	if (debug_zval) {
		if (Z_TYPE_P(*struc) >= IS_STRING && Z_TYPE_P(*struc) != IS_INDIRECT) {
			xdebug_str_add(str, xdebug_sprintf("(refcount=%d, is_ref=%d)=", (*struc)->value.counted->gc.refcount, Z_TYPE_P(*struc) == IS_REFERENCE), 1);
		} else {
			xdebug_str_add(str, "(refcount=0, is_ref=0)=", 0);
		}
	}
	if (Z_TYPE_P(*struc) == IS_REFERENCE) {
		tmpz = &((*struc)->value.ref->val);
		struc = &tmpz;
	}

	switch (Z_TYPE_P(*struc)) {
		case IS_TRUE:
		case IS_FALSE:
			xdebug_str_add(str, xdebug_sprintf("%s", Z_TYPE_P(*struc) == IS_TRUE ? "TRUE" : "FALSE"), 1);
			break;

		case IS_NULL:
			xdebug_str_addl(str, "NULL", 4, 0);
			break;

		case IS_LONG:
			xdebug_str_add(str, xdebug_sprintf(XDEBUG_INT_FMT, Z_LVAL_P(*struc)), 1);
			break;

		case IS_DOUBLE:
			xdebug_str_add(str, xdebug_sprintf("%.*G", (int) EG(precision), Z_DVAL_P(*struc)), 1);
			break;

		case IS_STRING: {
			zend_string *i_string = zend_string_init(Z_STRVAL_P(*struc), Z_STRLEN_P(*struc), 0);
			zend_string *tmp_zstr;

			tmp_zstr = php_addcslashes(i_string, 0, "'\\\0..\37", 7);

			tmp_str = estrndup(tmp_zstr->val, tmp_zstr->len);
			zend_string_release(tmp_zstr);
			zend_string_release(i_string);

			if (options->no_decoration) {
				xdebug_str_add(str, tmp_str, 0);
			} else if ((size_t) Z_STRLEN_P(*struc) <= (size_t) options->max_data) {
				xdebug_str_add(str, xdebug_sprintf("'%s'", tmp_str), 1);
			} else {
				xdebug_str_addl(str, "'", 1, 0);
				xdebug_str_addl(str, xdebug_sprintf("%s", tmp_str), options->max_data, 1);
				xdebug_str_addl(str, "...'", 4, 0);
			}
			efree(tmp_str);
		} break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);
			if (ZEND_HASH_GET_APPLY_COUNT(myht) < 1) {
				xdebug_str_addl(str, "array (", 7, 0);
				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					ZEND_HASH_INC_APPLY_COUNT(myht);
					ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
						xdebug_array_element_export(val, num, key, level, str, debug_zval, options);
					} ZEND_HASH_FOREACH_END();
					ZEND_HASH_DEC_APPLY_COUNT(myht);

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
			myht = xdebug_objdebug_pp(struc, &is_temp TSRMLS_CC);
			if (ZEND_HASH_GET_APPLY_COUNT(myht) < 1) {
				char *class_name = (char*) STR_NAME_VAL(Z_OBJCE_P(*struc)->name);
				xdebug_str_add(str, xdebug_sprintf("class %s { ", class_name), 1);

				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					ZEND_HASH_INC_APPLY_COUNT(myht);
					ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
						xdebug_object_element_export(val, num, key, level, str, debug_zval, options, class_name);
					} ZEND_HASH_FOREACH_END();
					ZEND_HASH_DEC_APPLY_COUNT(myht);

					/* Remove the ", " at the end of the string */
					if (myht->nNumOfElements > 0) {
						xdebug_str_chop(str, 2);
					}
				} else {
					xdebug_str_addl(str, "...", 3, 0);
				}
				xdebug_str_addl(str, " }", 2, 0);
			} else {
				xdebug_str_addl(str, "...", 3, 0);
			}
			if (is_temp) {
				zend_hash_destroy(myht);
				efree(myht);
			}
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc) TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("resource(%ld) of type (%s)", Z_RES_P(*struc)->handle, type_name ? type_name : "Unknown"), 1);
			break;
		}

		case IS_UNDEF:
			xdebug_str_addl(str, "*uninitialized*", sizeof("*uninitialized*") - 1, 0);
			break;

		default:
			xdebug_str_addl(str, "NFC", 3, 0);
			break;
	}
}

char* xdebug_get_zval_value(zval *val, int debug_zval, xdebug_var_export_options *options)
{
	xdebug_str str = XDEBUG_STR_INITIALIZER;
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
	zval *tmpz;

	if (!struc || !(*struc)) {
		return;
	}

	if (debug_zval) {
		if (Z_TYPE_P(*struc) >= IS_STRING && Z_TYPE_P(*struc) != IS_INDIRECT) {
			xdebug_str_add(str, xdebug_sprintf("(refcount=%d, is_ref=%d)=", (*struc)->value.counted->gc.refcount, Z_TYPE_P(*struc) == IS_REFERENCE), 1);
		} else {
			xdebug_str_add(str, "(refcount=0, is_ref=0)=", 0);
		}
	}
	if (Z_TYPE_P(*struc) == IS_REFERENCE) {
		tmpz = &((*struc)->value.ref->val);
		struc = &tmpz;
	}

	switch (Z_TYPE_P(*struc)) {
		case IS_TRUE:
			xdebug_str_addl(str, "true", 4, 0);
			break;

		case IS_FALSE:
			xdebug_str_addl(str, "false", 5, 0);
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
			xdebug_str_add(str, xdebug_sprintf("string(%d)", Z_STRLEN_P(*struc)), 1);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);
			xdebug_str_add(str, xdebug_sprintf("array(%d)", myht->nNumOfElements), 1);
			break;

		case IS_OBJECT: {
			xdebug_str_add(str, xdebug_sprintf("class %s", STR_NAME_VAL(Z_OBJCE_P(*struc)->name)), 1);
			break;
		}

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc) TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("resource(%ld) of type (%s)", Z_RES_P(*struc)->handle, type_name ? type_name : "Unknown"), 1);
			break;
		}

		case IS_UNDEF:
			xdebug_str_addl(str, "*uninitialized*", sizeof("*uninitialized*") - 1, 0);
			break;

		default:
			xdebug_str_addl(str, "NFC", 3, 0);
			break;

	}
}

char* xdebug_get_zval_synopsis(zval *val, int debug_zval, xdebug_var_export_options *options)
{
	xdebug_str str = XDEBUG_STR_INITIALIZER;
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
		xdebug_var_export_text_ansi(zv, str, mode, level + 1, debug_zval, options TSRMLS_CC);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add(str, xdebug_sprintf("\n%*s(more elements)...\n", (level * 2), ""), 1);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export_text_ansi(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, int mode, xdebug_str *str, int debug_zval, xdebug_var_export_options *options)
{
	zval **zv = &zv_nptr;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 2), ""), 1);

		if (!HASH_KEY_IS_NUMERIC(hash_key)) {
			char *prop_name, *class_name, *modifier;

			modifier = xdebug_get_property_info((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key), &prop_name, &class_name);
			xdebug_str_add(str, xdebug_sprintf("%s%s%s%s%s $%s %s=>%s\n",
			               ANSI_COLOR_MODIFIER, ANSI_COLOR_BOLD, modifier, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_RESET,
			               prop_name, ANSI_COLOR_POINTER, ANSI_COLOR_RESET), 1);

			xdfree(prop_name);
			xdfree(class_name);
		} else {
			xdebug_str_add(str, xdebug_sprintf("%s%spublic%s%s ${%d} %s=>%s\n",
			               ANSI_COLOR_MODIFIER, ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_RESET,
			               index_key, ANSI_COLOR_POINTER, ANSI_COLOR_RESET), 1);
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
	int       is_temp;
	zend_ulong num;
	zend_string *key;
	zval *val;
	zval *tmpz;

	if (!struc || !(*struc)) {
		return;
	}

	xdebug_str_add(str, xdebug_sprintf("%*s", (level * 2) - 2, ""), 1);

	if (debug_zval) {
		if (Z_TYPE_P(*struc) >= IS_STRING && Z_TYPE_P(*struc) != IS_INDIRECT) {
			xdebug_str_add(str, xdebug_sprintf("(refcount=%d, is_ref=%d)=", (*struc)->value.counted->gc.refcount, Z_TYPE_P(*struc) == IS_REFERENCE), 1);
		} else {
			xdebug_str_add(str, "(refcount=0, is_ref=0)=", 0);
		}
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
			char *pattern = (mode == 1) ? "'\\\0..\37" : "\0";
			size_t pattern_len = (mode == 1) ? 7 : 1;

			zend_string *i_string = zend_string_init(Z_STRVAL_P(*struc), Z_STRLEN_P(*struc), 0);
			zend_string *tmp_zstr;

			tmp_zstr = php_addcslashes(i_string, 0, pattern, pattern_len);

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
			if (ZEND_HASH_GET_APPLY_COUNT(myht) < 1) {
				xdebug_str_add(str, xdebug_sprintf("%sarray%s(%s%d%s) {\n", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_LONG, myht->nNumOfElements, ANSI_COLOR_RESET), 1);
				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					ZEND_HASH_INC_APPLY_COUNT(myht);
					ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
						xdebug_array_element_export_text_ansi(val, num, key, level, mode, str, debug_zval, options);
					} ZEND_HASH_FOREACH_END();
					ZEND_HASH_DEC_APPLY_COUNT(myht);
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 2), ""), 1);
				}
				xdebug_str_add(str, xdebug_sprintf("%*s}", (level * 2) - 2 , ""), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("&%sarray%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF), 1);
			}
			break;

		case IS_OBJECT:
			myht = xdebug_objdebug_pp(struc, &is_temp TSRMLS_CC);
			if (myht && ZEND_HASH_GET_APPLY_COUNT(myht) < 1) {
				xdebug_str_add(str, xdebug_sprintf("%sclass%s %s%s%s#%d (%s%d%s) {\n",
					ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF,
					ANSI_COLOR_OBJECT, STR_NAME_VAL(Z_OBJCE_P(*struc)->name), ANSI_COLOR_RESET,
					Z_OBJ_HANDLE_P(*struc),
					ANSI_COLOR_LONG, myht->nNumOfElements, ANSI_COLOR_RESET), 1);

				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					ZEND_HASH_INC_APPLY_COUNT(myht);
					ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
						xdebug_object_element_export_text_ansi(val, num, key, level, mode, str, debug_zval, options);
					} ZEND_HASH_FOREACH_END();
					ZEND_HASH_DEC_APPLY_COUNT(myht);
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 2), ""), 1);
				}
				xdebug_str_add(str, xdebug_sprintf("%*s}", (level * 2) - 2, ""), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 2), ""), 1);
			}
			if (is_temp) {
				zend_hash_destroy(myht);
				efree(myht);
			}
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc) TSRMLS_CC);
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

char* xdebug_get_zval_value_text_ansi(zval *val, int mode, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	xdebug_str str = XDEBUG_STR_INITIALIZER;
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini(TSRMLS_C);
		default_options = 1;
	}

	if (options->show_location && !debug_zval) {
		xdebug_str_add(&str, xdebug_sprintf("%s%s%s:%s%d%s:\n", ANSI_COLOR_BOLD, zend_get_executed_filename(TSRMLS_C), ANSI_COLOR_BOLD_OFF, ANSI_COLOR_BOLD, zend_get_executed_lineno(TSRMLS_C), ANSI_COLOR_BOLD_OFF), 1);
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
	zval *tmpz;

	if (!struc || !(*struc)) {
		return;
	}

	if (debug_zval) {
		if (Z_TYPE_P(*struc) >= IS_STRING && Z_TYPE_P(*struc) != IS_INDIRECT) {
			xdebug_str_add(str, xdebug_sprintf("(refcount=%d, is_ref=%d)=", (*struc)->value.counted->gc.refcount, Z_TYPE_P(*struc) == IS_REFERENCE), 1);
		} else {
			xdebug_str_add(str, "(refcount=0, is_ref=0)=", 0);
		}
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

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc) TSRMLS_CC);
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

char* xdebug_get_zval_synopsis_text_ansi(zval *val, int mode, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	xdebug_str str = XDEBUG_STR_INITIALIZER;
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini(TSRMLS_C);
		default_options = 1;
	}

	if (options->show_location && !debug_zval) {
		xdebug_str_add(&str, xdebug_sprintf("%s%s: %d%s\n", ANSI_COLOR_BOLD, zend_get_executed_filename(TSRMLS_C), zend_get_executed_lineno(TSRMLS_C), ANSI_COLOR_BOLD_OFF), 1);
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
	ulong index_key;
	zval *zv;
} xdebug_object_item;

static int object_item_add_to_merged_hash(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, HashTable *merged, int object_type)
{
	zval **zv = &zv_nptr;
	xdebug_object_item *item;

	item = xdcalloc(1, sizeof(xdebug_object_item));
	item->type = object_type;
	item->zv   = *zv;

	if (hash_key) {
		item->name = (char*) HASH_APPLY_KEY_VAL(hash_key);
		item->name_len = HASH_APPLY_KEY_LEN(hash_key) - 1;
		item->index_key = hash_key->h;
	} else {
		item->name = xdebug_sprintf(XDEBUG_INT_FMT, index_key);
		item->name_len = strlen(item->name);
	}

	zend_hash_next_index_insert_ptr(merged, item);

	return 0;
}

static int object_item_add_zend_prop_to_merged_hash(zend_property_info *zpp, HashTable *merged, int object_type, zend_class_entry *ce)
{
	xdebug_object_item *item;

	if ((zpp->flags & ZEND_ACC_STATIC) == 0) {
		return 0;
	}

	item = xdmalloc(sizeof(xdebug_object_item));
	item->type = object_type;
#if ZTS
	if (ce->type == 1) {
		item->zv   = &CG(static_members_table)[(zend_intptr_t) ce->static_members_table][zpp->offset];
	} else {
		item->zv   = &ce->static_members_table[zpp->offset];
	}
#else
	item->zv   = &ce->static_members_table[zpp->offset];
#endif
	item->name = (char*) STR_NAME_VAL(zpp->name);
	item->name_len = STR_NAME_LEN(zpp->name);

	zend_hash_next_index_insert_ptr(merged, item);

	return 0;
}

static void add_name_attribute_or_element(xdebug_var_export_options *options, xdebug_xml_node *node, char *field, int field_len, char *value, size_t value_len)
{
	size_t value_len_calculated = (value_len == SIZE_MAX) ? strlen(value) : value_len;

	if (options->force_extended || (memchr(value, '\0', value_len_calculated) != NULL && options->extended_properties)) {
		xdebug_xml_node *element;
		char            *tmp_base64;
		int              new_len;

		options->force_extended = 1;

		element = xdebug_xml_node_init(field);
		xdebug_xml_add_attribute(element, "encoding", "base64");

		tmp_base64 = (char*) xdebug_base64_encode((unsigned char*) value, value_len_calculated, &new_len);
		xdebug_xml_add_text_ex(element, strdup(tmp_base64), new_len, 1, 0);
		efree(tmp_base64);

		xdebug_xml_add_child(node, element);

		xdfree(value);
	} else {
		xdebug_xml_add_attribute_exl(node, field, field_len, value, value_len_calculated, 0, 1);
	}
}

static void add_unencoded_text_value_attribute_or_element(xdebug_var_export_options *options, xdebug_xml_node *node, char *value)
{
	if (options->force_extended) {
		xdebug_xml_node *element;
		char            *tmp_base64;
		int              new_len;

		element = xdebug_xml_node_init("value");
		xdebug_xml_add_attribute(element, "encoding", "base64");

		tmp_base64 = (char*) xdebug_base64_encode((unsigned char*) value, strlen(value), &new_len);
		xdebug_xml_add_text_ex(element, strdup(tmp_base64), new_len, 1, 0);
		efree(tmp_base64);

		xdebug_xml_add_child(node, element);
	} else {
		xdebug_xml_add_text(node, value);
	}
}

static void add_encoded_text_value_attribute_or_element(xdebug_var_export_options *options, xdebug_xml_node *node, char *value, size_t value_len)
{
	if (options->force_extended) {
		xdebug_xml_node *element;
		char            *tmp_base64;
		int              new_len;

		element = xdebug_xml_node_init("value");
		xdebug_xml_add_attribute(element, "encoding", "base64");

		tmp_base64 = (char*) xdebug_base64_encode((unsigned char*) value, value_len, &new_len);
		xdebug_xml_add_text_ex(element, strdup(tmp_base64), new_len, 1, 0);
		efree(tmp_base64);

		xdebug_xml_add_child(node, element);

		xdfree(value);
	} else {
		xdebug_xml_add_text_encodel(node, value, value_len);
	}
}


static int xdebug_array_element_export_xml_node(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, xdebug_xml_node *parent, char *parent_name, xdebug_var_export_options *options)
{
	zval **zv = &zv_nptr;
	xdebug_xml_node *node;
	char *name = NULL;
	int   name_len = 0;
	xdebug_str full_name = XDEBUG_STR_INITIALIZER;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		node = xdebug_xml_node_init("property");
		options->force_extended = 0;

		if (!HASH_KEY_IS_NUMERIC(hash_key)) { /* string key */
			zend_string *i_string = zend_string_init(HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key), 0);
			zend_string *tmp_fullname_zstr;
			
			tmp_fullname_zstr = php_addcslashes(i_string, 0, "\"\\", 2);

			name_len = HASH_APPLY_KEY_LEN(hash_key) - 1;
			name = xdstrndup(HASH_APPLY_KEY_VAL(hash_key), name_len);

			if (parent_name) {
				xdebug_str_add(&full_name, parent_name, 0);
				xdebug_str_addl(&full_name, "[\"", 2, 0);
				xdebug_str_addl(&full_name, tmp_fullname_zstr->val, tmp_fullname_zstr->len - 1, 0);
				xdebug_str_addl(&full_name, "\"]", 2, 0);
			}

			zend_string_release(tmp_fullname_zstr);
			zend_string_release(i_string);
		} else {
			name = xdebug_sprintf(XDEBUG_INT_FMT, index_key);
			name_len = strlen(name);
			if (parent_name) {
				xdebug_str_add(&full_name, xdebug_sprintf("%s[%s]", parent_name, name), 1);
			}
		}

		add_name_attribute_or_element(options, node, "name", 4, name, name_len);
		if (full_name.l) {
			add_name_attribute_or_element(options, node, "fullname", 8, full_name.d, full_name.l);
		}

		xdebug_xml_add_child(parent, node);
		xdebug_var_export_xml_node(zv, full_name.d, node, options, level + 1 TSRMLS_CC);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export_xml_node(xdebug_object_item *item_nptr, zend_ulong index_key, zend_string *hash_key, int level, xdebug_xml_node *parent, char *parent_name, xdebug_var_export_options *options, char *class_name)
{
	xdebug_object_item **item = &item_nptr;
	xdebug_xml_node *node;
	char *full_name = NULL;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		char *modifier;

		node = xdebug_xml_node_init("property");
		options->force_extended = 0;

		if ((*item)->name_len != 0) {
			char *prop_name, *prop_class_name;

			modifier = xdebug_get_property_info((*item)->name, (*item)->name_len + 1, &prop_name, &prop_class_name);

			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				add_name_attribute_or_element(options, node, "name", 4, xdstrdup(prop_name), SIZE_MAX);
			} else {
				add_name_attribute_or_element(options, node, "name", 4, xdebug_sprintf("*%s*%s", prop_class_name, prop_name), SIZE_MAX);
			}

			if (parent_name) {
				if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
					full_name = xdebug_sprintf("%s%s%s", parent_name, (*item)->type == XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY ? "::" : "->", prop_name);
				} else {
					full_name = xdebug_sprintf("%s%s*%s*%s", parent_name, (*item)->type == XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY ? "::" : "->", prop_class_name, prop_name);
				}
				add_name_attribute_or_element(options, node, "fullname", 8, full_name, SIZE_MAX);
			}

			xdfree(prop_name);
			xdfree(prop_class_name);
		} else { /* Numerical property name */
			modifier = "public";

			add_name_attribute_or_element(options, node, "name", 4, xdebug_sprintf(XDEBUG_INT_FMT, (*item)->index_key), SIZE_MAX);

			if (parent_name) {
				full_name = xdebug_sprintf("%s%s" XDEBUG_INT_FMT, parent_name, (*item)->type == XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY ? "::" : "->", (*item)->index_key);
				add_name_attribute_or_element(options, node, "fullname", 8, full_name, SIZE_MAX);
			}
		}

		xdebug_xml_add_attribute_ex(node, "facet", xdebug_sprintf("%s%s", (*item)->type == XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY ? "static " : "", modifier), 0, 1);
		xdebug_xml_add_child(parent, node);
		xdebug_var_export_xml_node(&((*item)->zv), full_name, node, options, level + 1 TSRMLS_CC);
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

void xdebug_attach_uninitialized_var(xdebug_var_export_options *options, xdebug_xml_node *node, char *name)
{
	xdebug_xml_node *contents = NULL;
	char            *tmp_name;

	contents = xdebug_xml_node_init("property");
	options->force_extended = 0;

	tmp_name = prepare_variable_name(name);
	add_name_attribute_or_element(options, contents, "name", 4, xdstrdup(tmp_name), SIZE_MAX);
	add_name_attribute_or_element(options, contents, "fullname", 8, xdstrdup(tmp_name), SIZE_MAX);
	xdfree(tmp_name);

	xdebug_xml_add_attribute(contents, "type", "uninitialized");
	xdebug_xml_add_child(node, contents);
}

void xdebug_attach_property_with_contents(zend_property_info *prop_info, xdebug_xml_node *node, xdebug_var_export_options *options, zend_class_entry *class_entry, char *class_name, int *children_count)
{
	char               *modifier;
	xdebug_xml_node    *contents = NULL;
	char               *prop_name, *prop_class_name;

	if ((prop_info->flags & ZEND_ACC_STATIC) == 0) {
		return;
	}

	(*children_count)++;
	modifier = xdebug_get_property_info(STR_NAME_VAL(prop_info->name), STR_NAME_LEN(prop_info->name) + 1, &prop_name, &prop_class_name);

	if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
		contents = xdebug_get_zval_value_xml_node_ex(prop_name, &class_entry->static_members_table[prop_info->offset], XDEBUG_VAR_TYPE_STATIC, options TSRMLS_CC);
	} else{
		char *priv_name = xdebug_sprintf("*%s*%s", prop_class_name, prop_name);
		contents = xdebug_get_zval_value_xml_node_ex(priv_name, &class_entry->static_members_table[prop_info->offset], XDEBUG_VAR_TYPE_STATIC, options TSRMLS_CC);
		xdfree(priv_name);
	}

	xdfree(prop_name);
	xdfree(prop_class_name);

	if (contents) {
		xdebug_xml_add_attribute_ex(contents, "facet", xdebug_sprintf("static %s", modifier), 0, 1);
		xdebug_xml_add_child(node, contents);
	} else {
		xdebug_attach_uninitialized_var(options, node, (char *) prop_info->name);
	}
}

void xdebug_attach_static_vars(xdebug_xml_node *node, xdebug_var_export_options *options, zend_class_entry *ce TSRMLS_DC)
{
	HashTable        *static_members = &ce->properties_info;
	int               children = 0;
	xdebug_xml_node  *static_container;
	zend_property_info *zpi;

	static_container = xdebug_xml_node_init("property");
	options->force_extended = 0;

	xdebug_xml_add_attribute(static_container, "name", "::");
	xdebug_xml_add_attribute(static_container, "fullname", "::");
	xdebug_xml_add_attribute(static_container, "type", "object");
	xdebug_xml_add_attribute_ex(static_container, "classname", xdstrdup(STR_NAME_VAL(ce->name)), 0, 1);

	ZEND_HASH_INC_APPLY_COUNT(static_members);
	ZEND_HASH_FOREACH_PTR(static_members, zpi) {
		xdebug_attach_property_with_contents(zpi, static_container, options, ce, STR_NAME_VAL(ce->name), &children);
	} ZEND_HASH_FOREACH_END();
	ZEND_HASH_DEC_APPLY_COUNT(static_members);

	xdebug_xml_add_attribute(static_container, "children", children > 0 ? "1" : "0");
	xdebug_xml_add_attribute_ex(static_container, "numchildren", xdebug_sprintf("%d", children), 0, 1);

	xdebug_xml_add_child(node, static_container);
}

void xdebug_var_export_xml_node(zval **struc, char *name, xdebug_xml_node *node, xdebug_var_export_options *options, int level TSRMLS_DC)
{
	HashTable *myht;
	char *class_name;
	size_t class_name_len;
	zend_ulong num;
	zend_string *key;
	zval *z_val;
	xdebug_object_item *xoi_val;
	zval *tmpz;

	if (Z_TYPE_P(*struc) == IS_INDIRECT) {
		tmpz = ((*struc)->value.zv);
		struc = &tmpz;
	}
	if (Z_TYPE_P(*struc) == IS_REFERENCE) {
		tmpz = &((*struc)->value.ref->val);
		struc = &tmpz;
	}

	switch (Z_TYPE_P(*struc)) {
		case IS_TRUE:
		case IS_FALSE:
			xdebug_xml_add_attribute(node, "type", "bool");
			add_unencoded_text_value_attribute_or_element(options, node, xdebug_sprintf("%d", Z_TYPE_P(*struc) == IS_TRUE ? 1 : 0));
			break;

		case IS_NULL:
			xdebug_xml_add_attribute(node, "type", "null");
			break;

		case IS_LONG:
			xdebug_xml_add_attribute(node, "type", "int");
			add_unencoded_text_value_attribute_or_element(options, node, xdebug_sprintf(XDEBUG_INT_FMT, Z_LVAL_P(*struc)));
			break;

		case IS_DOUBLE:
			xdebug_xml_add_attribute(node, "type", "float");
			add_unencoded_text_value_attribute_or_element(options, node, xdebug_sprintf("%.*G", (int) EG(precision), Z_DVAL_P(*struc)));
			break;

		case IS_STRING:
			xdebug_xml_add_attribute(node, "type", "string");
			if (options->max_data == 0 || (size_t) Z_STRLEN_P(*struc) <= (size_t) options->max_data) {
				add_encoded_text_value_attribute_or_element(options, node, xdstrndup(Z_STRVAL_P(*struc), Z_STRLEN_P(*struc)), Z_STRLEN_P(*struc));
			} else {
				add_encoded_text_value_attribute_or_element(options, node, xdstrndup(Z_STRVAL_P(*struc), options->max_data), options->max_data);
			}
			xdebug_xml_add_attribute_ex(node, "size", xdebug_sprintf("%d", Z_STRLEN_P(*struc)), 0, 1);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);
			xdebug_xml_add_attribute(node, "type", "array");
			xdebug_xml_add_attribute(node, "children", myht->nNumOfElements > 0?"1":"0");
			if (ZEND_HASH_GET_APPLY_COUNT(myht) < 1) {
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

					ZEND_HASH_INC_APPLY_COUNT(myht);
					ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, z_val) {
						xdebug_array_element_export_xml_node(z_val, num, key, level, node, name, options);
					} ZEND_HASH_FOREACH_END();
					ZEND_HASH_DEC_APPLY_COUNT(myht);
				}
			} else {
				xdebug_xml_add_attribute(node, "recursive", "1");
			}
			break;

		case IS_OBJECT: {
			HashTable *merged_hash;
			zend_class_entry *ce;
			int is_temp;
			zend_property_info *zpi_val;

			ALLOC_HASHTABLE(merged_hash);
			zend_hash_init(merged_hash, 128, NULL, NULL, 0);

			class_name = (char*) STR_NAME_VAL(Z_OBJCE_P(*struc)->name);
			class_name_len = STR_NAME_LEN(Z_OBJCE_P(*struc)->name);
			ce = xdebug_fetch_class(class_name, class_name_len, ZEND_FETCH_CLASS_DEFAULT TSRMLS_CC);

			/* Adding static properties */
			if (&ce->properties_info) {
				ZEND_HASH_INC_APPLY_COUNT(&ce->properties_info);
				ZEND_HASH_FOREACH_PTR(&ce->properties_info, zpi_val) {
					object_item_add_zend_prop_to_merged_hash(zpi_val, merged_hash, (int) XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY, ce);
				} ZEND_HASH_FOREACH_END();
				ZEND_HASH_DEC_APPLY_COUNT(&ce->properties_info);
			}

			/* Adding normal properties */
			myht = xdebug_objdebug_pp(struc, &is_temp TSRMLS_CC);
			if (myht) {
				zval *tmp_val;

				ZEND_HASH_INC_APPLY_COUNT(myht);
				ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, tmp_val) {
					object_item_add_to_merged_hash(tmp_val, num, key, merged_hash, (int) XDEBUG_OBJECT_ITEM_TYPE_PROPERTY);
				} ZEND_HASH_FOREACH_END();
				ZEND_HASH_DEC_APPLY_COUNT(myht);
			}

			xdebug_xml_add_attribute(node, "type", "object");
			add_name_attribute_or_element(options, node, "classname", 9, xdstrdup(class_name), -1);
			xdebug_xml_add_attribute(node, "children", merged_hash->nNumOfElements ? "1" : "0");

			if (ZEND_HASH_GET_APPLY_COUNT(merged_hash) < 1) {
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

					ZEND_HASH_INC_APPLY_COUNT(merged_hash);
					ZEND_HASH_FOREACH_KEY_PTR(merged_hash, num, key, xoi_val) {
						xdebug_object_element_export_xml_node(xoi_val, num, key, level, node, name, options, class_name);
					} ZEND_HASH_FOREACH_END();
					ZEND_HASH_DEC_APPLY_COUNT(merged_hash);
				}
			}

			zend_hash_destroy(merged_hash);
			FREE_HASHTABLE(merged_hash);
			break;
		}

		case IS_RESOURCE: {
			char *type_name;

			xdebug_xml_add_attribute(node, "type", "resource");
			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc) TSRMLS_CC);
			xdebug_xml_add_text(node, xdebug_sprintf("resource id='%ld' type='%s'", Z_RES_P(*struc)->handle, type_name ? type_name : "Unknown"));
			break;
		}

		case IS_UNDEF:
			xdebug_xml_add_attribute(node, "type", "uninitialized");
			break;

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
	options->force_extended = 0;

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

			case XDEBUG_VAR_TYPE_CONSTANT:
				short_name = xdstrdup(name);
				full_name =  xdstrdup(name);
				break;
		}

		add_name_attribute_or_element(options, node, "name", 4, short_name, SIZE_MAX);
		add_name_attribute_or_element(options, node, "fullname", 8, full_name, SIZE_MAX);
	}
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

static int xdebug_array_element_export_fancy(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, xdebug_str *str, int debug_zval, xdebug_var_export_options *options)
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
		xdebug_var_export_fancy(zv, str, level + 1, debug_zval, options TSRMLS_CC);
	}
	if (options->runtime[level].current_element_nr == options->runtime[level].end_element_nr) {
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 4) - 2, ""), 1);
		xdebug_str_addl(str, "<i>more elements...</i>\n", 24, 0);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export_fancy(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, xdebug_str *str, int debug_zval, xdebug_var_export_options *options, char *class_name)
{
	zval **zv = &zv_nptr;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		xdebug_str_add(str, xdebug_sprintf("%*s", (level * 4) - 2, ""), 1);

		if (!HASH_KEY_IS_NUMERIC(hash_key)) {
			char *prop_name, *modifier, *prop_class_name;

			modifier = xdebug_get_property_info((char*) HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key), &prop_name, &prop_class_name);
			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				xdebug_str_add(str, xdebug_sprintf("<i>%s</i> '%s' <font color='%s'>=&gt;</font> ", modifier, prop_name, COLOR_POINTER), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("<i>%s</i> '%s' <small>(%s)</small> <font color='%s'>=&gt;</font> ", modifier, prop_name, prop_class_name, COLOR_POINTER), 1);
			}

			xdfree(prop_name);
			xdfree(prop_class_name);
		} else {
			xdebug_str_add(str, xdebug_sprintf("<i>public</i> %d <font color='%s'>=&gt;</font> ", index_key, COLOR_POINTER), 1);
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
	size_t    newlen;
	int       is_temp;
	zend_ulong num;
	zend_string *key;
	zval *val;
	zval *tmpz;

	if (debug_zval) {
		if (Z_TYPE_P(*struc) >= IS_STRING && Z_TYPE_P(*struc) != IS_INDIRECT) {
			xdebug_str_add(str, xdebug_sprintf("<i>(refcount=%d, is_ref=%d)</i>", (*struc)->value.counted->gc.refcount, Z_TYPE_P(*struc) == IS_REFERENCE), 1);
		} else {
			xdebug_str_add(str, "<i>(refcount=0, is_ref=0)</i>", 0);
		}
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
			if (ZEND_HASH_GET_APPLY_COUNT(myht) < 1) {
				xdebug_str_add(str, xdebug_sprintf("<b>array</b> <i>(size=%d)</i>\n", myht->nNumOfElements), 1);
				if (level <= options->max_depth) {
					if (myht->nNumOfElements) {
						options->runtime[level].current_element_nr = 0;
						options->runtime[level].start_element_nr = 0;
						options->runtime[level].end_element_nr = options->max_children;

						ZEND_HASH_INC_APPLY_COUNT(myht);
						ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
							xdebug_array_element_export_fancy(val, num, key, level, str, debug_zval, options);
						} ZEND_HASH_FOREACH_END();
						ZEND_HASH_DEC_APPLY_COUNT(myht);
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
			myht = xdebug_objdebug_pp(struc, &is_temp TSRMLS_CC);
			xdebug_str_add(str, xdebug_sprintf("\n%*s", (level - 1) * 4, ""), 1);
			if (ZEND_HASH_GET_APPLY_COUNT(myht) < 1) {
				char *class_name = (char*) STR_NAME_VAL(Z_OBJCE_P(*struc)->name);
				xdebug_str_add(str, xdebug_sprintf("<b>object</b>(<i>%s</i>)", class_name), 1);
				xdebug_str_add(str, xdebug_sprintf("[<i>%d</i>]\n", Z_OBJ_HANDLE_P(*struc)), 1);

				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					ZEND_HASH_INC_APPLY_COUNT(myht);
					ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, val) {
						xdebug_object_element_export_fancy(val, num, key, level, str, debug_zval, options, class_name);
					} ZEND_HASH_FOREACH_END();
					ZEND_HASH_DEC_APPLY_COUNT(myht);
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 4) - 2, ""), 1);
				}
			} else {
				xdebug_str_add(str, xdebug_sprintf("<i>&amp;</i><b>object</b>(<i>%s</i>)", STR_NAME_VAL(Z_OBJCE_P(*struc)->name)), 1);
				xdebug_str_add(str, xdebug_sprintf("[<i>%d</i>]\n", Z_OBJ_HANDLE_P(*struc)), 1);
			}
			if (is_temp) {
				zend_hash_destroy(myht);
				efree(myht);
			}

			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc) TSRMLS_CC);
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

char* xdebug_get_zval_value_fancy(char *name, zval *val, int *len, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	xdebug_str str = XDEBUG_STR_INITIALIZER;
	int default_options = 0;

	if (!options) {
		options = xdebug_var_export_options_from_ini(TSRMLS_C);
		default_options = 1;
	}

	xdebug_str_addl(&str, "<pre class='xdebug-var-dump' dir='ltr'>", 39, 0);
	if (options->show_location && !debug_zval) {
		if (strlen(XG(file_link_format)) > 0) {
			char *file_link;

			xdebug_format_file_link(&file_link, zend_get_executed_filename(TSRMLS_C), zend_get_executed_lineno(TSRMLS_C) TSRMLS_CC);
			xdebug_str_add(&str, xdebug_sprintf("\n<small><a href='%s'>%s:%d</a>:</small>", file_link, zend_get_executed_filename(TSRMLS_C), zend_get_executed_lineno(TSRMLS_C)), 1);
			xdfree(file_link);
		} else {
			xdebug_str_add(&str, xdebug_sprintf("\n<small>%s:%d:</small>", zend_get_executed_filename(TSRMLS_C), zend_get_executed_lineno(TSRMLS_C)), 1);
		}
	}
	xdebug_var_export_fancy(&val, (xdebug_str*) &str, 1, debug_zval, options TSRMLS_CC);
	xdebug_str_addl(&str, "</pre>", 6, 0);

	if (default_options) {
		xdfree(options->runtime);
		xdfree(options);
	}

	*len = str.l;
	return str.d;
}

char* xdebug_get_zval_value_serialized(zval *val, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	zend_object *orig_exception = EG(exception);
	php_serialize_data_t var_hash;
	smart_str buf = { 0, 0 };

	if (!val) {
		return NULL;
	}

	PHP_VAR_SERIALIZE_INIT(var_hash);
	XG(in_var_serialisation) = 1;
	EG(exception) = NULL;
	php_var_serialize(&buf, val, &var_hash TSRMLS_CC);
	orig_exception = EG(exception) = orig_exception;
	XG(in_var_serialisation) = 0;
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (buf.a) {
		int new_len;
		char *tmp_base64, *tmp_ret;

		/* now we need to base64 it */
		tmp_base64 = (char*) xdebug_base64_encode((unsigned char*) buf.s->val, buf.s->len, &new_len);

		/* we need a malloc'ed and not an emalloc'ed string */
		tmp_ret = xdstrdup(tmp_base64);

		efree(tmp_base64);
		smart_str_free(&buf);

		return tmp_ret;
	} else {
		return NULL;
	}
}

static void xdebug_var_synopsis_fancy(zval **struc, xdebug_str *str, int level, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	HashTable *myht;
	zval *tmpz;

	if (debug_zval) {
		if (Z_TYPE_P(*struc) >= IS_STRING && Z_TYPE_P(*struc) != IS_INDIRECT) {
			xdebug_str_add(str, xdebug_sprintf("<i>(refcount=%d, is_ref=%d)</i>", (*struc)->value.counted->gc.refcount, Z_TYPE_P(*struc) == IS_REFERENCE), 1);
		} else {
			xdebug_str_add(str, "<i>(refcount=0, is_ref=0)</i>", 0);
		}
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

			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc) TSRMLS_CC);
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

char* xdebug_get_zval_synopsis_fancy(char *name, zval *val, int *len, int debug_zval, xdebug_var_export_options *options TSRMLS_DC)
{
	xdebug_str str = XDEBUG_STR_INITIALIZER;
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

char* xdebug_xmlize(char *string, size_t len, size_t *newlen)
{
	if (len) {
		char *tmp;
		char *tmp2;

		tmp = xdebug_str_to_str(string, len, "&", 1, "&amp;", 5, &len);

		tmp2 = xdebug_str_to_str(tmp, len, ">", 1, "&gt;", 4, &len);
		efree(tmp);

		tmp = xdebug_str_to_str(tmp2, len, "<", 1, "&lt;", 4, &len);
		efree(tmp2);

		tmp2 = xdebug_str_to_str(tmp, len, "\"", 1, "&quot;", 6, &len);
		efree(tmp);

		tmp = xdebug_str_to_str(tmp2, len, "'", 1, "&#39;", 5, &len);
		efree(tmp2);

		tmp2 = xdebug_str_to_str(tmp, len, "\n", 1, "&#10;", 5, &len);
		efree(tmp);

		tmp = xdebug_str_to_str(tmp2, len, "\r", 1, "&#13;", 5, &len);
		efree(tmp2);

		tmp2 = xdebug_str_to_str(tmp, len, "\0", 1, "&#0;", 4, (size_t*) newlen);
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
	char *tmp_target = NULL, *p, *retval;

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

		case XFUNC_ZEND_PASS:
			return xdstrdup("{zend_pass}");
			break;

		default:
			return xdstrdup("{unknown}");
	}
}
