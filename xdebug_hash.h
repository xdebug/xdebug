/* $Id: xdebug_hash.h,v 1.2 2006-01-22 23:30:59 derick Exp $ */

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
 * Contributor(s):
 */

#ifndef __XDEBUG_HASH_H__
#define __XDEBUG_HASH_H__

#include <stddef.h>

#include "xdebug_llist.h"

#define XDEBUG_HASH_KEY_IS_STRING 0
#define XDEBUG_HASH_KEY_IS_NUM    1

#define xdebug_ui32 unsigned long

typedef void (*xdebug_hash_dtor)(void *);

typedef struct _xdebug_hash {
	xdebug_llist     **table;
	xdebug_hash_dtor   dtor;
	int             slots;
	size_t          size;
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
xdebug_hash *xdebug_hash_alloc(int slots, xdebug_hash_dtor dtor);
int  xdebug_hash_add_or_update(xdebug_hash *h, char *str_key, unsigned int str_key_len, unsigned long num_key, const void *p);
int  xdebug_hash_extended_delete(xdebug_hash *h, char *str_key, unsigned int str_key_len, unsigned long num_key);
int  xdebug_hash_extended_find(xdebug_hash *h, char *str_key, unsigned int str_key_len, unsigned long num_key, void **p);
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
