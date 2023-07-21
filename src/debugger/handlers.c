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
 */

#include "php_xdebug.h"
#include "com.h"
#include "handlers.h"
#include "handler_dbgp.h"
#include "lib/mm.h"

xdebug_brk_info *xdebug_brk_info_ctor(void)
{
	xdebug_brk_info *tmp = xdmalloc(sizeof(xdebug_brk_info));

	tmp->id = -1;
	tmp->brk_type = -1;
	tmp->resolved = XDEBUG_BRK_UNRESOLVED;
	tmp->filename = NULL;
	tmp->original_lineno = 0;
	tmp->resolved_lineno = 0;
	tmp->classname = NULL;
	tmp->functionname = NULL;
	tmp->function_break_type = 0;
	tmp->exceptionname = NULL;
	tmp->condition = NULL;
	tmp->disabled = 0;
	tmp->temporary = 0;
	tmp->hit_count = 0;
	tmp->hit_value = 0;
	tmp->hit_condition = XDEBUG_HIT_DISABLED;

	return tmp;
}

void xdebug_brk_info_dtor(xdebug_brk_info *brk_info)
{
	if (brk_info->classname) {
		xdfree(brk_info->classname);
	}
	if (brk_info->functionname) {
		xdfree(brk_info->functionname);
	}
	if (brk_info->filename) {
		zend_string_release(brk_info->filename);
	}
	if (brk_info->exceptionname) {
		xdfree(brk_info->exceptionname);
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
		zend_string_release(ei->contents);
		xdfree(ei);
	}
}
