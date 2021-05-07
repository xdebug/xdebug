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
 */

#include <stdlib.h>
#include <string.h>

#include "llist.h"

xdebug_llist *xdebug_llist_alloc(xdebug_llist_dtor dtor)
{
	xdebug_llist *l;

	l = malloc(sizeof(xdebug_llist));
	xdebug_llist_init(l, dtor);

	return l;
}

void xdebug_llist_init(xdebug_llist *l, xdebug_llist_dtor dtor)
{
	l->size = 0;
	l->dtor = dtor;
	l->head = NULL;
	l->tail = NULL;
}

int xdebug_llist_insert_next(xdebug_llist *l, xdebug_llist_element *e, const void *p)
{
	xdebug_llist_element  *ne;

	if (!e) {
		e = XDEBUG_LLIST_TAIL(l);
	}

	ne = (xdebug_llist_element *) malloc(sizeof(xdebug_llist_element));
	ne->ptr = (void *) p;
	if (l->size == 0) {
		l->head = ne;
		l->head->prev = NULL;
		l->head->next = NULL;
		l->tail = ne;
	} else {
		ne->next = e->next;
		ne->prev = e;
		if (e->next) {
			e->next->prev = ne;
		} else {
			l->tail = ne;
		}
		e->next = ne;
	}

	++l->size;

	return 1;
}

int xdebug_llist_insert_prev(xdebug_llist *l, xdebug_llist_element *e, const void *p)
{
	xdebug_llist_element *ne;

	if (!e) {
		e = XDEBUG_LLIST_HEAD(l);
	}

	ne = (xdebug_llist_element *) malloc(sizeof(xdebug_llist_element));
	ne->ptr = (void *) p;
	if (l->size == 0) {
		l->head = ne;
		l->head->prev = NULL;
		l->head->next = NULL;
		l->tail = ne;
	} else {
		ne->next = e;
		ne->prev = e->prev;
		if (e->prev)
			e->prev->next = ne;
		else
			l->head = ne;
		e->prev = ne;
	}

	++l->size;

	return 0;
}

int xdebug_llist_remove(xdebug_llist *l, xdebug_llist_element *e, void *user)
{
	if (e == NULL || l->size == 0)
		return 0;

	if (e == l->head) {
		l->head = e->next;

		if (l->head == NULL)
			l->tail = NULL;
		else
			e->next->prev = NULL;
	} else {
		e->prev->next = e->next;
		if (!e->next)
			l->tail = e->prev;
		else
			e->next->prev = e->prev;
	}

	if (l->dtor) {
		l->dtor(user, e->ptr);
	}
	free(e);
	--l->size;

	return 0;
}

int xdebug_llist_remove_next(xdebug_llist *l, xdebug_llist_element *e, void *user)
{
	return xdebug_llist_remove(l, e->next, user);
}

int xdebug_llist_remove_prev(xdebug_llist *l, xdebug_llist_element *e, void *user)
{
	return xdebug_llist_remove(l, e->prev, user);
}

xdebug_llist_element *xdebug_llist_jump(xdebug_llist *l, int where, int pos)
{
    xdebug_llist_element *e=NULL;
    int i;

    if (where == LIST_HEAD) {
        e = XDEBUG_LLIST_HEAD(l);
        for (i = 0; i < pos; ++i) {
            e = XDEBUG_LLIST_NEXT(e);
        }
    }
    else if (where == LIST_TAIL) {
        e = XDEBUG_LLIST_TAIL(l);
        for (i = 0; i < pos; ++i) {
            e = XDEBUG_LLIST_PREV(e);
        }
    }

    return e;
}

size_t xdebug_llist_count(xdebug_llist *l)
{
	return l->size;
}

void xdebug_llist_empty(xdebug_llist *l, void *user)
{
	while (xdebug_llist_count(l) > 0 && XDEBUG_LLIST_TAIL(l)) {
		xdebug_llist_remove(l, XDEBUG_LLIST_TAIL(l), user);
	}
}

void xdebug_llist_destroy(xdebug_llist *l, void *user)
{
	xdebug_llist_empty(l, user);

	free (l);
}

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
