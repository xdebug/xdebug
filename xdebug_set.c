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
/* $Id: xdebug_set.c,v 1.5 2010-05-07 20:39:13 derick Exp $ */

#include <stdlib.h>
#include <math.h>
#include "xdebug_set.h"

xdebug_set *xdebug_set_create(unsigned int size)
{
	xdebug_set *tmp;

	tmp = calloc(1, sizeof(xdebug_set));
	tmp->size = size;
	size = ceil((size + 7) / 8) + 1;
	tmp->setinfo = calloc(1, size);

	return tmp;
}

void xdebug_set_free(xdebug_set *set)
{
	free(set->setinfo);
	free(set);
}

void xdebug_set_add(xdebug_set *set, unsigned int position)
{
	unsigned char *byte;
	unsigned int   bit;
 
	byte = &(set->setinfo[position / 8]);
	bit  = position % 8;

	*byte = *byte | 1 << bit;
}

void xdebug_set_remove(xdebug_set *set, unsigned int position)
{
	unsigned char *byte;
	unsigned int   bit;

	byte = &(set->setinfo[position / 8]);
	bit  = position % 8;

	*byte = *byte & ~(1 << bit);
}

int xdebug_set_in_ex(xdebug_set *set, unsigned int position, int noisy)
{
	unsigned char *byte;
	unsigned int   bit;

	byte = &(set->setinfo[position / 8]);
	bit  = position % 8;

	return (*byte & (1 << bit));
}
