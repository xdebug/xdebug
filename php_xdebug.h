/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2023 Derick Rethans                               |
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

#ifndef PHP_XDEBUG_H
#define PHP_XDEBUG_H

#define XDEBUG_NAME       "Xdebug"
#define XDEBUG_VERSION    "3.2.2"
#define XDEBUG_AUTHOR     "Derick Rethans"
#define XDEBUG_COPYRIGHT  "Copyright (c) 2002-2023 by Derick Rethans"
#define XDEBUG_COPYRIGHT_SHORT "Copyright (c) 2002-2023"
#define XDEBUG_URL        "https://xdebug.org"
#define XDEBUG_URL_FAQ    "https://xdebug.org/docs/faq#api"

#include "lib/php-header.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "base/base_globals.h"
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

ZEND_BEGIN_MODULE_GLOBALS(xdebug)
	struct {
		xdebug_base_globals_t     base;
		xdebug_coverage_globals_t coverage;
		xdebug_debugger_globals_t debugger;
		xdebug_develop_globals_t  develop;
		xdebug_gc_stats_globals_t gc_stats;
		xdebug_library_globals_t  library;
		xdebug_profiler_globals_t profiler;
		xdebug_tracing_globals_t  tracing;
	} globals;
	struct {
		xdebug_base_settings_t     base;
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

#define XG_BASE(v)     (XG(globals.base.v))
#define XINI_BASE(v)     (XG(settings.base.v))

#endif
