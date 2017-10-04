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

#include "xdebug_collection.h"

xdebug_ptr_collection* xdebug_ptr_collection_ctor(xdebug_ptr_collection_dtor_t dtor)
{
	xdebug_ptr_collection *tmp = xdmalloc(sizeof(xdebug_ptr_collection));
	tmp->pointers = xdmalloc(64 * sizeof(void *));
	tmp->size = 64;
	tmp->count = 0;
	tmp->dtor = dtor;

//fprintf(stderr, "Allocated PTR collector\n");

	return tmp;
}

void xdebug_ptr_collection_dtor(xdebug_ptr_collection *ptr_collection)
{
	size_t i;

	/* Destroy hashes and free hash tables stored in the collection */
	for (i = 0; i < ptr_collection->count; ++i) {
		ptr_collection->dtor(ptr_collection->pointers[i]);

//fprintf(stderr, "Freeing item %lu\n", i);
	}

	xdfree(ptr_collection);
//fprintf(stderr, "Freeing PTR collector\n");
}

void xdebug_ptr_collection_add(xdebug_ptr_collection *ptr_collection, void *ptr)
{
	if (ptr_collection->count == ptr_collection->size) {
		ptr_collection->size *= 2;
//fprintf(stderr, "Resizing collector to %lu\n", ptr_collection->size);
		ptr_collection->pointers = xdrealloc(ptr_collection, ptr_collection->size * sizeof(void *));
	}

//fprintf(stderr, "Added item %lu (%p)\n", ptr_collection->count, ptr);

	ptr_collection->pointers[ptr_collection->count] = ptr;
	ptr_collection->count++;
}

void xdebug_free_hash_table(void *ptr)
{
	zend_hash_destroy((HashTable*) ptr);
	efree(ptr);
//fprintf(stderr, "Freeing %p\n", ptr);
}

