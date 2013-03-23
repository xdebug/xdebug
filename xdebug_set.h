/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
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
/* $Id: xdebug_set.h,v 1.4 2010-05-07 20:39:13 derick Exp $ */

#ifndef __XDEBUG_SET_H__
#define __XDEBUG_SET_H__

typedef struct _xdebug_set {
	unsigned int size;
	unsigned char *setinfo;
} xdebug_set;

xdebug_set *xdebug_set_create(unsigned int size);
void xdebug_set_add(xdebug_set *set, unsigned int position);
void xdebug_set_remove(xdebug_set *set, unsigned int position);
#define xdebug_set_in(x,y) xdebug_set_in_ex(x,y,1)
int xdebug_set_in_ex(xdebug_set *set, unsigned int position, int noisy);
void xdebug_set_dump(xdebug_set *set);
void xdebug_set_free(xdebug_set *set);

#endif
