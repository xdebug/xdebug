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

#ifndef __HAVE_XDEBUG_STR_H__
#define __HAVE_XDEBUG_STR_H__

#include "xdebug_mm.h"

#define XDEBUG_STR_INITIALIZER { 0, 0, NULL }
#define XDEBUG_STR_PREALLOC 1024
#define xdebug_str_ptr_init(str) str = xdmalloc(sizeof(xdebug_str)); str->l = 0; str->a = 0; str->d = NULL;
#define xdebug_str_ptr_dtor(str) xdfree(str->d); xdfree(str)
#define xdebug_str_dtor(str)     xdfree(str.d)

typedef struct xdebug_str {
	signed long l;
	signed long a;
	char *d;
} xdebug_str;

void xdebug_str_add(xdebug_str *xs, char *str, int f);
void xdebug_str_addl(xdebug_str *xs, char *str, int le, int f);
void xdebug_str_chop(xdebug_str *xs, int c);
void xdebug_str_free(xdebug_str *s);

char* xdebug_sprintf (const char* fmt, ...);
char* xdebug_strndup(const char *s, int length);

#endif
