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
# include "win32/time.h"
# include <versionhelpers.h>
#else
# include <sys/time.h>
#endif
#if __APPLE__
# include <mach/mach_time.h>
#endif

#include "timing.h"

#define NANOTIME_MIN_STEP 10

#if PHP_WIN32
# define WIN_NANOS_IN_TICK          100
# define WIN_TICKS_SINCE_1601_JAN_1 116444736000000000ULL
#endif

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static uint64_t xdebug_get_nanotime_abs(xdebug_nanotime_context *nanotime_context)
{
#if PHP_WIN32
	// Windows, win_precise_time_func is only available on Win 8 and later.
	{
		FILETIME filetime;

		if (nanotime_context->win_precise_time_func != NULL) {
			nanotime_context->win_precise_time_func(&filetime);
			return ((((uint64_t)filetime.dwHighDateTime << 32) + (uint64_t)filetime.dwLowDateTime) - WIN_TICKS_SINCE_1601_JAN_1)
				* WIN_NANOS_IN_TICK;
		}
	}
#endif

#if HAVE_GETTIMEOFDAY | PHP_WIN32
	// Fallback to gettimeofday() if better platform specific time is not
	// available. gettimeofday() is always available in PHP on Windows, as it's
	// expose through 'win32/time.[ch]'
	{
		struct timeval tp;

		if (gettimeofday(&tp, NULL) == 0) {
			return (uint64_t)tp.tv_sec * NANOS_IN_SEC + (uint64_t)tp.tv_usec * NANOS_IN_MICROSEC;
		}
	}
#endif

	// We give up
	php_error(E_WARNING, "Xdebug could not determine a suitable clock source on your system");
	return 0;
}

#if PHP_WIN32
static uint64_t xdebug_counter_and_freq_to_nanotime(uint64_t counter, uint64_t freq)
{
	uint32_t mul = 1, freq32;
	uint64_t q, r;

	while (freq >= (1ULL << 32)) {
		freq /= 2;
		mul *= 2;
	}
	freq32 = (uint32_t)freq;

	q = counter / freq32;
	r = counter % freq32;
	return (q * NANOS_IN_SEC + (r * NANOS_IN_SEC) / freq32) * mul;
}
#endif


// Windows 7 and lower
#if PHP_WIN32
static uint64_t xdebug_get_nanotime_rel(xdebug_nanotime_context *nanotime_context)
{
	LARGE_INTEGER tcounter;

	if (nanotime_context->win_precise_time_func == NULL) {
		QueryPerformanceCounter(&tcounter);
		return xdebug_counter_and_freq_to_nanotime((uint64_t)tcounter.QuadPart, nanotime_context->win_freq);
	}
}

// Mac
// should be fast but can be relative
#elif __APPLE__
static uint64_t xdebug_get_nanotime_rel(xdebug_nanotime_context *nanotime_context)
{
	return clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
}

// Linux/Unix with clock_gettime
#elif defined(CLOCK_MONOTONIC)
static uint64_t xdebug_get_nanotime_rel(xdebug_nanotime_context *nanotime_context)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		return (uint64_t)ts.tv_sec * NANOS_IN_SEC + (uint64_t)ts.tv_nsec;
	}

	return 0;
}
#endif

void xdebug_nanotime_init(void)
{
	xdebug_nanotime_context context = {0};

#if PHP_WIN32
	LARGE_INTEGER tcounter;

	if (IsWindows8OrGreater()) {
		context.win_precise_time_func = (WIN_PRECISE_TIME_FUNC)GetProcAddress(
			GetModuleHandle(TEXT("kernel32.dll")),
			"GetSystemTimePreciseAsFileTime"
		);
	} else {
		context.win_precise_time_func = NULL;
		QueryPerformanceFrequency(&tcounter);
		context.win_freq = (uint64_t)tcounter.QuadPart;
		context.use_rel_time = 1;
	}

#elif __APPLE__ | defined(CLOCK_MONOTONIC)
	context.use_rel_time = 1;
#endif

	context.start_abs = xdebug_get_nanotime_abs(&context);
	context.last_abs = 0;
#if PHP_WIN32 | __APPLE__ | defined(CLOCK_MONOTONIC)
	context.start_rel = xdebug_get_nanotime_rel(&context);
	context.last_rel = 0;
#endif

	XG_BASE(nanotime_context) = context;
}

uint64_t xdebug_get_nanotime(void)
{
	uint64_t nanotime;
	xdebug_nanotime_context *context;

	context = &XG_BASE(nanotime_context);

#if PHP_WIN32 | __APPLE__ | defined(CLOCK_MONOTONIC)
	/* Relative timing */
	if (context->use_rel_time) {
		nanotime = xdebug_get_nanotime_rel(context);

		if (nanotime < context->last_rel + NANOTIME_MIN_STEP) {
			context->last_rel += NANOTIME_MIN_STEP;
			nanotime = context->last_rel;
		}
		context->last_rel = nanotime;
		nanotime = context->start_abs + (nanotime - context->start_rel);

		return nanotime;
	}
#endif

	/* Absolute timing */
	nanotime = xdebug_get_nanotime_abs(context);

	if (nanotime < context->last_abs + NANOTIME_MIN_STEP) {
		context->last_abs += NANOTIME_MIN_STEP;
		nanotime = context->last_abs;
	}
	context->last_abs = nanotime;

	return nanotime;
}

char* xdebug_nanotime_to_chars(uint64_t nanotime, unsigned char precision)
{
	char *res;
	time_t secs;

	secs = (time_t)(nanotime / NANOS_IN_SEC);
	if (precision > 0) {
		res = xdmalloc(30);
	} else {
		res = xdmalloc(20);
	}
	strftime(res, 20, "%Y-%m-%d %H:%M:%S", gmtime(&secs));
	if (precision > 0) {
		sprintf(res + 19, ".%09u", (uint32_t)(nanotime % NANOS_IN_SEC));
		if (precision < 9) {
			*(res + 20 + precision) = '\0';
		}
	}
	return res;
}
