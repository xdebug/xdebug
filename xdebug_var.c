/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2015 Derick Rethans                               |
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
#include "ext/standard/php_smart_str.h"
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
#if PHP_VERSION_ID >= 50500
# define T(offset) (*EX_TMP_VAR(zdata, offset))
#else
# define T(offset) (*(temp_variable *)((char*)zdata->Ts + offset))
#endif

zval *xdebug_get_zval(zend_execute_data *zdata, int node_type, znode_op *node, int *is_var)
{
	switch (node_type) {
		case IS_CONST:
			return node->zv;
			break;

		case IS_TMP_VAR:
			*is_var = 1;
			return &T(node->var).tmp_var;
			break;

		case IS_VAR:
			*is_var = 1;
			if (T(node->var).var.ptr) {
				return T(node->var).var.ptr;
			}
			break;

		case IS_CV: {
			zval **tmp;
			tmp = zend_get_compiled_variable_value(zdata, node->constant);
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
	char *name;
	zend_uint name_len;
	zend_class_entry *tmp_ce;

	if (Z_TYPE_P(z) != IS_OBJECT) {
		return NULL;
	}

	tmp_ce = zend_get_class_entry(z TSRMLS_CC);
	if (Z_OBJ_HT_P(z)->get_class_name == NULL ||
		Z_OBJ_HT_P(z)->get_class_name(z, (const char **) &name, &name_len, 0 TSRMLS_CC) != SUCCESS) {

		if (!tmp_ce) {
			return NULL;
		}

		*length = tmp_ce->name_length;
		*ce = tmp_ce;
		return estrdup(tmp_ce->name);
	} else {
		*ce = tmp_ce;
	}

	*length = name_len;
	return name;
}

static char* prepare_search_key(char *name, int *name_length, char *prefix, int prefix_length)
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

static zval **get_arrayobject_storage(zval *parent TSRMLS_DC)
{
	zval **tmp = NULL;
	int is_temp;
	HashTable *properties = Z_OBJDEBUG_P(parent, is_temp);

	if (zend_hash_find(properties, "\0ArrayObject\0storage", sizeof("*ArrayObject*storage"), (void **) &tmp) == SUCCESS) {
		return tmp;
	}

	return NULL;
}

static zval **get_splobjectstorage_storage(zval *parent TSRMLS_DC)
{
	zval **tmp = NULL;
	int is_temp;
	HashTable *properties = Z_OBJDEBUG_P(parent, is_temp);

	if (zend_hash_find(properties, "\0SplObjectStorage\0storage", sizeof("*SplObjectStorage*storage"), (void **) &tmp) == SUCCESS) {
		return tmp;
	}

	return NULL;
}

static zval **get_arrayiterator_storage(zval *parent TSRMLS_DC)
{
	zval **tmp = NULL;
	int is_temp;
	HashTable *properties = Z_OBJDEBUG_P(parent, is_temp);

	if (zend_hash_find(properties, "\0ArrayIterator\0storage", sizeof("*ArrayIterator*storage"), (void **) &tmp) == SUCCESS) {
		return tmp;
	}

	return NULL;
}

static zval* fetch_zval_from_symbol_table(zval *parent, char* name, int name_length, int type, char* ccn, int ccnl, zend_class_entry *cce TSRMLS_DC)
{
	HashTable *ht = NULL;
	zval **retval_pp = NULL, *retval_p = NULL;
	char  *element = NULL;
	int    element_length = name_length;
	zend_property_info *zpp;

	if (parent) {
		ht = fetch_ht_from_zval(parent TSRMLS_CC);
	}

	switch (type) {
		case XF_ST_STATIC_ROOT:
		case XF_ST_STATIC_PROPERTY:
			/* First we try a public,private,protected property */
			element = prepare_search_key(name, &element_length, "", 0);
			if (cce && &cce->properties_info && zend_hash_find(&cce->properties_info, element, element_length + 1, (void **) &zpp) == SUCCESS) {
				retval_p = cce->static_members_table[zpp->offset];
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
					if (cce && &cce->properties_info && zend_hash_find(&cce->properties_info, element, element_length + 1, (void **) &zpp) == SUCCESS) {
						retval_p = cce->static_members_table[zpp->offset];
						goto cleanup;
					}
				}
			}

			break;

		case XF_ST_ROOT:
			/* Check for compiled vars */
			element = prepare_search_key(name, &element_length, "", 0);
			if (XG(active_execute_data) && XG(active_execute_data)->op_array) {
				int i = 0;
				ulong hash_value = zend_inline_hash_func(element, element_length + 1);
				zend_op_array *opa = XG(active_execute_data)->op_array;
				zval **CV;

				while (i < opa->last_var) {
					if (opa->vars[i].hash_value == hash_value &&
						opa->vars[i].name_len == element_length &&
						strcmp(opa->vars[i].name, element) == 0)
					{
#if PHP_VERSION_ID >= 50500
						CV = (*EX_CV_NUM(XG(active_execute_data), i));
#else
						CV = XG(active_execute_data)->CVs[i];
#endif
						if (CV) {
							retval_p = *CV;
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
					retval_p = XG(This);
				} else {
					retval_p = NULL;
				}
				goto cleanup;
			}

			if (ht && zend_hash_find(ht, element, name_length + 1, (void **) &retval_pp) == SUCCESS) {
				retval_p = *retval_pp;
				goto cleanup;
			}
			break;

		case XF_ST_ARRAY_INDEX_NUM:
			element = prepare_search_key(name, &name_length, "", 0);
			if (ht && zend_hash_index_find(ht, strtoul(element, NULL, 10), (void **) &retval_pp) == SUCCESS) {
				retval_p = *retval_pp;
				goto cleanup;
			}
			break;

		case XF_ST_OBJ_PROPERTY:
			/* First we try an object handler */
			if (cce) {
				zval *tmp_val;

				tmp_val = zend_read_property(cce, parent, name, name_length, 0 TSRMLS_CC);
				if (tmp_val && tmp_val != EG(uninitialized_zval_ptr)) {
					retval_p = tmp_val;
					goto cleanup;
				}
			}

			/* Then we try a public property */
			element = prepare_search_key(name, &element_length, "", 0);
			if (ht && zend_symtable_find(ht, element, element_length + 1, (void **) &retval_pp) == SUCCESS) {
				retval_p = *retval_pp;
				goto cleanup;
			}
			element_length = name_length;

			/* Then we try it again as protected property */
			free(element);
			element = prepare_search_key(name, &element_length, "*", 1);
			if (ht && zend_hash_find(ht, element, element_length + 1, (void **) &retval_pp) == SUCCESS) {
				retval_p = *retval_pp;
				goto cleanup;
			}
			element_length = name_length;

			/* Then we try it again as private property */
			free(element);
			element = prepare_search_key(name, &element_length, ccn, ccnl);
			if (ht && zend_hash_find(ht, element, element_length + 1, (void **) &retval_pp) == SUCCESS) {
				retval_p = *retval_pp;
				goto cleanup;
			}
			element_length = name_length;
					
			/* All right, time for a mega hack. It's SplObjectStorage access time! */
			if (strncmp(ccn, "SplObjectStorage", ccnl) == 0 && strncmp(name, "storage", name_length) == 0) {
				element = NULL;
				if ((retval_pp = get_splobjectstorage_storage(parent TSRMLS_CC)) != NULL) {
					if (retval_pp) {
						retval_p = *retval_pp;
						goto cleanup;
					}
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
						element = NULL;
						if ((retval_pp = get_arrayobject_storage(parent TSRMLS_CC)) != NULL) {
							if (retval_pp) {
								retval_p = *retval_pp;
								goto cleanup;
							}
						}
					}
					/* All right, time for a mega hack. It's ArrayIterator access time! */
					if (strncmp(name + 1, "ArrayIterator", secondStar - name - 1) == 0 && strncmp(secondStar + 1, "storage", element_length) == 0) {
						element = NULL;
						if ((retval_pp = get_arrayiterator_storage(parent TSRMLS_CC)) != NULL) {
							if (retval_pp) {
								retval_p = *retval_pp;
								goto cleanup;
							}
						}
					}

					/* The normal one */
					element = prepare_search_key(secondStar + 1, &element_length, name + 1, secondStar - name - 1);
					if (ht && zend_hash_find(ht, element, element_length + 1, (void **) &retval_pp) == SUCCESS) {
						retval_p = *retval_pp;
						goto cleanup;
					}
				}
			}

			break;
	}
cleanup:
	if (element) {
		free(element);
	}
	return retval_p;
}

zval* xdebug_get_php_symbol(char* name, int name_length TSRMLS_DC)
{
	int        found = -1;
	int        state = 0;
	char     **p = &name;
	char      *keyword = NULL, *keyword_end = NULL;
	int        type = XF_ST_ROOT;
	zval      *retval = NULL;
	char      *current_classname = NULL;
	zend_class_entry *current_ce = NULL;
	int        cc_length = 0;
	char       quotechar = 0;

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
							retval = fetch_zval_from_symbol_table(retval, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce TSRMLS_CC);
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
							retval = fetch_zval_from_symbol_table(retval, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce TSRMLS_CC);
							if (current_classname) {
								efree(current_classname);
							}
							current_classname = NULL;
							cc_length = 0;
							current_ce = NULL;
							if (retval) {
								current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce TSRMLS_CC);
							}
							keyword = NULL;
						}
						state = 2;
						type = XF_ST_OBJ_PROPERTY;
					} else if (*p[0] == ':') {
						keyword_end = *p;
						if (keyword) {
							retval = fetch_zval_from_symbol_table(retval, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce TSRMLS_CC);
							if (current_classname) {
								efree(current_classname);
							}
							current_classname = NULL;
							cc_length = 0;
							if (retval) {
								current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce TSRMLS_CC);
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
					if (*p[0] == quotechar) {
						quotechar = 0;
						state = 5;
						keyword_end = *p;
						retval = fetch_zval_from_symbol_table(retval, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce TSRMLS_CC);
						if (current_classname) {
							efree(current_classname);
						}
						current_classname = NULL;
						cc_length = 0;
						if (retval) {
							current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce TSRMLS_CC);
						}
						keyword = NULL;
					}
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
						retval = fetch_zval_from_symbol_table(retval, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce TSRMLS_CC);
						if (current_classname) {
							efree(current_classname);
						}
						current_classname = NULL;
						cc_length = 0;
						if (retval) {
							current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce TSRMLS_CC);
						}
						keyword = NULL;
					}
					break;
				case 7: /* special cases, started with a ":" */
					if (*p[0] == ':') {
						state = 1;
						keyword_end = *p;

						if (strncmp(keyword, "::", 2) == 0) { /* static class properties */
							zend_class_entry *ce = zend_fetch_class(XG(active_fse)->function.class, strlen(XG(active_fse)->function.class), ZEND_FETCH_CLASS_SELF TSRMLS_CC);

							current_classname = estrdup(ce->name);
							cc_length = strlen(ce->name);
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
		retval = fetch_zval_from_symbol_table(retval, keyword, *p - keyword, type, current_classname, cc_length, current_ce TSRMLS_CC);
	}
	if (current_classname) {
		efree(current_classname);
	}
	return retval;
}

