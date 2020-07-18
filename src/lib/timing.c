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

#include "php_xdebug.h"
#if PHP_WIN32
	#include "win32/time.h"
	// for MFllMulDiv()
	#include <mfapi.h>
	#pragma comment(lib, "mfplat.lib")
#else
	#include <sys/time.h>
#endif
#ifdef __APPLE__
	#include <mach/mach_time.h>
#endif

#include "timing.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static uint64_t xdebug_get_nanotime_abs(void)
{
	#ifdef HAVE_GETTIMEOFDAY
		struct timeval tp;

		if (gettimeofday(&tp, NULL) == 0) {
			return (uint64_t)tp.tv_sec * NANOS_IN_SEC + (uint64_t)tp.tv_usec * NANOS_IN_MICROSEC;
		}
	#endif

	return 0;
}

static uint64_t xdebug_get_nanotime_rel(xdebug_nanotime_init nanotime_init)
{
	#ifdef _SC_MONOTONIC_CLOCK
		struct timespec ts;
	#elif PHP_WIN32
		LARGE_INTEGER tcounter;
	#endif

	// Linux/Unix
	#ifdef _SC_MONOTONIC_CLOCK
		if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
			return (uint64_t)ts.tv_sec * NANOS_IN_SEC + (uint64_t)ts.tv_nsec;
		}

		return 0;
	#endif

	// Windows
	#if PHP_WIN32
		QueryPerformanceCounter(&tcounter);
		return (uint64_t)MFllMulDiv(
			tcounter.QuadPart,
			NANOS_IN_SEC,
			nanotime_init.win_freq,
			(nanotime_init.win_freq / 2)
		);
	#endif

	// Mac
	#ifdef __APPLE__
		return = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
	#endif

	// fallback if better platform specific relative time library is not available
	return xdebug_get_nanotime_abs();
}

uint64_t xdebug_get_nanotime(void)
{
	return XG_BASE(nanotime_init).start_abs + (xdebug_get_nanotime_rel(XG_BASE(nanotime_init)) - XG_BASE(nanotime_init).start_rel);
}

xdebug_nanotime_init xdebug_get_nanotime_init(void)
{
	xdebug_nanotime_init res;
	#if PHP_WIN32
		LARGE_INTEGER tcounter;
	#endif

	res.start_abs = xdebug_get_nanotime_abs();
	#if PHP_WIN32
		QueryPerformanceFrequency(&tcounter);
		res.win_freq = (uint64_t)tcounter.QuadPart;
	#endif
	res.start_rel = xdebug_get_nanotime_rel(res);

	return res;
}

double xdebug_get_utime(void)
{
	return xdebug_get_nanotime() / (double)NANOS_IN_SEC;
}

char* xdebug_get_time(void)
{
	uint64_t nanotime;
	time_t time_secs;
	char  *res;

	nanotime = xdebug_get_nanotime_abs();
	time_secs = (time_t)(nanotime / NANOS_IN_SEC);
	res = xdmalloc(24);
	strftime(res, 24, "%Y-%m-%d %H:%M:%S", gmtime(&time_secs));
	return res;
}
