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
#include "xdebug_var.h"

void xdebug_var_export(zval **struc, int level TSRMLS_DC);


/* {{{ xdebug_var_export */
static int xdebug_array_element_export(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	TSRMLS_FETCH();

	level = va_arg(args, int);

	if (hash_key->nKeyLength==0) { /* numeric key */
		php_printf("%ld => ", hash_key->h);
	} else { /* string key */
		php_printf("'%s' => ", hash_key->arKey);
	}
	xdebug_var_export(zv, level + 2 TSRMLS_CC);
	PUTS (", ");
	return 0;
}

static int xdebug_object_element_export(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	int level;
	TSRMLS_FETCH();

	level = va_arg(args, int);

	if (hash_key->nKeyLength != 0) {
		php_printf("var $%s = ", hash_key->arKey);
	}
	xdebug_var_export(zv, level + 2 TSRMLS_CC);
	PUTS ("; ");
	return 0;
}

void xdebug_var_export(zval **struc, int level TSRMLS_DC)
{
	HashTable *myht;
	char*     tmp_str;
	int       tmp_len;

	switch (Z_TYPE_PP(struc)) {
		case IS_BOOL:
			php_printf("%s", Z_LVAL_PP(struc) ? "TRUE" : "FALSE");
			break;

		case IS_NULL:
			php_printf("NULL");
			break;

		case IS_LONG:
			php_printf("%ld", Z_LVAL_PP(struc));
			break;

		case IS_DOUBLE:
			php_printf("%.*G", (int) EG(precision), Z_DVAL_PP(struc));
			break;

		case IS_STRING:
			tmp_str = php_addcslashes(Z_STRVAL_PP(struc), Z_STRLEN_PP(struc), &tmp_len, 0, "'\\", 2 TSRMLS_CC);
			PUTS ("'");
			PHPWRITE(tmp_str, tmp_len);
			PUTS ("'");
			efree (tmp_str);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_PP(struc);
			PUTS ("array (");
			zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_array_element_export, 1, level);
			PUTS(")");
			break;

		case IS_OBJECT:
			myht = Z_OBJPROP_PP(struc);
			php_printf ("class %s {", Z_OBJCE_PP(struc)->name);
			zend_hash_apply_with_arguments(myht, (apply_func_args_t) xdebug_object_element_export, 1, level);
			PUTS("}");
			break;

		case IS_RESOURCE: {
			char *type_name;

			type_name = zend_rsrc_list_get_rsrc_type(Z_LVAL_PP(struc) TSRMLS_CC);
			php_printf("resource(%ld) of type (%s)\n", Z_LVAL_PP(struc), type_name ? type_name : "Unknown");
			break;
		}

		default:
			PUTS ("NULL");
			break;
	}
}

/* }}} */
char* get_zval_value (zval *val)
{
	zval return_val;
	TSRMLS_FETCH();

	INIT_ZVAL(return_val);
	
	php_start_ob_buffer (NULL, 0, 1 TSRMLS_CC);
	xdebug_var_export (&val, 1 TSRMLS_CC);
	php_ob_get_buffer (&return_val TSRMLS_CC);
	php_end_ob_buffer (0, 0 TSRMLS_CC);

	return return_val.value.str.val;
}
