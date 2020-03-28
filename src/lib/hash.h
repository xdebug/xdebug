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

#ifndef __XDEBUG_HASH_H__
#define __XDEBUG_HASH_H__

#include <stddef.h>

#include "llist.h"

#define XDEBUG_HASH_KEY_IS_STRING 0
#define XDEBUG_HASH_KEY_IS_NUM    1

#define xdebug_ui32 unsigned long

typedef void (*xdebug_hash_dtor_t)(void *);
typedef int (*xdebug_hash_apply_sorter_t)(const void *le1, const void *le2);

typedef struct _xdebug_hash {
	xdebug_llist               **table;
	xdebug_hash_dtor_t           dtor;
	xdebug_hash_apply_sorter_t   sorter;
	int                          slots;
	size_t                       size;
} xdebug_hash;

typedef struct _xdebug_hash_key {
	union {
		struct {
			char *val;
			unsigned int len;
		} str;

		unsigned long num;
	} value;

	int type;
} xdebug_hash_key;

typedef struct _xdebug_hash_element {
	void         *ptr;
	xdebug_hash_key  key;
} xdebug_hash_element;

/* Helper functions */
char* xdebug_hash_key_to_str(xdebug_hash_key* key, int* new_len);

/* Standard functions */
xdebug_hash *xdebug_hash_alloc(int slots, xdebug_hash_dtor_t dtor);
xdebug_hash *xdebug_hash_alloc_with_sort(int slots, xdebug_hash_dtor_t dtor, xdebug_hash_apply_sorter_t sort_func);
int  xdebug_hash_add_or_update(xdebug_hash *h, const char *str_key, unsigned int str_key_len, unsigned long num_key, const void *p);
int  xdebug_hash_extended_delete(xdebug_hash *h, const char *str_key, unsigned int str_key_len, unsigned long num_key);
int  xdebug_hash_extended_find(xdebug_hash *h, const char *str_key, unsigned int str_key_len, unsigned long num_key, void **p);
void xdebug_hash_apply(xdebug_hash *h, void *user, void (*cb)(void *, xdebug_hash_element *));
void xdebug_hash_apply_with_argument(xdebug_hash *h, void *user, void (*cb)(void *, xdebug_hash_element *, void *), void *argument);
void xdebug_hash_destroy(xdebug_hash *h);

#define xdebug_hash_find(h, key, key_len, p) xdebug_hash_extended_find(h, key, key_len, 0, p)
#define xdebug_hash_delete(h, key, key_len) xdebug_hash_extended_delete(h, key, key_len, 0)
#define xdebug_hash_add(h, key, key_len, p) xdebug_hash_add_or_update(h, key, key_len, 0, p)
#define xdebug_hash_update xdebug_hash_add
#define xdebug_hash_index_find(h, key, p) xdebug_hash_extended_find(h, NULL, 0, key, p)
#define xdebug_hash_index_delete(h, key) xdebug_hash_extended_delete(h, NULL, 0, key)
#define xdebug_hash_index_add(h, key, p) xdebug_hash_add_or_update(h, NULL, 0, key, p)
#define xdebug_hash_index_update xdebug_hash_index_add

#endif /* __XDEBUG_HASH_H__ */
