/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2017 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
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
	zend_string *function_name;
	zend_string *class_name;
	zval stack;
} xdebug_gc_run;

int (*xdebug_old_gc_collect_cycles)(void);
int xdebug_gc_collect_cycles(void);
int xdebug_gc_stats_init(char *fname, char *script_name);
void xdebug_gc_stats_print_run(xdebug_gc_run *run);
void xdebug_gc_stats_run_free(xdebug_gc_run *run);
void xdebug_gc_stats_stop();

void xdebug_gc_stats_show_report();

#endif
