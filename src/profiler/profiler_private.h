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
#ifndef __XDEBUG_PROFILER_PRIVATE_H__
#define __XDEBUG_PROFILER_PRIVATE_H__

typedef struct _xdebug_call_entry {
	int          type; /* 0 = function call, 1 = line */
	int          user_defined;
	zend_string *filename;
	char        *function;
	int          lineno;
	uint64_t     nanotime_taken;
	long         mem_used;
} xdebug_call_entry;

#define XG_PROF(v)     (XG(globals.profiler.v))
#define XINI_PROF(v)   (XG(settings.profiler.v))

#endif
