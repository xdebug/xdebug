/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 Derick Rethans                              |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   +----------------------------------------------------------------------+
 */

#include "xdebug_com.h"
#include "php_xdebug.h"
#include "xdebug_handlers.h"
#include "xdebug_handler_php3.h"
#include "xdebug_handler_gdb.h"

xdebug_remote_handler_info handlers[] = {
	{ "php3", xdebug_handler_php3 },
	{ "gdb",  xdebug_handler_gdb },
	{ 0, { NULL } }
};

xdebug_remote_handler* xdebug_handler_get(char* mode)
{
	xdebug_remote_handler_info *ptr = handlers;

	while (ptr->name) {
		if (strcmp(mode, ptr->name) == 0) {
			return &ptr->handler;
		}
		ptr++;
	}
	return NULL;
}

void xdebug_brk_info_dtor(xdebug_brk_info *brk)
{
	if (brk->classname) {
		xdfree(brk->classname);
	}
	if (brk->functionname) {
		xdfree(brk->functionname);
	}
	if (brk->file) {
		xdfree(brk->file);
	}
	if (brk->condition) {
		xdfree(brk->condition);
	}
	xdfree(brk);
}

void xdebug_hash_brk_dtor(xdebug_brk_info *brk)
{
	xdebug_brk_info_dtor(brk);
}

void xdebug_llist_brk_dtor(void *dummy, xdebug_brk_info *brk)
{
	xdebug_brk_info_dtor(brk);
}

