/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000, 2001 The PHP Group             |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@vl-srm.net>                         |
   +----------------------------------------------------------------------+
 */

#include "php.h"
#include "ext/standard/php_string.h"
#include "zend.h"
#include "zend_extensions.h"
#include "xdebug_var.h"


void XDEBUG_STR_ADD(xdebug_str *xs, char *str, int free) { 
	int l = strlen(str);                
	if (xs->l + l > xs->a) {            
		xs->d = erealloc (xs->d, xs->a + l + 128); 
	   	xs->a = xs->a + l + 128;        
	}                                   
	if (!xs->l) {                       
		xs->d[0] = '\0';                
	}                                   
	strcat (xs->d, str);                
	xs->l = xs->l + l;                  
	if (free) {                         
		efree(str);                     
	}                                   
}

void XDEBUG_STR_ADDL(xdebug_str *xs, char *str, int le, int free) { 
	if (xs->l + le > xs->a) {                
		xs->d = erealloc (xs->d, xs->a + le + 128); 
	   	xs->a = xs->a + le + 128;            
	}                                        
	if (!xs->l) {                            
		xs->d[0] = '\0';                     
	}                                        
	strcat (xs->d, str);                     
	xs->l = xs->l + le;                      
	if (free) {                              
		efree(str);                          
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

char *xdebug_sprintf (const char* fmt, ...)
{
	char   *new_str;
	int     size = 1;
	va_list args;

	new_str = (char *) emalloc (size);

	va_start(args, fmt);
	for (;;) {
		int n = vsnprintf (new_str, size, fmt, args);
		if (n > -1 && n < size) {
			break;
		}
		if (n < 0) {
			size *= 2;
		} else {
			size = n + 1;
		}
		new_str = (char *) erealloc (new_str, size);
	}
	va_end (args);

	return new_str;
}


/* {{{ xdebug_var_export */
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
			efree (tmp_str);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			if (myht->nApplyCount < 2) {
				XDEBUG_STR_ADDL(str, "array (", 7, 0);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_array_element_export, 2, level, str);
				XDEBUG_STR_CHOP(str, 2);
				XDEBUG_STR_ADDL(str, ")", 1, 0);
			} else {
				XDEBUG_STR_ADDL(str, "...", 3, 0);
			}
			break;

		case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);
			if (myht->nApplyCount < 2) {
				XDEBUG_STR_ADD(str, xdebug_sprintf ("class %s {", Z_OBJCE_PP(struc)->name), 1);
				zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_object_element_export, 2, level, str);
				XDEBUG_STR_CHOP(str, 2);
				XDEBUG_STR_ADDL(str, "}", 1, 0);
			} else {
				XDEBUG_STR_ADDL(str, "...", 3, 0);
			}
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			XDEBUG_STR_ADD(str, xdebug_sprintf("resource(%ld) of type (%s)\n", Z_LVAL_PP(struc), type_name ? type_name : "Unknown"), 1);
			break;
		}

		default:
			XDEBUG_STR_ADDL(str, "NULL", 4, 0);
			break;
	}
}

/* }}} */
char* get_zval_value (zval *val)
{
	xdebug_str str = {0, 0, NULL};
	TSRMLS_FETCH();

	xdebug_var_export (&val, (xdebug_str*) &str, 1 TSRMLS_CC);

	return str.d;
}
