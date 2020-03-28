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

#include "php_xdebug.h"
#include "com.h"
#include "handlers.h"
#include "handler_dbgp.h"
#include "lib/mm.h"

void xdebug_brk_info_dtor(xdebug_brk_info *brk_info)
{
	if (brk_info->classname) {
		xdfree(brk_info->classname);
	}
	if (brk_info->functionname) {
		xdfree(brk_info->functionname);
	}
	if (brk_info->file) {
		xdfree(brk_info->file);
	}
	if (brk_info->condition) {
		xdfree(brk_info->condition);
	}
	xdfree(brk_info);
}

void xdebug_hash_brk_dtor(xdebug_brk_info *brk_info)
{
	xdebug_brk_info_dtor(brk_info);
}

void xdebug_llist_brk_dtor(void *dummy, xdebug_brk_info *brk_info)
{
	xdebug_brk_info_dtor(brk_info);
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
