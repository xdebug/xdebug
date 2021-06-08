/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2025 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
 */

#ifndef __XDEBUG_SET_H__
#define __XDEBUG_SET_H__

typedef struct _xdebug_set {
	unsigned int   size;
	unsigned char *setinfo;
	char           resizable;
} xdebug_set;

typedef struct _xdebug_mset {
	unsigned int   size;
	unsigned char *setinfo;
	unsigned char  bits;
} xdebug_mset;

xdebug_set *xdebug_set_create(unsigned int size);
xdebug_set *xdebug_set_create_resizable(void);
void xdebug_set_add(xdebug_set *set, unsigned int position);
void xdebug_set_remove(xdebug_set *set, unsigned int position);
int xdebug_set_in(xdebug_set *set, unsigned int position);
void xdebug_set_dump(xdebug_set *set);
void xdebug_set_clear(xdebug_set *set);
void xdebug_set_free(xdebug_set *set);

xdebug_mset *xdebug_mset_create(unsigned char bits);
void xdebug_mset_add(xdebug_mset *set, unsigned int position, unsigned char bit);
int xdebug_mset_in(xdebug_mset *set, unsigned int position, unsigned char bit);
void xdebug_mset_dump(const char *text, xdebug_mset *set);
void xdebug_mset_clear(xdebug_mset *set);
void xdebug_mset_free(xdebug_mset *set);

#endif
