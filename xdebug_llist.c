/* $Id: xdebug_llist.c,v 1.5 2007-01-02 16:02:37 derick Exp $ */

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

#include <stdlib.h>
#include <string.h>

#include "xdebug_llist.h"

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
	while (xdebug_llist_count(l) > 0) {
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
