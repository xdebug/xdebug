/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 Derick Rethans                              |
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
#include "xdebug_var.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

#define XDEBUG_STR_PREALLOC 1024

inline void XDEBUG_STR_ADD(xdebug_str *xs, char *str, int f)
{
	int l = strlen(str);
	if (xs->l + l > xs->a - 1) {
		xs->d = xdrealloc(xs->d, xs->a + l + XDEBUG_STR_PREALLOC);
		xs->a = xs->a + l + XDEBUG_STR_PREALLOC;
	}
	if (!xs->l) {
		xs->d[0] = '\0';
	}
	memcpy(xs->d + xs->l, str, l);
	xs->d[xs->l + l] = '\0';
	xs->l = xs->l + l;
	if (f) {
		xdfree(str);
	}
}

inline void XDEBUG_STR_ADDL(xdebug_str *xs, char *str, int le, int f)
{
	if (xs->l + le > xs->a - 1) {
		xs->d = xdrealloc(xs->d, xs->a + le + XDEBUG_STR_PREALLOC);
		xs->a = xs->a + le + XDEBUG_STR_PREALLOC;
	}
	if (!xs->l) {
		xs->d[0] = '\0';
	}
	memcpy(xs->d + xs->l, str, le);
	xs->d[xs->l + le] = '\0';
	xs->l = xs->l + le;

	if (f) {
		xdfree(str);
	}
}

void XDEBUG_STR_CHOP(xdebug_str *xs, int c)
{
	if (c > xs->l) {
		/* Do nothing if the chop amount is larger than the buffer size */
	} else {
		xs->l -= c;
		xs->d[xs->l] = '\0';
	}
}

void XDEBUG_STR_FREE(xdebug_str *s)
{
	if (s->d) {
		xdfree(s->d);
	}
}

char *error_type(int type)
{
	switch (type) {
		case E_ERROR:
		case E_CORE_ERROR:
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			return xdstrdup("Fatal error");
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
		default:
			return xdstrdup("Unknown error");
			break;
	}
}


char *xdebug_sprintf(const char* fmt, ...)
{
	char   *new_str;
	int     size = 1;
	va_list args;

	new_str = (char *) xdmalloc(size);

	va_start(args, fmt);
	for (;;) {
		int n = vsnprintf(new_str, size, fmt, args);
		if (n > -1 && n < size) {
			break;
		}
		if (n < 0) {
			size *= 2;
		} else {
			size = n + 1;
		}
		new_str = (char *) xdrealloc(new_str, size);
	}
	va_end(args);

	return new_str;
}

/*****************************************************************************
* ** Normal variable printing routines
* */

static int xdebug_array_element_export(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_str *str;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	str   = va_arg(args, struct xdebug_str*);

	if (hash_key->nKeyLength==0) { /* numeric key */
		XDEBUG_STR_ADD(str, xdebug_sprintf("%ld => ", hash_key->h), 1);
	} else { /* string key */
		XDEBUG_STR_ADD(str, xdebug_sprintf("'%s' => ", hash_key->arKey), 1);
	}
	xdebug_var_export(zv, str, level + 2 TSRMLS_CC);
	XDEBUG_STR_ADDL(str, ", ", 2, 0);
	return 0;
}

static int xdebug_object_element_export(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_str *str;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	str   = va_arg(args, struct xdebug_str*);

	if (hash_key->nKeyLength != 0) {
		XDEBUG_STR_ADD(str, xdebug_sprintf("var $%s = ", hash_key->arKey), 1);
	}
	xdebug_var_export(zv, str, level + 2 TSRMLS_CC);
	XDEBUG_STR_ADDL(str, "; ", 2, 0);
	return 0;
}

