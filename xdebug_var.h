/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 The PHP Group                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <d.rethans@jdimedia.nl>                     |
   +----------------------------------------------------------------------+
 */

#include "zend.h"
#include "php_xdebug.h"

#ifndef __HAVE_XDEBUG_VAR_H__
#define __HAVE_XDEBUG_VAR_H__

typedef struct xdebug_str {
	int   l;
	int   a;
	char *d;
} xdebug_str;

void XDEBUG_STR_ADD(xdebug_str *xs, char *str, int f);
void XDEBUG_STR_ADDL(xdebug_str *xs, char *str, int le, int f);
void XDEBUG_STR_CHOP(xdebug_str *xs, int c);
void XDEBUG_STR_FREE(xdebug_str *s);

void xdebug_var_export(zval **struc, xdebug_str *str, int level TSRMLS_DC);
char* error_type (int type);
char* xdebug_sprintf (const char* fmt, ...);
char* get_zval_value (zval *val);
char* show_fname (struct function_stack_entry* entry, int html TSRMLS_DC);

#endif
