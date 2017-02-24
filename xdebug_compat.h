/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2016 Derick Rethans                               |
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

#ifndef __HAVE_XDEBUG_COMPAT_H__
#define __HAVE_XDEBUG_COMPAT_H__

#include "php.h"

#include "ext/standard/head.h"
#include "ext/standard/php_var.h"
#define xdebug_php_var_dump php_var_dump

zval *xdebug_zval_ptr(int op_type, const znode_op *node, zend_execute_data *zdata TSRMLS_DC);

#if PHP_VERSION_ID >= 70000
char *xdebug_str_to_str(char *haystack, size_t length, char *needle, size_t needle_len, char *str, size_t str_len, size_t *new_len);
char *xdebug_base64_encode(unsigned char *data, int data_len, int *new_len);
unsigned char *xdebug_base64_decode(unsigned char *data, int data_len, int *new_len);
void xdebug_stripcslashes(char *string, int *new_len);
zend_class_entry *xdebug_fetch_class(char *classname, int classname_len, int flags TSRMLS_DC);
int xdebug_get_constant(char *val, int len, zval *const_val TSRMLS_DC);
void xdebug_setcookie(char *name, int name_len, char *value, int value_len, time_t expires, char *path, int path_len, char *domain, int domain_len, int secure, int url_encode, int httponly TSRMLS_CC);
char *xdebug_get_compiled_variable_name(zend_op_array *op_array, uint32_t var, int *cv_len);
zval *xdebug_read_property(zend_class_entry *ce, zval *exception, char *name, int length, int flags TSRMLS_DC);

# define ADD_STRING_COPY
# define XDEBUG_ENFORCE_SAFE_MODE 0x00
# define SIZETorINT size_t
# define SIZETorUINT size_t
# define SIZETorZUINT size_t
# define zppLONG zend_long
# define iniLONG zend_long
# define hashULONG zend_ulong

# define XDEBUG_MAKE_STD_ZVAL(zv) \
	zv = ecalloc(sizeof(zval), 1);

# define XDEBUG_APPLY_COUNT(ht) ZEND_HASH_GET_APPLY_COUNT(ht)
# define HASH_KEY_VAL(k) (k)->key->val
# define HASH_KEY_LEN(k) (k)->key->len
# define HASH_KEY_SIZEOF(k) (sizeof(k) - 1)
# define HASH_KEY_STRLEN(k) (strlen(k))
# define HASH_KEY_IS_NUMERIC(k) ((k) == NULL)
# define HASH_APPLY_KEY_VAL(k) (k)->val
# define HASH_APPLY_KEY_LEN(k) (k)->len + 1

# define ZEND_USER_OPCODE_HANDLER_ARGS zend_execute_data *execute_data
# define ZEND_USER_OPCODE_HANDLER_ARGS_PASSTHRU execute_data

# define STR_NAME_VAL(k) (k)->val
# define STR_NAME_LEN(k) (k)->len

#else
# include "ext/standard/base64.h"
# define xdebug_str_to_str    php_str_to_str
# define xdebug_base64_encode php_base64_encode
# define xdebug_base64_decode php_base64_decode
# define xdebug_stripcslashes php_stripcslashes
# define xdebug_fetch_class   zend_fetch_class
# define xdebug_get_constant  zend_get_constant
# define xdebug_setcookie     php_setcookie
# define xdebug_get_compiled_variable_name zend_get_compiled_variable_name
# define xdebug_read_property zend_read_property

# define ADD_STRING_COPY , 1
# define XDEBUG_ENFORCE_SAFE_MODE ENFORCE_SAFE_MODE
# define SIZETorINT int
# define SIZETorUINT unsigned int
# define SIZETorZUINT zend_uint
# define zppLONG long
# define iniLONG long
# define hashULONG ulong

# define XDEBUG_MAKE_STD_ZVAL(zv) \
	MAKE_STD_ZVAL(zv)

# define XDEBUG_APPLY_COUNT(ht) (ht->nApplyCount)
# define HASH_KEY_VAL(k) (k)->arKey
# define HASH_KEY_LEN(k) (k)->nKeyLength
# define HASH_KEY_SIZEOF(k) (sizeof(k))
# define HASH_KEY_STRLEN(k) (strlen(k) + 1)
# define HASH_KEY_IS_NUMERIC(k) ((k)->nKeyLength == 0)
# define HASH_APPLY_KEY_VAL(k) (k)->arKey
# define HASH_APPLY_KEY_LEN(k) (k)->nKeyLength
# define HASH_APPLY_NUMERIC(k) (k)->h

# define ZEND_USER_OPCODE_HANDLER_ARGS ZEND_OPCODE_HANDLER_ARGS
# define ZEND_USER_OPCODE_HANDLER_ARGS_PASSTHRU ZEND_OPCODE_HANDLER_ARGS_PASSTHRU

# define STR_NAME_VAL(k) (k)
# define STR_NAME_LEN(k) (k ## _length)

#endif

#endif