void xdebug_var_export(zval **struc, xdebug_str *str, int level TSRMLS_DC)
{
	HashTable *myht;
	char*     tmp_str;
	int       tmp_len;

	switch (Z_TYPE_PP(struc)) {
		case IS_BOOL:
			XDEBUG_STR_ADD(str, xdebug_sprintf("%s", Z_LVAL_PP(struc) ? "TRUE" : "FALSE"), 1);
			break;

		case IS_NULL:
			XDEBUG_STR_ADDL(str, "NULL", 4, 0);
			break;

		case IS_LONG:
			XDEBUG_STR_ADD(str, xdebug_sprintf("%ld", Z_LVAL_PP(struc)), 1);
			break;

		case IS_DOUBLE:
			XDEBUG_STR_ADD(str, xdebug_sprintf("%.*G", (int) EG(precision), Z_DVAL_PP(struc)), 1);
			break;

		case IS_STRING:
			tmp_str = php_addcslashes(Z_STRVAL_PP(struc), Z_STRLEN_PP(struc), &tmp_len, 0, "'\\", 2 TSRMLS_CC);
			XDEBUG_STR_ADD(str, xdebug_sprintf("'%s'", tmp_str), 1);
			efree(tmp_str);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			if (myht->nApplyCount < 1) {
				XDEBUG_STR_ADDL(str, "array (", 7, 0);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_array_element_export, 2, level, str);
				if (myht->nNumOfElements > 0) {
					XDEBUG_STR_CHOP(str, 2);
				}
				XDEBUG_STR_ADDL(str, ")", 1, 0);
			} else {
				XDEBUG_STR_ADDL(str, "...", 3, 0);
			}
			break;

		case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);
			if (myht->nApplyCount < 1) {
				XDEBUG_STR_ADD(str, xdebug_sprintf("class %s {", Z_OBJCE_PP(struc)->name), 1);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_object_element_export, 2, level, str);
				if (myht->nNumOfElements > 0) {
					XDEBUG_STR_CHOP(str, 2);
				}
				XDEBUG_STR_ADDL(str, "}", 1, 0);
			} else {
				XDEBUG_STR_ADDL(str, "...", 3, 0);
			}
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			XDEBUG_STR_ADD(str, xdebug_sprintf("resource(%ld) of type (%s)", Z_LVAL_PP(struc), type_name ? type_name : "Unknown"), 1);
			break;
		}

		default:
			XDEBUG_STR_ADDL(str, "NULL", 4, 0);
			break;
	}
}

char* get_zval_value(zval *val)
{
	xdebug_str str = {0, 0, NULL};
	TSRMLS_FETCH();

	xdebug_var_export(&val, (xdebug_str*) &str, 1 TSRMLS_CC);

	return str.d;
}

/*****************************************************************************
* ** XML variable printing routines
* */

static int xdebug_array_element_export_xml(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_str *str;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	str   = va_arg(args, struct xdebug_str*);

	XDEBUG_STR_ADDL(str, "<var", 4, 0);
	if (hash_key->nKeyLength == 0) { /* numeric key */
		XDEBUG_STR_ADD(str, xdebug_sprintf(" name='%ld'", hash_key->h), 1);
	} else { /* string key */
		XDEBUG_STR_ADD(str, xdebug_sprintf(" name='%s'", hash_key->arKey), 1);
	}
	XDEBUG_STR_ADD(str, xdebug_sprintf(" id='%p'>", *zv), 1);
	xdebug_var_export_xml(zv, str, level + 2 TSRMLS_CC);
	XDEBUG_STR_ADDL(str, "</var>", 6, 0);
	return 0;
}

static int xdebug_object_element_export_xml(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_str *str;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	str   = va_arg(args, struct xdebug_str*);

	XDEBUG_STR_ADDL(str, "<var", 4, 0);
	if (hash_key->nKeyLength != 0) {
		XDEBUG_STR_ADD(str, xdebug_sprintf(" name='%s'", hash_key->arKey), 1);
	}
	XDEBUG_STR_ADD(str, xdebug_sprintf(" id='%p'>", *zv), 1);
	xdebug_var_export_xml(zv, str, level + 2 TSRMLS_CC);
	XDEBUG_STR_ADDL(str, "</var>", 6, 0);
	return 0;
}

