/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
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

#include "php_xdebug.h"
#include "xdebug_com.h"
#include "xdebug_handlers.h"
#include "xdebug_handler_dbgp.h"
#include "xdebug_mm.h"

xdebug_remote_handler_info handlers[] = {
	{ "dbgp", "DBGp - Common DeBuGger Protocol", xdebug_handler_dbgp },
	{ 0, NULL, { NULL } }
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

xdebug_remote_handler_info* xdebug_handlers_get(void)
{
	return handlers;
}

void xdebug_brk_info_dtor(xdebug_brk_info *brk)
{
	if (brk->type) {
		xdfree(brk->type);
	}
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
		efree(brk->condition);
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

void xdebug_hash_eval_info_dtor(xdebug_eval_info *ei)
{
	ei->refcount--;

	if (ei->refcount == 0) {
		xdfree(ei->contents);
		xdfree(ei);
	} else {
		/* refcount wasn't 0 yet, so we won't free it yet */
	}
}