char* xdebug_get_property_info(char *mangled_property, int mangled_len, char **property_name, char **class_name)
{
	const char *prop_name, *cls_name;

	zend_unmangle_property_name(mangled_property, mangled_len, &cls_name, &prop_name);
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
	options->show_location = XG(overload_var_dump) > 1;

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

xdebug_var_export_options xdebug_var_nolimit_options = { XDEBUG_MAX_INT, XDEBUG_MAX_INT, 1023, 1, 0, NULL, 0 };

xdebug_var_export_options* xdebug_var_get_nolimit_options(TSRMLS_D)
{
	return &xdebug_var_nolimit_options;
}

/*****************************************************************************
** Normal variable printing routines
*/

static int xdebug_array_element_export(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, debug_zval;
	xdebug_str *str;
	xdebug_var_export_options *options;

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

static int xdebug_object_element_export(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, debug_zval;
	xdebug_str *str;
	xdebug_var_export_options *options;
	char *prop_name, *class_name, *modifier, *prop_class_name;

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
		} else {
			xdebug_str_add(str, xdebug_sprintf("public $%d = ", hash_key->h), 1);
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
	int       is_temp;

	if (!struc || !(*struc)) {
		return;
	}
	if (debug_zval) {
		xdebug_str_add(str, xdebug_sprintf("(refcount=%d, is_ref=%d)=", (*struc)->refcount__gc, (*struc)->is_ref__gc), 1);
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

					zend_hash_apply_with_arguments(myht TSRMLS_CC, (apply_func_args_t) xdebug_array_element_export, 4, level, str, debug_zval, options);
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
			myht = Z_OBJDEBUG_PP(struc, is_temp);
			if (myht->nApplyCount < 1) {
				char *class_name;
				zend_uint class_name_len;

				zend_get_object_classname(*struc, (const char **) &class_name, &class_name_len TSRMLS_CC);
				xdebug_str_add(str, xdebug_sprintf("class %s { ", class_name), 1);

				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					zend_hash_apply_with_arguments(myht TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export, 5, level, str, debug_zval, options, class_name);
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
			if (is_temp) {
				zend_hash_destroy(myht);
				efree(myht);
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
		xdebug_str_add(str, xdebug_sprintf("(refcount=%d, is_ref=%d)=", (*struc)->refcount__gc, (*struc)->is_ref__gc), 1);
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

static int xdebug_array_element_export_text_ansi(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, mode, debug_zval;
	xdebug_str *str;
	xdebug_var_export_options *options;

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

static int xdebug_object_element_export_text_ansi(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, mode, debug_zval;
	xdebug_str *str;
	xdebug_var_export_options *options;
	char *prop_name, *class_name, *modifier;

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
		} else {
			xdebug_str_add(str, xdebug_sprintf("%s%spublic%s%s ${%d} %s=>%s\n",
			               ANSI_COLOR_MODIFIER, ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF, ANSI_COLOR_RESET, 
			               hash_key->h, ANSI_COLOR_POINTER, ANSI_COLOR_RESET), 1);
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

	if (!struc || !(*struc)) {
		return;
	}
	
	xdebug_str_add(str, xdebug_sprintf("%*s", (level * 2) - 2, ""), 1);
	if (debug_zval) {
		xdebug_str_add(str, xdebug_sprintf("(refcount=%d, is_ref=%d)=", (*struc)->refcount__gc, (*struc)->is_ref__gc), 1);
	}

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

					zend_hash_apply_with_arguments(myht TSRMLS_CC, (apply_func_args_t) xdebug_array_element_export_text_ansi, 5, level, mode, str, debug_zval, options);
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 2), ""), 1);
				}
				xdebug_str_add(str, xdebug_sprintf("%*s}", (level * 2) - 2 , ""), 1);
			} else {
				xdebug_str_add(str, xdebug_sprintf("&%sarray%s", ANSI_COLOR_BOLD, ANSI_COLOR_BOLD_OFF), 1);
			}
			break;

		case IS_OBJECT:
			myht = Z_OBJDEBUG_PP(struc, is_temp);
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

					zend_hash_apply_with_arguments(myht TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export_text_ansi, 5, level, mode, str, debug_zval, options);
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

	if (options->show_location) {
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

	if (!struc || !(*struc)) {
		return;
	}
	if (debug_zval) {
		xdebug_str_add(str, xdebug_sprintf("(refcount=%d, is_ref=%d)=", (*struc)->refcount__gc, (*struc)->is_ref__gc), 1);
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

	if (options->show_location) {
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
	ulong index;
	zval *zv;
} xdebug_object_item;

static int object_item_add_to_merged_hash(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
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
	item->index = hash_key->h;

	zend_hash_next_index_insert(merged, &item, sizeof(xdebug_object_item*), NULL);

	return 0;
}

#if PHP_VERSION_ID >= 50400
static int object_item_add_zend_prop_to_merged_hash(zend_property_info *zpp TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
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

static int xdebug_array_element_export_xml_node(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_xml_node *parent;
	xdebug_xml_node *node;
	xdebug_var_export_options *options;
	char *name = NULL, *parent_name = NULL;
	int   name_len = 0;
	xdebug_str full_name = { 0, 0, NULL };

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

static int xdebug_object_element_export_xml_node(xdebug_object_item **item TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_xml_node *parent;
	xdebug_xml_node *node;
	xdebug_var_export_options *options;
	char *prop_name, *modifier, *class_name, *prop_class_name;
	char *parent_name = NULL, *full_name = NULL;

	level  = va_arg(args, int);
	parent = va_arg(args, xdebug_xml_node*);
	full_name = parent_name = va_arg(args, char *);
	options = va_arg(args, xdebug_var_export_options*);
	class_name = va_arg(args, char *);

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		node = xdebug_xml_node_init("property");

		if ((*item)->name_len != 0) {
			modifier = xdebug_get_property_info((*item)->name, (*item)->name_len, &prop_name, &prop_class_name);

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
		} else { /* Numerical property name */
			modifier = "public";

			xdebug_xml_add_attribute_ex(node, "name", xdebug_sprintf("%ld", (*item)->index), 0, 1);

			if (parent_name) {
				full_name = xdebug_sprintf("%s%s%ld", parent_name, (*item)->type == XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY ? "::" : "->", (*item)->index);
				xdebug_xml_add_attribute_ex(node, "fullname", full_name, 0, 1);
			}
		}

		xdebug_xml_add_attribute_ex(node, "facet", xdebug_sprintf("%s%s", (*item)->type == XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY ? "static " : "", modifier), 0, 1);
		xdebug_xml_add_attribute_ex(node, "address", xdebug_sprintf("%ld", (long) (*item)->zv), 0, 1);
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

void xdebug_attach_property_with_contents(zend_property_info *prop_info TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
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

void xdebug_attach_static_vars(xdebug_xml_node *node, xdebug_var_export_options *options, zend_class_entry *ce TSRMLS_DC)
{
	HashTable        *static_members = &ce->properties_info;
	int               children = 0;
	xdebug_xml_node  *static_container;

	static_container = xdebug_xml_node_init("property");
	xdebug_xml_add_attribute(static_container, "name", "::");
	xdebug_xml_add_attribute(static_container, "fullname", "::");
	xdebug_xml_add_attribute(static_container, "type", "object");
	xdebug_xml_add_attribute_ex(static_container, "classname", xdstrdup(ce->name), 0, 1);

	zend_hash_apply_with_arguments(static_members TSRMLS_CC, (apply_func_args_t) xdebug_attach_property_with_contents, 5, static_container, options, ce, ce->name, &children); 
	xdebug_xml_add_attribute(static_container, "children", children > 0 ? "1" : "0");
	xdebug_xml_add_attribute_ex(static_container, "numchildren", xdebug_sprintf("%d", children), 0, 1);

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
					zend_hash_apply_with_arguments(myht TSRMLS_CC, (apply_func_args_t) xdebug_array_element_export_xml_node, 4, level, node, name, options);
				}
			} else {
				xdebug_xml_add_attribute(node, "recursive", "1");
			}
			break;

		case IS_OBJECT: {
			HashTable *merged_hash;
			zend_class_entry *ce;
			int is_temp;

			ALLOC_HASHTABLE(merged_hash);
			zend_hash_init(merged_hash, 128, NULL, NULL, 0);

			zend_get_object_classname(*struc, (const char **) &class_name, &class_name_len TSRMLS_CC);
			ce = zend_fetch_class(class_name, strlen(class_name), ZEND_FETCH_CLASS_DEFAULT TSRMLS_CC);

			/* Adding static properties */
			if (&ce->properties_info) {
				zend_hash_apply_with_arguments(&ce->properties_info TSRMLS_CC, (apply_func_args_t) object_item_add_zend_prop_to_merged_hash, 3, merged_hash, (int) XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY, ce);
			}

			/* Adding normal properties */
			myht = Z_OBJDEBUG_PP(struc, is_temp);
			if (myht) {
				zend_hash_apply_with_arguments(myht TSRMLS_CC, (apply_func_args_t) object_item_add_to_merged_hash, 2, merged_hash, (int) XDEBUG_OBJECT_ITEM_TYPE_PROPERTY);
			}

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
					zend_hash_apply_with_arguments(merged_hash TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export_xml_node, 5, level, node, name, options, class_name);
				}
			}
			efree(class_name);

			zend_hash_destroy(merged_hash);
			FREE_HASHTABLE(merged_hash);
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

			case XDEBUG_VAR_TYPE_CONSTANT:
				short_name = xdstrdup(name);
				full_name =  xdstrdup(name);
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

static int xdebug_array_element_export_fancy(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, debug_zval, newlen;
	char *tmp_str;
	xdebug_str *str;
	xdebug_var_export_options *options;

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

static int xdebug_object_element_export_fancy(zval **zv TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level, debug_zval;
	xdebug_str *str;
	xdebug_var_export_options *options;
	char *prop_name, *class_name, *modifier, *prop_class_name;

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
		} else {
			xdebug_str_add(str, xdebug_sprintf("<i>public</i> %d <font color='%s'>=&gt;</font> ", hash_key->h, COLOR_POINTER), 1);
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
	int       is_temp;

	if (debug_zval) {
		xdebug_str_add(str, xdebug_sprintf("<i>(refcount=%d, is_ref=%d)</i>,", (*struc)->refcount__gc, (*struc)->is_ref__gc), 1);
	} else {
		if ((*struc)->is_ref__gc) {
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

						zend_hash_apply_with_arguments(myht TSRMLS_CC, (apply_func_args_t) xdebug_array_element_export_fancy, 4, level, str, debug_zval, options);
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
			myht = Z_OBJDEBUG_PP(struc, is_temp);
			xdebug_str_add(str, xdebug_sprintf("\n%*s", (level - 1) * 4, ""), 1);
			if (myht->nApplyCount < 1) {
				xdebug_str_add(str, xdebug_sprintf("<b>object</b>(<i>%s</i>)", Z_OBJCE_PP(struc)->name), 1);
				xdebug_str_add(str, xdebug_sprintf("[<i>%d</i>]\n", Z_OBJ_HANDLE_PP(struc)), 1);
				if (level <= options->max_depth) {
					options->runtime[level].current_element_nr = 0;
					options->runtime[level].start_element_nr = 0;
					options->runtime[level].end_element_nr = options->max_children;

					zend_hash_apply_with_arguments(myht TSRMLS_CC, (apply_func_args_t) xdebug_object_element_export_fancy, 5, level, str, debug_zval, options, Z_OBJCE_PP(struc)->name);
				} else {
					xdebug_str_add(str, xdebug_sprintf("%*s...\n", (level * 4) - 2, ""), 1);
				}
			} else {
				xdebug_str_add(str, xdebug_sprintf("<i>&</i><b>object</b>(<i>%s</i>)", Z_OBJCE_PP(struc)->name), 1);
				xdebug_str_add(str, xdebug_sprintf("[<i>%d</i>]\n", Z_OBJ_HANDLE_PP(struc)), 1);
			}
			if (is_temp) {
				zend_hash_destroy(myht);
				efree(myht);
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
	if (options->show_location) {
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
	php_serialize_data_t var_hash;
	smart_str buf = {0};

	if (!val) {
		return NULL;
	}

	PHP_VAR_SERIALIZE_INIT(var_hash);
	XG(in_var_serialisation) = 1;
	php_var_serialize(&buf, &val, &var_hash TSRMLS_CC);
	XG(in_var_serialisation) = 0;
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (buf.c) {
		int new_len;
		char *tmp_base64, *tmp_ret;

		/* now we need to base64 it */
		tmp_base64 = (char*) xdebug_base64_encode((unsigned char*) buf.c, buf.len, &new_len);

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

	if (debug_zval) {
		xdebug_str_add(str, xdebug_sprintf("<i>(refcount=%d, is_ref=%d)</i>,", (*struc)->refcount__gc, (*struc)->is_ref__gc), 1);
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

