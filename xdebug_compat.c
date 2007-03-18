/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2004 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Jim Winstead <jimw@php.net>                                  |
   | Modifications: Derick Rethans <derick@xdebug.org>                    |
   +----------------------------------------------------------------------+
 */
/* $Id: xdebug_compat.c,v 1.9 2007-03-18 09:04:18 derick Exp $ */

#include "php.h"
#include "main/php_version.h"
#include "xdebug_compat.h"
#include "zend_extensions.h"

#if PHP_MAJOR_VERSION >= 6

void xdebug_php_var_dump(zval **struc, int level TSRMLS_DC)
{
	php_var_dump(struc, 1, 1 TSRMLS_CC);
}

#elif (PHP_MAJOR_VERSION == 4) && (PHP_MINOR_VERSION == 3) && (PHP_RELEASE_VERSION <= 1)

#define COMMON ((*struc)->is_ref ? "&" : "")
/* {{{ xdebug_var_dump */

static int php_array_element_dump(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	TSRMLS_FETCH();

	level = va_arg(args, int);

	if (hash_key->nKeyLength==0) { /* numeric key */
		php_printf("%*c[%ld]=>\n", level + 1, ' ', hash_key->h);
	} else { /* string key */
		php_printf("%*c[\"", level + 1, ' ');
		PHPWRITE(hash_key->arKey, hash_key->nKeyLength - 1);
		php_printf("\"]=>\n");
	}
	xdebug_php_var_dump(zv, level + 2 TSRMLS_CC);
	return 0;
}

void xdebug_php_var_dump(zval **struc, int level TSRMLS_DC)
{
	HashTable *myht = NULL;
	zend_object *object = NULL;

	if (level > 1) {
		php_printf("%*c", level - 1, ' ');
	}

	switch (Z_TYPE_PP(struc)) {
	case IS_BOOL:
		php_printf("%sbool(%s)\n", COMMON, Z_LVAL_PP(struc)?"true":"false");
		break;
	case IS_NULL:
		php_printf("%sNULL\n", COMMON);
		break;
	case IS_LONG:
		php_printf("%sint(%ld)\n", COMMON, Z_LVAL_PP(struc));
		break;
	case IS_DOUBLE:
		php_printf("%sfloat(%.*G)\n", COMMON, (int) EG(precision), Z_DVAL_PP(struc));
		break;
	case IS_STRING:
		php_printf("%sstring(%d) \"", COMMON, Z_STRLEN_PP(struc));
		PHPWRITE(Z_STRVAL_PP(struc), Z_STRLEN_PP(struc));
		PUTS("\"\n");
		break;
	case IS_ARRAY:
		myht = Z_ARRVAL_PP(struc);
		if (myht->nApplyCount > 1) {
			PUTS("*RECURSION*\n");
			return;
		}
		php_printf("%sarray(%d) {\n", COMMON, zend_hash_num_elements(myht));
		goto head_done;
	case IS_OBJECT:
		object = Z_OBJ_PP(struc);
		myht = Z_OBJPROP_PP(struc);
		if (myht->nApplyCount > 1) {
			PUTS("*RECURSION*\n");
			return;
		}
		php_printf("%sobject(%s)(%d) {\n", COMMON, Z_OBJCE_PP(struc)->name, zend_hash_num_elements(myht));
head_done:
		zend_hash_apply_with_arguments(myht, (apply_func_args_t) php_array_element_dump, 1, level);
		if (level > 1) {
			php_printf("%*c", level-1, ' ');
		}
		PUTS("}\n");
		break;
	case IS_RESOURCE: {
		char *type_name;

		type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
		php_printf("%sresource(%ld) of type (%s)\n", COMMON, Z_LVAL_PP(struc), type_name ? type_name : "Unknown");
		break;
	}
	default:
		php_printf("%sUNKNOWN:0\n", COMMON);
		break;
	}
}
/* }}} */
#endif


#if (PHP_MAJOR_VERSION == 4) && (PHP_MINOR_VERSION == 3) && (PHP_RELEASE_VERSION <= 4)

#include <string.h>

/* {{{ */
static const char base64_table[] =
	{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
	};

static const char base64_pad = '=';

