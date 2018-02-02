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
   | Authors: Benjamin Eberlei <kontakt@beberlei.de>					  |
   +----------------------------------------------------------------------+
 */

#ifndef __XDEBUG_GC_STATS_H__
#define __XDEBUG_GC_STATS_H__

typedef struct _xdebug_gc_run {
	zend_long    collected;
	zend_long    duration;
	zend_long    memory_before;
	zend_long    memory_after;
	char        *function_name;
	char        *class_name;
} xdebug_gc_run;

extern int (*xdebug_old_gc_collect_cycles)(void);

int xdebug_gc_stats_init(char *fname, char *script_name);
void xdebug_gc_stats_stop();
int xdebug_gc_collect_cycles(void);

#endif
