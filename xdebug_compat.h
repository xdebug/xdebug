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

#ifndef __HAVE_XDEBUG_COMPAT_H__
#define __HAVE_XDEBUG_COMPAT_H__


#if (PHP_MAJOR_VERSION == 4) && (PHP_MINOR_VERSION == 3) && (PHP_RELEASE_VERSION <= 1)

void xdebug_var_dump(zval **struc, int level TSRMLS_DC);

#else

#	include "ext/standard/php_var.h"
#	define xdebug_var_dump php_var_dump

#endif


#if (PHP_MAJOR_VERSION == 4) && (PHP_MINOR_VERSION == 3) && (PHP_RELEASE_VERSION <= 4)

unsigned char *xdebug_base64_encode(const unsigned char *, int, int *);
unsigned char *xdebug_base64_decode(const unsigned char *str, int length, int *ret_length);

#else

#	include "ext/standard/base64.h"
#	define xdebug_base64_encode php_base64_encode
#	define xdebug_base64_decode php_base64_decode

#endif

#endif
