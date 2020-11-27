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

#define NANOS_IN_MICROSEC 1000
#define NANOS_IN_MILLISEC 1000000
#define NANOS_IN_SEC      1000000000

#if PHP_WIN32
typedef void (WINAPI *WIN_PRECISE_TIME_FUNC)(LPFILETIME);
#endif

typedef struct _xdebug_nanotime_context {
	uint64_t start_abs;
	uint64_t last_abs;
#if PHP_WIN32 | __APPLE__ | defined(CLOCK_MONOTONIC)
	uint64_t start_rel;
	uint64_t last_rel;
	int      use_rel_time;
#endif
#if PHP_WIN32
	WIN_PRECISE_TIME_FUNC win_precise_time_func;
	uint64_t win_freq;
#endif
} xdebug_nanotime_context;

void xdebug_nanotime_init(void);

uint64_t xdebug_get_nanotime(void);

char* xdebug_nanotime_to_chars(uint64_t nanotime, unsigned char precision);

#define XDEBUG_SECONDS_SINCE_START(nanotime) (((nanotime) - XG_BASE(start_nanotime)) / (double)NANOS_IN_SEC)

#endif
