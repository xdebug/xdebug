/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2010 Derick Rethans                               |
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

#if PHP_MAJOR_VERSION >= 6
void xdebug_php_var_dump(zval **struc, int level TSRMLS_DC);
#else
# include "ext/standard/php_var.h"
# define xdebug_php_var_dump php_var_dump
#endif

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 1
# define zend_memrchr php_zend_memrchr
void *php_zend_memrchr(const void *s, int c, size_t n);
#endif


#include "ext/standard/base64.h"
#define xdebug_base64_encode php_base64_encode
#define xdebug_base64_decode php_base64_decode

zval *xdebug_zval_ptr(znode *node, temp_variable *Ts TSRMLS_DC);

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || (PHP_MAJOR_VERSION >= 6)
#	define XDEBUG_REFCOUNT refcount__gc
#	define XDEBUG_IS_REF is_ref__gc
#else
#	define XDEBUG_REFCOUNT refcount
#	define XDEBUG_IS_REF is_ref
#endif

#if defined(PHP_VERSION_ID) && PHP_VERSION_ID >= 50300 && ZTS
#	define XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC TSRMLS_DC
#	define XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC TSRMLS_CC
#else
#	define XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC
#	define XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC
#endif

#endif
