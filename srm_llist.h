/* $Id: srm_llist.h,v 1.1.1.1 2002-04-24 14:26:19 derick Exp $ */

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

#ifndef __SRM_LLIST_H__
#define __SRM_LLIST_H__

#include <stddef.h>

typedef void (*srm_llist_dtor)(void *, void *);

typedef struct _srm_llist_element {
	void *ptr;

	struct _srm_llist_element *prev;
	struct _srm_llist_element *next;
} srm_llist_element;

typedef struct _srm_llist {
	srm_llist_element *head;
	srm_llist_element *tail;

	srm_llist_dtor dtor;

	size_t size;
} srm_llist;

srm_llist *srm_llist_alloc(srm_llist_dtor dtor);
int srm_llist_insert_next(srm_llist *l, srm_llist_element *e, const void *p);
int srm_llist_insert_prev(srm_llist *l, srm_llist_element *e, const void *p);
int srm_llist_remove(srm_llist *l, srm_llist_element *e, void *user);
int srm_llist_remove_next(srm_llist *l, srm_llist_element *e, void *user);
srm_llist_element *srm_llist_jump(srm_llist *l, int where, int pos);
size_t srm_llist_count(srm_llist *l);
void srm_llist_destroy(srm_llist *l, void *user);

#if !defined(LIST_HEAD)
#define LIST_HEAD 0
#endif

#if !defined(LIST_TAIL)
#define LIST_TAIL 1
#endif

#define SRM_LLIST_HEAD(__l) ((__l)->head)
#define SRM_LLIST_TAIL(__l) ((__l)->tail)
#define SRM_LLIST_NEXT(__e) ((__e)->next)
#define SRM_LLIST_PREV(__e) ((__e)->prev)
#define SRM_LLIST_VALP(__e) ((__e)->ptr)
#define SRM_LLIST_IS_TAIL(__e) ((__e)->next ? 0 : 1)
#define SRM_LLIST_IS_HEAD(__e) ((__e)->prev ? 0 : 1)

#endif /* __SRM_LLIST_H__ */