void xdebug_var_export_xml(zval **struc, xdebug_str *str, int level TSRMLS_DC)
{
	HashTable *myht;
	char*     tmp_str;

	switch (Z_TYPE_PP(struc)) {
		case IS_BOOL:
			XDEBUG_STR_ADD(str, xdebug_sprintf("<bool>%s</bool>", Z_LVAL_PP(struc) ? "1" : "0"), 1);
			break;

		case IS_NULL:
			XDEBUG_STR_ADDL(str, "<null/>", 7, 0);
			break;

		case IS_LONG:
			XDEBUG_STR_ADD(str, xdebug_sprintf("<int>%ld</int>", Z_LVAL_PP(struc)), 1);
			break;

		case IS_DOUBLE:
			XDEBUG_STR_ADD(str, xdebug_sprintf("<float>%.*G</float>", (int) EG(precision), Z_DVAL_PP(struc)), 1);
			break;

		case IS_STRING:
			tmp_str = xmlize(Z_STRVAL_PP(struc));
			XDEBUG_STR_ADD(str, xdebug_sprintf("<string>%s</string>", tmp_str), 1);
			efree(tmp_str);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			if (myht->nApplyCount < 1) {
				XDEBUG_STR_ADDL(str, "<array>", 7, 0);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_array_element_export_xml, 2, level, str);
				XDEBUG_STR_ADDL(str, "</array>", 8, 0);
			} else {
				XDEBUG_STR_ADDL(str, "<array hidden='true' recursive='true'/>", 39, 0);
			}
			break;

		case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);
			if (myht->nApplyCount < 1) {
				XDEBUG_STR_ADD(str, xdebug_sprintf("<object class='%s'>", Z_OBJCE_PP(struc)->name), 1);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_object_element_export_xml, 2, level, str);
				XDEBUG_STR_ADDL(str, "</object>", 9, 0);
			} else {
				XDEBUG_STR_ADDL(str, "<object hidden='true' recursive='true'/>", 40, 0);
			}
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			XDEBUG_STR_ADD(str, xdebug_sprintf("<resource id='%ld' type='%s'/>", Z_LVAL_PP(struc), type_name ? type_name : "Unknown"), 1);
			break;
		}

		default:
			XDEBUG_STR_ADDL(str, "<null/>", 7, 0);
			break;
	}
}

char* get_zval_value_xml(char *name, zval *val)
{
	xdebug_str str = {0, 0, NULL};
	TSRMLS_FETCH();

	if (name) {
		XDEBUG_STR_ADDL(&str, "<var name='", 11, 0);
		XDEBUG_STR_ADD(&str, name, 0);
		XDEBUG_STR_ADD(&str, xdebug_sprintf("' id='%p'>", val), 1);
	} else {
		XDEBUG_STR_ADD(&str, xdebug_sprintf("<var id='%p'>", val), 1);
	}
	
	xdebug_var_export_xml(&val, (xdebug_str*) &str, 1 TSRMLS_CC);

	XDEBUG_STR_ADDL(&str, "</var>", 7, 0);

	return str.d;
}

/*****************************************************************************
* ** Fancy variable printing routines
* */

#define BLUE       "#0000ff"
#define RED        "#ff0000"
#define GREEN      "#00bb00"
#define BLUE_GREEN "#00bbbb"
#define PURPLE     "#bb00bb"
#define DGREY      "#777777"

static int xdebug_array_element_export_fancy(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_str *str;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	str   = va_arg(args, struct xdebug_str*);

	XDEBUG_STR_ADD(str, xdebug_sprintf("%*s", level * 2, ""), 1);

	if (hash_key->nKeyLength==0) { /* numeric key */
		XDEBUG_STR_ADD(str, xdebug_sprintf("%ld <font color='%s'>=&gt;</font> ", hash_key->h, DGREY), 1);
	} else { /* string key */
		XDEBUG_STR_ADD(str, xdebug_sprintf("'%s' <font color='%s'>=&gt;</font> ", hash_key->arKey, DGREY), 1);
	}
	xdebug_var_export_fancy(zv, str, level + 2 TSRMLS_CC);

	return 0;
}

static int xdebug_object_element_export_fancy(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	xdebug_str *str;
	TSRMLS_FETCH();

	level = va_arg(args, int);
	str   = va_arg(args, struct xdebug_str*);

	XDEBUG_STR_ADD(str, xdebug_sprintf("%*s", level * 2, ""), 1);

	if (hash_key->nKeyLength != 0) {
		XDEBUG_STR_ADD(str, xdebug_sprintf("'%s' <font color='%s'>=&gt;</font> ", hash_key->arKey, DGREY), 1);
	}
	xdebug_var_export_fancy(zv, str, level + 2 TSRMLS_CC);
	return 0;
}

