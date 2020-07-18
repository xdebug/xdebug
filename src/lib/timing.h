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
   |          Michael Voříšek <mvorisek@mvorisek.cz>                      |
   +----------------------------------------------------------------------+
 */

#ifndef __XDEBUG_TIMING_H__
#define __XDEBUG_TIMING_H__

#define NANOS_IN_SEC 1000000000
#define NANOS_IN_MICROSEC 1000

typedef struct _xdebug_nanotime_init {
	uint64_t start_abs;
#if PHP_WIN32
	uint64_t win_freq;
#endif
	uint64_t start_rel;
} xdebug_nanotime_init;

uint64_t xdebug_get_nanotime(void);
xdebug_nanotime_init xdebug_get_nanotime_init(void);
double xdebug_get_utime(void);
char* xdebug_get_time(void);

#endif
