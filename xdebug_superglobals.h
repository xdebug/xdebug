/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 The PHP Group                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Harald Radi <harald.radi@nme.at>                            |
   +----------------------------------------------------------------------+
 */

#ifndef __HAVE_XDEBUG_SUPERGLOBALS_H__
#define __HAVE_XDEBUG_SUPERGLOBALS_H__

#include "php.h"

void dump_dtor(void *, void*);
void dump_superglobals(int html, int log TSRMLS_DC);
void dump_tok(xdebug_llist *l, char *str);

#define DUMP_TOK(__llist) \
	xdebug_llist_empty(&XG(__llist), NULL); \
	if (new_value && *new_value) { \
		char *str = estrndup(new_value, new_value_length); \
		dump_tok(&XG(__llist), str); \
		efree(str); \
	} \
	return SUCCESS;

#endif /* __HAVE_XDEBUG_SUPERGLOBALS_H__ */
