/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2021 Derick Rethans                               |
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "set.h"

#define SET_RESIZE_AS_NEEDED(s,p) if (((p) >= (s)->size) && ((s)->resizable)) { set_resize((s), (p+1)); }

#define MEM_SIZE(s) ((s) / 8) + (((s) % 8) != 0)

static inline void set_resize(xdebug_set *set, unsigned int new_size)
{
	unsigned int   old_mem_size, new_mem_size;
	unsigned char *new_mem = NULL;

	old_mem_size = MEM_SIZE(set->size);
	new_mem_size = MEM_SIZE(new_size);

	if (new_mem_size == old_mem_size) {
		set->size = new_size;
		return;
	}

	new_mem = calloc(1, new_mem_size);
	if (old_mem_size && set->setinfo) {
		memcpy(new_mem, set->setinfo, old_mem_size);
	}
	if (set->setinfo) {
		free(set->setinfo);
	}
	set->setinfo = new_mem;
	set->size = new_size;
}

xdebug_set *xdebug_set_create(unsigned int size)
{
	xdebug_set *tmp;

	tmp = calloc(1, sizeof(xdebug_set));
	tmp->resizable = 0;
	set_resize(tmp, size);

	return tmp;
}

xdebug_set *xdebug_set_create_resizable(void)
{
	xdebug_set *tmp = xdebug_set_create(0);

	tmp->resizable = 1;

	return tmp;
}

void xdebug_set_clear(xdebug_set *set)
{
	memset(set->setinfo, 0, MEM_SIZE(set->size));
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

	SET_RESIZE_AS_NEEDED(set, position);

	byte = &(set->setinfo[position / 8]);
	bit  = position % 8;

	*byte = *byte | 1 << bit;
}

void xdebug_set_remove(xdebug_set *set, unsigned int position)
{
	unsigned char *byte;
	unsigned int   bit;

	SET_RESIZE_AS_NEEDED(set, position);

	byte = &(set->setinfo[position / 8]);
	bit  = position % 8;

	*byte = *byte & ~(1 << bit);
}

int xdebug_set_in(xdebug_set *set, unsigned int position)
{
	unsigned char *byte;
	unsigned int   bit;

	if (position >= set->size) {
		return 0;
	}

	byte = &(set->setinfo[position / 8]);
	bit  = position % 8;

	return (*byte & (1 << bit));
}

void xdebug_set_dump(xdebug_set *set)
{
	unsigned int i;

	printf("%p (%d): \n", set, set->size);
	for (i = 0; i < set->size; i++) {
		if (xdebug_set_in(set, i)) {
			printf("%02d ", i);
		}
	}
	printf("\n\n");
}


/** MSET */

#define MSET_RESIZE_AS_NEEDED(s,p) if ((p) >= (s)->size) { mset_resize((s), (p+1)); }

static inline void mset_resize(xdebug_mset *mset, unsigned int new_size)
{
	unsigned char *new_mem = NULL;

	new_mem = calloc(1, new_size);
	if (mset->size && mset->setinfo) {
		memcpy(new_mem, mset->setinfo, mset->size);
	}
	if (mset->setinfo) {
		free(mset->setinfo);
	}
	mset->setinfo = new_mem;
	mset->size = new_size;
}


xdebug_mset *xdebug_mset_create(unsigned char bits)
{
	xdebug_mset *tmp = calloc(1, sizeof(xdebug_mset));

	tmp->size    = 0;
	tmp->setinfo = NULL;

	return tmp;
}

void xdebug_mset_add(xdebug_mset *mset, unsigned int pos, unsigned char bit)
{
	MSET_RESIZE_AS_NEEDED(mset, pos);

	mset->setinfo[pos] |= 1 << bit;
}

int xdebug_mset_in(xdebug_mset *mset, unsigned int pos, unsigned char bit)
{
	if (pos >= mset->size) {
		return 0;
	}

	return (mset->setinfo[pos] & (1 << bit));
}

void xdebug_mset_dump(const char *text, xdebug_mset *mset)
{
	unsigned int i;

	printf("%s: %p (%d): \n", text, mset, mset->size);
	for (i = 0; i < mset->size; i++) {
		if (i % 10 == 0) {
			printf("%d: ", i);
		}
		printf("%02x ", mset->setinfo[i]);
	}
	printf("\n\n");
}

void xdebug_mset_clear(xdebug_mset *mset)
{
	memset(mset->setinfo, 0, mset->size);
}

void xdebug_mset_free(xdebug_mset *mset)
{
	free(mset->setinfo);
	free(mset);
}
