/* $Id: xdebug_llist.h,v 1.4 2003-02-20 14:30:54 derick Exp $ */

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
 * Contributor(s): Sterling Hughes <sterling@php.net> 
 * Daniel R. Kalowsky <dank@deadmime.org>
 */

#ifndef __XDEBUG_LLIST_H__
#define __XDEBUG_LLIST_H__

#include <stddef.h>

typedef void (*xdebug_llist_dtor)(void *, void *);

typedef struct _xdebug_llist_element {
	void *ptr;

	struct _xdebug_llist_element *prev;
	struct _xdebug_llist_element *next;
} xdebug_llist_element;

typedef struct _xdebug_llist {
	xdebug_llist_element *head;
	xdebug_llist_element *tail;

	xdebug_llist_dtor dtor;

	size_t size;
} xdebug_llist;

xdebug_llist *xdebug_llist_alloc(xdebug_llist_dtor dtor);
void xdebug_llist_init(xdebug_llist *l, xdebug_llist_dtor dtor);
int xdebug_llist_insert_next(xdebug_llist *l, xdebug_llist_element *e, const void *p);
int xdebug_llist_insert_prev(xdebug_llist *l, xdebug_llist_element *e, const void *p);
int xdebug_llist_remove(xdebug_llist *l, xdebug_llist_element *e, void *user);
int xdebug_llist_remove_next(xdebug_llist *l, xdebug_llist_element *e, void *user);
xdebug_llist_element *xdebug_llist_jump(xdebug_llist *l, int where, int pos);
size_t xdebug_llist_count(xdebug_llist *l);
void xdebug_llist_empty(xdebug_llist *l, void *user);
void xdebug_llist_destroy(xdebug_llist *l, void *user);

#if !defined(LIST_HEAD)
#define LIST_HEAD 0
#endif

#if !defined(LIST_TAIL)
#define LIST_TAIL 1
#endif

#define XDEBUG_LLIST_HEAD(__l) ((__l)->head)
#define XDEBUG_LLIST_TAIL(__l) ((__l)->tail)
#define XDEBUG_LLIST_NEXT(__e) ((__e)->next)
#define XDEBUG_LLIST_PREV(__e) ((__e)->prev)
#define XDEBUG_LLIST_VALP(__e) ((__e)->ptr)
#define XDEBUG_LLIST_IS_TAIL(__e) ((__e)->next ? 0 : 1)
#define XDEBUG_LLIST_IS_HEAD(__e) ((__e)->prev ? 0 : 1)
#define XDEBUG_LLIST_COUNT(__l) ((__l)->size)

#endif /* __XDEBUG_LLIST_H__ */
