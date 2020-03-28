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
   +----------------------------------------------------------------------+
 */

#include "php_xdebug.h"

#include "ext/standard/php_smart_string.h"
#include "Zend/zend_smart_str.h"

#include "str.h"
#include "var_export_serialized.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

xdebug_str* xdebug_get_zval_value_serialized(zval *val, int debug_zval, xdebug_var_export_options *options)
{
	zend_object *orig_exception = EG(exception);
	php_serialize_data_t var_hash;
	smart_str buf = { 0, 0 };

	if (!val) {
		return NULL;
	}

	PHP_VAR_SERIALIZE_INIT(var_hash);
	XG_BASE(in_var_serialisation) = 1;
	EG(exception) = NULL;
	php_var_serialize(&buf, val, &var_hash);
	orig_exception = EG(exception) = orig_exception;
	XG_BASE(in_var_serialisation) = 0;
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (buf.a) {
		unsigned char *tmp_base64;
		size_t         new_len;
		xdebug_str    *tmp_ret;

		/* now we need to base64 it */
		tmp_base64 = xdebug_base64_encode((unsigned char*) buf.s->val, buf.s->len, &new_len);

		/* we need a malloc'ed and not an emalloc'ed string */
		tmp_ret = xdebug_str_create((char*) tmp_base64, new_len);

		xdfree(tmp_base64);
		smart_str_free(&buf);

		return tmp_ret;
	} else {
		return NULL;
	}
}
