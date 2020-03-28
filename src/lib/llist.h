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
   | Authors: Sterling Hughes <sterling@php.net>                          |
   |          Daniel R. Kalowsky <dank@deadmime.org>                      |
   +----------------------------------------------------------------------+
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
