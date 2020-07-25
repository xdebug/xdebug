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

#ifndef __XDEBUG_VECTOR_H__
#define __XDEBUG_VECTOR_H__

#include <stddef.h>

typedef void (*xdebug_vector_dtor)(void *);

typedef struct _xdebug_vector {
	size_t capacity;
	size_t count;

	size_t              element_size;
	void               *data;
	xdebug_vector_dtor  dtor;
} xdebug_vector;

static void __xdebug_grow_vector_if_needed(xdebug_vector *v)
{
	if (v->count + 1 > v->capacity) {
		if (v->capacity == 0) {
			v->capacity = 32;
		} else {
			v->capacity = v->capacity * 3 / 2;
		}

		v->data = xdrealloc(v->data, v->capacity * v->element_size);
	}
}

static inline void *xdebug_vector_push(xdebug_vector *v)
{
	void *tmp_top = NULL;

	__xdebug_grow_vector_if_needed(v);

	v->count++;

	tmp_top = ((char*) v->data + ((v->count - 1) * v->element_size));
	memset(tmp_top, '\0', v->element_size);

	return tmp_top;
}

static inline void xdebug_vector_pop(xdebug_vector *v)
{
	v->dtor(((char*) v->data + ((v->count - 1) * v->element_size)));
	v->count--;
}

static inline void *xdebug_vector_element_get(xdebug_vector *v, size_t index)
{
	if (!v || index >= v->count) {
		return NULL;
	}
	return ((char*) v->data + (index * (v)->element_size));
}

static inline int xdebug_vector_element_is_valid(xdebug_vector *v, void *element)
{
	if ((char*) element < (char*) v->data) {
		return 0;
	}
	if ((char*) element > ((char*) v->data + ((v->count - 1) * v->element_size))) {
		return 0;
	}

	return 1;
}

#define XDEBUG_VECTOR_HEAD(v) xdebug_vector_element_get((v), 0)
#define XDEBUG_VECTOR_TAIL(v) xdebug_vector_element_get((v), (v)->count-1)

#define XDEBUG_VECTOR_COUNT(v) (v)->count

static inline xdebug_vector *xdebug_vector_alloc(size_t element_size, xdebug_vector_dtor dtor)
{
	xdebug_vector *tmp;

	tmp = xdmalloc(sizeof(xdebug_vector));
	tmp->capacity = 0;
	tmp->count = 0;
	tmp->data = NULL;
	tmp->dtor = dtor;
	tmp->element_size = element_size;

	return tmp;
}

static inline void xdebug_vector_destroy(xdebug_vector *v)
{
	xdfree(v->data);
	xdfree(v);
}

#endif /* __XDEBUG_VECTOR_H__ */
