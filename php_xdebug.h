/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2021 Derick Rethans                               |
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

#ifndef PHP_XDEBUG_H
#define PHP_XDEBUG_H

#define XDEBUG_NAME       "Xdebug"
#define XDEBUG_VERSION    "3.0.4"
#define XDEBUG_AUTHOR     "Derick Rethans"
#define XDEBUG_COPYRIGHT  "Copyright (c) 2002-2021 by Derick Rethans"
#define XDEBUG_COPYRIGHT_SHORT "Copyright (c) 2002-2021"
#define XDEBUG_URL        "https://xdebug.org"
#define XDEBUG_URL_FAQ    "https://xdebug.org/docs/faq#api"

#include "php.h"

#include "coverage/branch_info.h"
#include "coverage/code_coverage.h"
#include "debugger/debugger.h"
#include "develop/develop.h"
#include "lib/lib.h"
#include "gcstats/gc_stats.h"
#include "profiler/profiler.h"
#include "tracing/tracing.h"
#include "lib/compat.h"
#include "lib/hash.h"
#include "lib/llist.h"
#include "lib/vector.h"
#include "lib/timing.h"

extern zend_module_entry xdebug_module_entry;
#define phpext_xdebug_ptr &xdebug_module_entry

#define OUTPUT_NOT_CHECKED -1
#define OUTPUT_IS_TTY       1
#define OUTPUT_NOT_TTY      0

#ifdef PHP_WIN32
#define PHP_XDEBUG_API __declspec(dllexport)
#else
#define PHP_XDEBUG_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include "main/SAPI.h"

#define XDEBUG_ALLOWED_HALT_LEVELS (E_WARNING | E_NOTICE | E_USER_WARNING | E_USER_NOTICE )

PHP_MINIT_FUNCTION(xdebug);
PHP_MSHUTDOWN_FUNCTION(xdebug);
PHP_RINIT_FUNCTION(xdebug);
PHP_RSHUTDOWN_FUNCTION(xdebug);
PHP_MINFO_FUNCTION(xdebug);
ZEND_MODULE_POST_ZEND_DEACTIVATE_D(xdebug);

int xdebug_is_output_tty();

struct xdebug_base_info {
	unsigned long level;
	xdebug_vector *stack;
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

	/* filters */
	zend_long     filter_type_code_coverage;
	zend_long     filter_type_stack;
	zend_long     filter_type_tracing;
	xdebug_llist *filters_code_coverage;
	xdebug_llist *filters_stack;
	xdebug_llist *filters_tracing;

	struct {
		zend_long     max_nesting_level;
	} settings;
};

ZEND_BEGIN_MODULE_GLOBALS(xdebug)
	struct xdebug_base_info     base;
	struct {
		xdebug_coverage_globals_t coverage;
		xdebug_debugger_globals_t debugger;
		xdebug_develop_globals_t  develop;
		xdebug_gc_stats_globals_t gc_stats;
		xdebug_library_globals_t  library;
		xdebug_profiler_globals_t profiler;
		xdebug_tracing_globals_t  tracing;
	} globals;
	struct {
		xdebug_coverage_settings_t coverage;
		xdebug_debugger_settings_t debugger;
		xdebug_develop_settings_t  develop;
		xdebug_gc_stats_settings_t gc_stats;
		xdebug_library_settings_t  library;
		xdebug_profiler_settings_t profiler;
		xdebug_tracing_settings_t  tracing;
	} settings;
ZEND_END_MODULE_GLOBALS(xdebug)

#ifdef ZTS
#define XG(v) TSRMG(xdebug_globals_id, zend_xdebug_globals *, v)
#else
#define XG(v) (xdebug_globals.v)
#endif

#define XG_BASE(v)     (XG(base.v))
#define XINI_BASE(v)     (XG(base.settings.v))

#endif
