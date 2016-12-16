/* $Id: xdebug_hash.c,v 1.6 2008-11-27 18:56:26 derick Exp $ */

/* The contents of this file are subject to the Vulcan Logic Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.vl-srm.net/vlpl/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is vl-srm.net code.
 *
 * The Initial Developer of the Original Code is the Vulcan Logic
 * Group.  Portions created by Vulcan Logic Group are Copyright (C)
 * 2000, 2001, 2002 Vulcan Logic Group. All Rights Reserved.
 *
 * Author(s): Sterling Hughes <sterling@php.net>
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "xdebug_hash.h"
#include "xdebug_llist.h"

/*
 * Helper function to make a null terminated string from a key
 */

char *xdebug_hash_key_to_str(xdebug_hash_key* key, int* new_len)
{
	char *tmp;

	tmp = calloc (key->value.str.len + 1, 1);
	memcpy(tmp, key->value.str.val, key->value.str.len);
	*new_len = key->value.str.len;

	return tmp;
}

static xdebug_ui32 xdebug_hash_str(const char *key, unsigned int key_length)
{
	char *p = (char *) key;
	char *end = (char *) key + key_length;
	unsigned long h = 5381;

	while (p < end) {
		h += h << 5;
		h ^= (unsigned long) *p++;
	}

	return h;
}

static xdebug_ui32 xdebug_hash_num(xdebug_ui32 key)
{
	key += ~(key << 15);
	key ^= (key >> 10);
	key += (key << 3);
	key ^= (key >> 6);
	key += (key << 11);
	key ^= (key >> 16);

	return key;
}

static void hash_element_dtor(void *u, void *ele)
{
	xdebug_hash_element *e = (xdebug_hash_element *) ele;
	xdebug_hash         *h = (xdebug_hash *) u;

	if (e->key.type == XDEBUG_HASH_KEY_IS_STRING) {
		free(e->key.value.str.val);
	}
	if (h->dtor) {
		h->dtor(e->ptr);
	}

	free(e);
	e = NULL;
}

xdebug_hash *xdebug_hash_alloc(int slots, xdebug_hash_dtor dtor)
{
	xdebug_hash *h;
	int          i;

	h = malloc(sizeof(xdebug_hash));
	h->dtor  = dtor;
	h->size  = 0;
	h->slots = slots;

	h->table = (xdebug_llist **) malloc(slots * sizeof(xdebug_llist *));
	for (i = 0; i < h->slots; ++i) {
		h->table[i] = xdebug_llist_alloc((xdebug_llist_dtor) hash_element_dtor);
	}

	return h;
}

#define FIND_SLOT(__h, __s_key, __s_key_len, __n_key) \
	((__s_key ? xdebug_hash_str(__s_key, __s_key_len) : xdebug_hash_num(__n_key)) % (__h)->slots)

#define KEY_CREATE(__k, __s_key, __s_key_len, __n_key, __dup) \
	if (__s_key) { \
		if (__dup) { \
			(__k)->value.str.val = (char *) malloc(__s_key_len); \
			memcpy((__k)->value.str.val, __s_key, __s_key_len); \
		} else { \
			(__k)->value.str.val = __s_key; \
		} \
		(__k)->value.str.len = __s_key_len; \
		(__k)->type = XDEBUG_HASH_KEY_IS_STRING; \
	} else { \
		(__k)->value.num = __n_key; \
		(__k)->type = XDEBUG_HASH_KEY_IS_NUM; \
	}

static int xdebug_hash_key_compare(xdebug_hash_key *key1, xdebug_hash_key *key2)
{
	if (key1->type == XDEBUG_HASH_KEY_IS_NUM) {
		if (key2->type == XDEBUG_HASH_KEY_IS_STRING)
			return 0;

		if (key1->value.num == key2->value.num)
			return 1;
	} else {
		if (key2->type == XDEBUG_HASH_KEY_IS_NUM)
			return 0;

		if (key1->value.str.len == key2->value.str.len &&
			*key1->value.str.val == *key2->value.str.val &&
			memcmp(key1->value.str.val, key2->value.str.val, key1->value.str.len) == 0) {
			return 1;
		}
	}

	return 0;
}

