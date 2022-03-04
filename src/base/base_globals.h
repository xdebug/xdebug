/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2022 Derick Rethans                               |
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

#ifndef __XDEBUG_BASE_GLOBALS_H__
#define __XDEBUG_BASE_GLOBALS_H__

#include "lib/hash.h"
#include "lib/llist.h"
#include "lib/vector.h"


#if PHP_WIN32
typedef void (WINAPI *WIN_PRECISE_TIME_FUNC)(LPFILETIME);
#endif

typedef struct _xdebug_nanotime_context {
	uint64_t start_abs;
	uint64_t last_abs;
#if PHP_WIN32 | HAVE_XDEBUG_CLOCK_GETTIME | HAVE_XDEBUG_CLOCK_GETTIME_NSEC_NP
	uint64_t start_rel;
	uint64_t last_rel;
	int      use_rel_time;
#endif
#if PHP_WIN32
	WIN_PRECISE_TIME_FUNC win_precise_time_func;
	uint64_t win_freq;
#endif
} xdebug_nanotime_context;


typedef struct _xdebug_base_globals_t {
	xdebug_vector *stack;
#if PHP_VERSION_ID >= 80100
	xdebug_hash   *fiber_stacks;
#endif
	xdebug_nanotime_context nanotime_context;
	uint64_t      start_nanotime;
	unsigned int  prev_memory;
	zif_handler   orig_set_time_limit_func;
	zif_handler   orig_error_reporting_func;
	zif_handler   orig_pcntl_exec_func;
	zif_handler   orig_pcntl_fork_func;
	int           output_is_tty;
	zend_bool     in_debug_info;
	zend_long     error_reporting_override;
	zend_bool     error_reporting_overridden;
	unsigned int  function_count;
	zend_string  *last_eval_statement;
	char         *last_exception_trace;

	/* in-execution checking */
	zend_bool  in_execution;
	zend_bool  in_var_serialisation;

	/* Systemd Private Temp */
	char         *private_tmp;

	/* filters */
	zend_long     filter_type_code_coverage;
	zend_long     filter_type_stack;
	zend_long     filter_type_tracing;
	xdebug_llist *filters_code_coverage;
	xdebug_llist *filters_stack;
	xdebug_llist *filters_tracing;

	/* PHP versions */
	const char   *php_version_compile_time;
	const char   *php_version_run_time;
} xdebug_base_globals_t;

typedef struct _xdebug_base_settings_t {
	zend_long     max_nesting_level;
} xdebug_base_settings_t;

#endif // __XDEBUG_BASE_GLOBALS_H__
