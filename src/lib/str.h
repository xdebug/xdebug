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

#ifndef __HAVE_XDEBUG_STR_H__
#define __HAVE_XDEBUG_STR_H__

#include <stddef.h>

#include "php.h"

#include "mm.h"

#define XDEBUG_STR_INITIALIZER { 0, 0, NULL }
#define XDEBUG_STR_PREALLOC 1024
#define xdebug_str_dtor(str)     xdfree(str.d)

#define XDEBUG_STR_WRAP_CHAR(v) (&((xdebug_str){strlen(v), strlen(v)+1, ((char*)(v))}))

typedef struct xdebug_str {
	size_t l;
	size_t a;
	char *d;
} xdebug_str;

void xdebug_str_add(xdebug_str *xs, const char *str, int f);
void xdebug_str_addl(xdebug_str *xs, const char *str, int le, int f);
void xdebug_str_add_str(xdebug_str *xs, const xdebug_str *str);
void xdebug_str_add_zstr(xdebug_str *xs, const zend_string *str);
void xdebug_str_addc(xdebug_str *xs, char letter);
void xdebug_str_add_uint64(xdebug_str *xs, uint64_t num);
void xdebug_str_add_fmt(xdebug_str *xs, const char *fmt, ...);
#define xdebug_str_add_literal(s,l) xdebug_str_addl((s), (l), sizeof(l)-1, 0)
#define xdebug_str_add_const(s,l) xdebug_str_addl((s), (l), strlen(l), 0)

void xdebug_str_chop(xdebug_str *xs, size_t c);

xdebug_str *xdebug_str_new(void);
xdebug_str *xdebug_str_create_from_char(char *c);
xdebug_str *xdebug_str_create(char *c, size_t len);
xdebug_str *xdebug_str_copy(xdebug_str *orig);
void xdebug_str_destroy(xdebug_str *s);
void xdebug_str_free(xdebug_str *s);

char* xdebug_sprintf(const char* fmt, ...);
char* xdebug_strndup(const char *s, int length);

#endif