static const short base64_reverse_table[256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
/* }}} */

/* {{{ */
unsigned char *xdebug_base64_encode(const unsigned char *str, int length, int *ret_length)
{
	const unsigned char *current = str;
	int i = 0;
	unsigned char *result = (unsigned char *)emalloc(((length + 3 - length % 3) * 4 / 3 + 1) * sizeof(char));

	while (length > 2) { /* keep going until we have less than 24 bits */
		result[i++] = base64_table[current[0] >> 2];
		result[i++] = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
		result[i++] = base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
		result[i++] = base64_table[current[2] & 0x3f];

		current += 3;
		length -= 3; /* we just handle 3 octets of data */
	}

	/* now deal with the tail end of things */
	if (length != 0) {
		result[i++] = base64_table[current[0] >> 2];
		if (length > 1) {
			result[i++] = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
			result[i++] = base64_table[(current[1] & 0x0f) << 2];
			result[i++] = base64_pad;
		}
		else {
			result[i++] = base64_table[(current[0] & 0x03) << 4];
			result[i++] = base64_pad;
			result[i++] = base64_pad;
		}
	}
	if(ret_length) {
		*ret_length = i;
	}
	result[i] = '\0';
	return result;
}
/* }}} */

/* {{{ */
/* as above, but backwards. :) */
unsigned char *xdebug_base64_decode(const unsigned char *str, int length, int *ret_length)
{
	const unsigned char *current = str;
	int ch, i = 0, j = 0, k;
	/* this sucks for threaded environments */
	unsigned char *result;
	
	result = (unsigned char *)emalloc(length + 1);
	if (result == NULL) {
		return NULL;
	}

	/* run through the whole string, converting as we go */
	while ((ch = *current++) != '\0' && length-- > 0) {
		if (ch == base64_pad) break;

	    /* When Base64 gets POSTed, all pluses are interpreted as spaces.
		   This line changes them back.  It's not exactly the Base64 spec,
		   but it is completely compatible with it (the spec says that
		   spaces are invalid).  This will also save many people considerable
		   headache.  - Turadg Aleahmad <turadg@wise.berkeley.edu>
	    */

		if (ch == ' ') ch = '+'; 

		ch = base64_reverse_table[ch];
		if (ch < 0) continue;

		switch(i % 4) {
		case 0:
			result[j] = ch << 2;
			break;
		case 1:
			result[j++] |= ch >> 4;
			result[j] = (ch & 0x0f) << 4;
			break;
		case 2:
			result[j++] |= ch >>2;
			result[j] = (ch & 0x03) << 6;
			break;
		case 3:
			result[j++] |= ch;
			break;
		}
		i++;
	}

	k = j;
	/* mop things up if we ended on a boundary */
	if (ch == base64_pad) {
		switch(i % 4) {
		case 0:
		case 1:
			efree(result);
			return NULL;
		case 2:
			k++;
		case 3:
			result[k++] = 0;
		}
	}
	if(ret_length) {
		*ret_length = j;
	}
	result[j] = '\0';
	return result;
}
/* }}} */

#endif

/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2004 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   | Modifications: Derick Rethans <derick@xdebug.org>                    |
   +----------------------------------------------------------------------+
*/

#ifdef ZEND_ENGINE_2

#define T(offset) (*(temp_variable *)((char *) Ts + offset))

zval *xdebug_zval_ptr(znode *node, temp_variable *Ts TSRMLS_DC)
{
	switch (node->op_type) {
		case IS_CONST:
			return &node->u.constant;
			break;
		case IS_TMP_VAR:
			return &T(node->u.var).tmp_var;
			break;
		case IS_VAR:
			if (T(node->u.var).var.ptr) {
				return T(node->u.var).var.ptr;
			} else {
				temp_variable *T = &T(node->u.var);
				zval *str = T->str_offset.str;

				if (T->str_offset.str->type != IS_STRING
					|| ((int)T->str_offset.offset<0)
					|| (T->str_offset.str->value.str.len <= T->str_offset.offset)) {
					zend_error(E_NOTICE, "Uninitialized string offset:  %d", T->str_offset.offset);
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1) || PHP_MAJOR_VERSION == 6
					T->tmp_var.value.str.val = STR_EMPTY_ALLOC();
#else
					T->tmp_var.value.str.val = empty_string;
#endif
					T->tmp_var.value.str.len = 0;
				} else {
					char c = str->value.str.val[T->str_offset.offset];

					T->tmp_var.value.str.val = estrndup(&c, 1);
					T->tmp_var.value.str.len = 1;
				}
				T->tmp_var.refcount=1;
				T->tmp_var.is_ref=1;
				T->tmp_var.type = IS_STRING;
				return &T->tmp_var;
			}
			break;
		case IS_UNUSED:
			return NULL;
			break;
	}
	return NULL;
}

#else

static zval get_overloaded_property(temp_variable *T TSRMLS_DC)
{
	zval result;

	result = Z_OBJCE_P(T->EA.data.overloaded_element.object)->handle_property_get(&T->EA.data.overloaded_element);

	zend_llist_destroy(T->EA.data.overloaded_element.elements_list);
	efree(T->EA.data.overloaded_element.elements_list);
	return result;
}

zval *xdebug_zval_ptr(znode *node, temp_variable *Ts TSRMLS_DC)
{
	switch(node->op_type) {
		case IS_CONST:
			return &node->u.constant;
			break;
		case IS_TMP_VAR:
			return &Ts[node->u.var].tmp_var;
			break;
		case IS_VAR:
			if (Ts[node->u.var].var.ptr) {
				return Ts[node->u.var].var.ptr;
			} else {

				switch (Ts[node->u.var].EA.type) {
					case IS_OVERLOADED_OBJECT:
						Ts[node->u.var].tmp_var = get_overloaded_property(&Ts[node->u.var] TSRMLS_CC);
						Ts[node->u.var].tmp_var.refcount=1;
						Ts[node->u.var].tmp_var.is_ref=1;
						return &Ts[node->u.var].tmp_var;
						break;
					case IS_STRING_OFFSET: {
							temp_variable *T = &Ts[node->u.var];
							zval *str = T->EA.data.str_offset.str;

							if (T->EA.data.str_offset.str->type != IS_STRING
								|| (T->EA.data.str_offset.offset<0)
								|| (T->EA.data.str_offset.str->value.str.len <= T->EA.data.str_offset.offset)) {
								zend_error(E_NOTICE, "Uninitialized string offset:  %d", T->EA.data.str_offset.offset);
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1
								T->tmp_var.value.str.val = STR_EMPTY_ALLOC();
#else
								T->tmp_var.value.str.val = empty_string;
#endif
								T->tmp_var.value.str.len = 0;
							} else {
								char c = str->value.str.val[T->EA.data.str_offset.offset];

								T->tmp_var.value.str.val = estrndup(&c, 1);
								T->tmp_var.value.str.len = 1;
							}
							T->tmp_var.refcount=1;
							T->tmp_var.is_ref=1;
							T->tmp_var.type = IS_STRING;
							return &T->tmp_var;
						}
						break;
				}
			}
			break;
		case IS_UNUSED:
			return NULL;
			break;
	}
	return NULL;
}

#endif