void xdebug_var_export_fancy(zval **struc, xdebug_str *str, int level TSRMLS_DC)
{
	HashTable *myht;
	char*     tmp_str;

	switch (Z_TYPE_PP(struc)) {
		case IS_BOOL:
			XDEBUG_STR_ADD(str, xdebug_sprintf("<font color='%s'>%s</font>", BLUE, Z_LVAL_PP(struc) ? "true" : "false"), 1);
			break;

		case IS_NULL:
			XDEBUG_STR_ADD(str, xdebug_sprintf("<font color='%s'>null</font>", RED), 1);
			break;

		case IS_LONG:
			XDEBUG_STR_ADD(str, xdebug_sprintf("<font color='%s'>%ld</font>", GREEN, Z_LVAL_PP(struc)), 1);
			break;

		case IS_DOUBLE:
			XDEBUG_STR_ADD(str, xdebug_sprintf("<font color='%s'>%.*G</font>", BLUE_GREEN, (int) EG(precision), Z_DVAL_PP(struc)), 1);
			break;

		case IS_STRING:
			tmp_str = xmlize(Z_STRVAL_PP(struc));
			XDEBUG_STR_ADD(str, xdebug_sprintf("<font color='%s'>'%s'</font>", PURPLE, tmp_str), 1);
			efree(tmp_str);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			XDEBUG_STR_ADD(str, xdebug_sprintf("\n%*s", (level - 1) * 2, ""), 1);
			XDEBUG_STR_ADDL(str, "<b>array</b>\n", 13, 0);
			if (myht->nApplyCount < 2) {
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_array_element_export_fancy, 2, level, str);
			}
			break;

		case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);
			XDEBUG_STR_ADD(str, xdebug_sprintf("\n%*s", (level - 1) * 2, ""), 1);
			if (myht->nApplyCount < 2) {
				XDEBUG_STR_ADD(str, xdebug_sprintf("<b>object</b>(<i>%s</i>)\n", Z_OBJCE_PP(struc)->name), 1);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_object_element_export_fancy, 2, level, str);
			} else {
				XDEBUG_STR_ADDL(str, "<b>object</b> {\n", 16, 0);
			}
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			XDEBUG_STR_ADD(str, xdebug_sprintf("<b>resource</b>(<i>%ld</i><font color='%s'>,</font> <i>%s</i>)", Z_LVAL_PP(struc), DGREY, type_name ? type_name : "Unknown"), 1);
			break;
		}

		default:
			XDEBUG_STR_ADD(str, xdebug_sprintf("<font color='%s'>null</font>", RED), 0);
			break;
	}
	if (Z_TYPE_PP(struc) != IS_ARRAY && Z_TYPE_PP(struc) != IS_OBJECT) {
		XDEBUG_STR_ADDL(str, "\n", 1, 0);
	}
}

char* get_zval_value_fancy(char *name, zval *val TSRMLS_DC)
{
	xdebug_str str = {0, 0, NULL};

	XDEBUG_STR_ADDL(&str, "<pre>", 5, 0);
	xdebug_var_export_fancy(&val, (xdebug_str*) &str, 1 TSRMLS_CC);
	XDEBUG_STR_ADDL(&str, "</pre>", 6, 0);

	return str.d;
}

/*****************************************************************************
* ** XML encoding function
* */

char* xmlize(char *string)
{
	int   len = strlen(string);
	char *tmp;
	char *tmp2;

	if (strlen(string)) {
		tmp = php_str_to_str(tmp2, len, "&", 1, "&amp;", 5, &len);

		tmp2 = php_str_to_str(tmp, len, ">", 1, "&gt;", 4, &len);
		efree(tmp);

		tmp = php_str_to_str(string, len, "<", 1, "&lt;", 4, &len);
		efree(tmp2);

		tmp2 = php_str_to_str(tmp, len, "\n", 1, "&#10;", 5, &len);
		efree(tmp);
		return tmp2;
	} else {
		return estrdup(string);
	}
}

/*****************************************************************************
* ** Function name printing function
* */

char* show_fname(struct function_stack_entry* entry, int html TSRMLS_DC)
{
	char *tmp;
	xdebug_func f;

	f = entry->function;

	switch (f.type) {
		case XFUNC_NORMAL: {
			zend_function *zfunc;

			if (PG(html_errors) && zend_hash_find(EG(function_table), f.function, strlen(f.function) + 1, (void**) &zfunc) == SUCCESS) {
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
