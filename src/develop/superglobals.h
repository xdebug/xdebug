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

#ifndef __HAVE_XDEBUG_SUPERGLOBALS_H__
#define __HAVE_XDEBUG_SUPERGLOBALS_H__

#include "php.h"

#include "develop_private.h"

void xdebug_superglobals_dump_dtor(void *, void*);
char *xdebug_get_printable_superglobals(int html);
void xdebug_superglobals_dump_tok(xdebug_llist *l, char *str);

# define DUMP_TOK(__llist) \
	xdebug_llist_empty(&XG_DEV(__llist), NULL); \
	if (new_value && new_value->val) { \
		char *str = estrndup(new_value->val, new_value->len); \
		xdebug_superglobals_dump_tok(&XG_DEV(__llist), str); \
		efree(str); \
	} \
	return SUCCESS;

#endif /* __HAVE_XDEBUG_SUPERGLOBALS_H__ */