int xdebug_hash_add_or_update(xdebug_hash *h, char *str_key, unsigned int str_key_len, unsigned long num_key, const void *p)
{
	xdebug_hash_element  *e;
	xdebug_hash_key       tmp;
	xdebug_llist         *l;
	xdebug_llist_element *le;
	int                slot;

	slot = FIND_SLOT(h, str_key, str_key_len, num_key);
	l = h->table[slot];
	KEY_CREATE(&tmp, str_key, str_key_len, num_key, 0);
	for (le = XDEBUG_LLIST_HEAD(l); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
		if (xdebug_hash_key_compare(&tmp, &((xdebug_hash_element *) XDEBUG_LLIST_VALP(le))->key)) {
			xdebug_hash_element *to_update = XDEBUG_LLIST_VALP(le);
			if (h->dtor) {
				h->dtor(to_update->ptr);
			}
			to_update->ptr = (void *) p;
			return 1;
		}
	}

	e = (xdebug_hash_element *) malloc(sizeof(xdebug_hash_element));
	KEY_CREATE(&e->key, str_key, str_key_len, num_key, 1);
	e->ptr = (void *) p;

	if (xdebug_llist_insert_next(l, XDEBUG_LLIST_TAIL(l), e)) {
		++h->size;
		return 1;
	} else {
		return 0;
	}
}

int xdebug_hash_extended_delete(xdebug_hash *h, char *str_key, unsigned int str_key_len, xdebug_ui32 num_key)
{
	xdebug_llist         *l;
	xdebug_llist_element *le;
	xdebug_hash_key       tmp;
	int                slot;

	slot = FIND_SLOT(h, str_key, str_key_len, num_key);
	l = h->table[slot];

	KEY_CREATE(&tmp, str_key, str_key_len, num_key, 0);
	for (le = XDEBUG_LLIST_HEAD(l); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
		if (xdebug_hash_key_compare(&tmp, &((xdebug_hash_element *) XDEBUG_LLIST_VALP(le))->key)) {
			xdebug_llist_remove(l, le, (void *) h);
			--h->size;
			return 1;
		}
	}

	return 0;
}

int xdebug_hash_extended_find(xdebug_hash *h, char *str_key, unsigned int str_key_len, xdebug_ui32 num_key, void **p)
{
	xdebug_llist         *l;
	xdebug_llist_element *le;
	xdebug_hash_key       tmp;
	int                slot;

	slot = FIND_SLOT(h, str_key, str_key_len, num_key);
	l = h->table[slot];

	KEY_CREATE(&tmp, str_key, str_key_len, num_key, 0);
	for (le = XDEBUG_LLIST_HEAD(l); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
		if (xdebug_hash_key_compare(&tmp, &((xdebug_hash_element *) XDEBUG_LLIST_VALP(le))->key)) {
			*p = ((xdebug_hash_element *) XDEBUG_LLIST_VALP(le))->ptr;
			return 1;
		}
	}

	return 0;
}

void xdebug_hash_apply(xdebug_hash *h, void *user, void (*cb)(void *, xdebug_hash_element *))
{
	 xdebug_llist_element  *le;
	 int                    i;

	 for (i = 0; i < h->slots; ++i) {
		  for (le = XDEBUG_LLIST_HEAD(h->table[i]); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
			   cb(user, (xdebug_hash_element *) XDEBUG_LLIST_VALP(le));
		  }
	 }
}

static int xdebug_compare_le_name(const void *le1, const void *le2)
{
	return strcmp((char *) XDEBUG_LLIST_VALP(*(xdebug_llist_element **) le1),
		(char *) XDEBUG_LLIST_VALP(*(xdebug_llist_element **) le2));
}

void xdebug_hash_apply_with_argument(xdebug_hash *h, void *user, void (*cb)(void *, xdebug_hash_element *, void *), void *argument)
{
	xdebug_llist_element  *le;
	int                    i;
	int                    num_items = 0;
	xdebug_hash_element  **pp_he_list;

	for (i = 0; i < h->slots; ++i) {
		for (le = XDEBUG_LLIST_HEAD(h->table[i]); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
			num_items += 1;
		}
	}
	pp_he_list = (xdebug_hash_element **) malloc((size_t) (num_items * sizeof(xdebug_hash_element **)));
	if (pp_he_list) {
		int j = 0;
		for (i = 0; i < h->slots; ++i) {
			for (le = XDEBUG_LLIST_HEAD(h->table[i]); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
				pp_he_list[j++] = XDEBUG_LLIST_VALP(le);
			}
		}
		qsort(pp_he_list, num_items, sizeof(xdebug_llist_element *), xdebug_compare_le_name);
		for (i = 0; i < num_items; ++i) {
			cb(user, pp_he_list[i], argument);
		}
		free((void *) pp_he_list);
	} else {
		for (i = 0; i < h->slots; ++i) {
			for (le = XDEBUG_LLIST_HEAD(h->table[i]); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
				cb(user, (xdebug_hash_element *) XDEBUG_LLIST_VALP(le), argument);
			}
		}
	}
}

void xdebug_hash_destroy(xdebug_hash *h)
{
	int i;

    for (i = 0; i < h->slots; ++i) {
		xdebug_llist_destroy(h->table[i], (void *) h);
	}

	free(h->table);
	free(h);
}

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
