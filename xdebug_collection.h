/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2017 Derick Rethans                               |
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

#ifndef __HAVE_XDEBUG_COLLECTION_H__
#define __HAVE_XDEBUG_COLLECTION_H__

#include "php_xdebug.h"

/*****************************************************************************
** Utility to capture pointers that need to be efreed after variable expansion
*/
typedef void (*xdebug_ptr_collection_dtor_t)(void *ptr);

typedef struct _xdebug_ptr_collection {
	void                         **pointers;
	size_t                         size;
	size_t                         count;
	xdebug_ptr_collection_dtor_t   dtor;
} xdebug_ptr_collection;

xdebug_ptr_collection* xdebug_ptr_collection_ctor(xdebug_ptr_collection_dtor_t dtor);
void xdebug_ptr_collection_dtor(xdebug_ptr_collection *ptr_collection);
void xdebug_ptr_collection_add(xdebug_ptr_collection *ptr_collection, void *ptr);

void xdebug_free_hash_table(void *ptr);

#endif
