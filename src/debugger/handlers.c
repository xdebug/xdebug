/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2018 Derick Rethans                               |
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

xdebug_brk_info *xdebug_brk_info_ctor(void)
{
	xdebug_brk_info *brk_info;

	brk_info = (xdebug_brk_info *)xdmalloc(sizeof(xdebug_brk_info));

	if (!brk_info) {
		return NULL;
	}
	brk_info->id = -1;
	brk_info->brk_type = -1;
	brk_info->resolved = XDEBUG_BRK_UNRESOLVED;
	brk_info->file = NULL;
	brk_info->file_len = 0;
	brk_info->original_lineno = 0;
	brk_info->resolved_lineno = 0;
	brk_info->resolved_span.start = XDEBUG_RESOLVED_SPAN_MIN;
	brk_info->resolved_span.end   = XDEBUG_RESOLVED_SPAN_MAX;
	brk_info->classname = NULL;
	brk_info->functionname = NULL;
	brk_info->function_break_type = 0;
	brk_info->exceptionname = NULL;
	brk_info->condition = NULL;
	brk_info->disabled = 0;
	brk_info->temporary = 0;
	brk_info->hit_count = 0;
	brk_info->hit_value = 0;
	brk_info->hit_condition = XDEBUG_HIT_DISABLED;

	return brk_info;
}

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
