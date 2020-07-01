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
   | Authors: Benjamin Eberlei <kontakt@beberlei.de>                      |
   |          Derick Rethans <derick@xdebug.org>                          |
   +----------------------------------------------------------------------+
 */
#include "php.h"
#include "zend_builtin_functions.h"
#include "SAPI.h"
#include "Zend/zend_long.h"

#include "php_xdebug.h"
#include "gc_stats_private.h"

#include "base/base.h"
#include "lib/lib.h"

/* Set correct int format to use */
#if SIZEOF_ZEND_LONG == 4
# define XDEBUG_GCINT_FMT "u"
#else
# define XDEBUG_GCINT_FMT "lu"
#endif

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static void xdebug_gc_stats_print_run(xdebug_gc_run *run);
static void xdebug_gc_stats_run_free(xdebug_gc_run *run);
int (*xdebug_old_gc_collect_cycles)(void);

static int xdebug_gc_collect_cycles(void)
{
	int                ret;
	uint32_t           collected;
	xdebug_gc_run     *run;
	zend_execute_data *execute_data;
	long int           memory;
	double             start;
	xdebug_func        tmp;
#if PHP_VERSION_ID >= 70300
	zend_gc_status     status;
#endif

	if (!XG_GCSTATS(active)) {
		return xdebug_old_gc_collect_cycles();
	}

	execute_data = EG(current_execute_data);

#if PHP_VERSION_ID >= 70300
	zend_gc_get_status(&status);
	collected = status.collected;
#else
	collected = GC_G(collected);
#endif
	start = xdebug_get_utime();
	memory = zend_memory_usage(0);

	ret = xdebug_old_gc_collect_cycles();

	run = xdmalloc(sizeof(xdebug_gc_run));
	run->function_name = NULL;
	run->class_name = NULL;

#if PHP_VERSION_ID >= 70300
	zend_gc_get_status(&status);
	run->collected = status.collected - collected;
#else
	run->collected = GC_G(collected) - collected;
#endif
	run->duration = xdebug_get_utime() - start;
	run->memory_before = memory;
	run->memory_after = zend_memory_usage(0);

	xdebug_build_fname(&tmp, execute_data);

	run->function_name = tmp.function ? xdstrdup(tmp.function) : NULL;
	run->class_name = tmp.class ? xdstrdup(tmp.class) : NULL;

	xdebug_gc_stats_print_run(run);

	xdebug_gc_stats_run_free(run);
	xdebug_func_dtor_by_ref(&tmp);

	return ret;
}

static void xdebug_gc_stats_run_free(xdebug_gc_run *run)
{
	if (run) {
		if (run->function_name) {
			xdfree(run->function_name);
		}
		if (run->class_name) {
			xdfree(run->class_name);
		}
		xdfree(run);
	}
}

static int xdebug_gc_stats_init(char *fname, zend_string *script_name)
{
	char *filename = NULL;

	if (fname && strlen(fname)) {
		filename = xdstrdup(fname);
	} else {
		char *output_dir = xdebug_lib_get_output_dir(); /* not duplicated */

		if (!strlen(XINI_GCSTATS(output_name)) ||
			xdebug_format_output_filename(&fname, XINI_GCSTATS(output_name), ZSTR_VAL(script_name)) <= 0)
		{
			return FAILURE;
		}

		if (IS_SLASH(output_dir[strlen(output_dir) - 1])) {
			filename = xdebug_sprintf("%s%s", output_dir, fname);
		} else {
			filename = xdebug_sprintf("%s%c%s", output_dir, DEFAULT_SLASH, fname);
		}
		xdfree(fname);
	}

	XG_GCSTATS(file) = xdebug_fopen(filename, "w", NULL, &XG_GCSTATS(filename));
	xdfree(filename);

	if (!XG_GCSTATS(file)) {
		return FAILURE;
	}

	fprintf(XG_GCSTATS(file), "Garbage Collection Report\n");
	fprintf(XG_GCSTATS(file), "version: 1\ncreator: xdebug %s (PHP %s)\n\n", XDEBUG_VERSION, PHP_VERSION);

	fprintf(XG_GCSTATS(file), "Collected | Efficiency%% | Duration | Memory Before | Memory After | Reduction%% | Function\n");
	fprintf(XG_GCSTATS(file), "----------+-------------+----------+---------------+--------------+------------+---------\n");

	fflush(XG_GCSTATS(file));

	return SUCCESS;
}

static void xdebug_gc_stats_stop()
{
	XG_GCSTATS(active) = 0;

	if (XG_GCSTATS(file)) {
		fclose(XG_GCSTATS(file));
		XG_GCSTATS(file) = NULL;
	}
}

static void xdebug_gc_stats_print_run(xdebug_gc_run *run)
{
	double reduction;

	if (run->memory_before) {
		reduction = (1 - (float)run->memory_after / (float)run->memory_before) * 100.0;
	} else {
		reduction = 0;
	}

	if (!XG_GCSTATS(file)) {
		return;
	}

	if (!run->function_name) {
		fprintf(XG_GCSTATS(file),
			"%9" XDEBUG_GCINT_FMT " | %9.2f %% | %5.2f ms | %13" XDEBUG_GCINT_FMT " | %12" XDEBUG_GCINT_FMT " | %8.2f %% | -\n",
			run->collected,
			(run->collected / 10000.0) * 100.0,
			run->duration / 1000.0,
			run->memory_before,
			run->memory_after,
			reduction
		);
	} else if (!run->class_name && run->function_name) {
		fprintf(XG_GCSTATS(file),
			"%9" XDEBUG_GCINT_FMT " | %9.2f %% | %5.2f ms | %13" XDEBUG_GCINT_FMT " | %12" XDEBUG_GCINT_FMT " | %8.2f %% | %s\n",
			run->collected,
			(run->collected / 10000.0) * 100.0,
			run->duration / 1000.0,
			run->memory_before,
			run->memory_after,
			reduction,
			run->function_name
		);
	} else if (run->class_name && run->function_name) {
		fprintf(XG_GCSTATS(file),
			"%9" XDEBUG_GCINT_FMT " | %9.2f %% | %5.2f ms | %13" XDEBUG_GCINT_FMT " | %12" XDEBUG_GCINT_FMT " | %8.2f %% | %s::%s\n",
			run->collected,
			(run->collected / 10000.0) * 100.0,
			run->duration / 1000.0,
			run->memory_before,
			run->memory_after,
			reduction,
			run->class_name,
			run->function_name
		);
	}

	fflush(XG_GCSTATS(file));
}

/* {{{ proto void xdebug_get_gcstats_filename()
   Returns the name of the current garbage collection statistics report file */
PHP_FUNCTION(xdebug_get_gcstats_filename)
{
	if (XG_GCSTATS(filename)) {
		RETURN_STRING(XG_GCSTATS(filename));
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto void xdebug_start_gcstats([string $fname])
   Start collecting garbage collection statistics */
PHP_FUNCTION(xdebug_start_gcstats)
{
	char                 *fname = NULL;
	size_t                fname_len = 0;
	function_stack_entry *fse;

	if (XG_GCSTATS(active)) {
		php_error(E_NOTICE, "Garbage Collection statistics are already being collected.");
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|s", &fname, &fname_len) == FAILURE) {
		return;
	}

	fse = xdebug_get_stack_frame(0);

	if (xdebug_gc_stats_init(fname, fse->filename) == SUCCESS) {
		XG_GCSTATS(active) = 1;
		RETVAL_STRING(XG_GCSTATS(filename));
		return;
	} else {
		php_error(E_NOTICE, "Garbage Collection statistics could not be started");
	}

	XG_GCSTATS(active) = 0;
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto void xdebug_stop_gcstats()
   Stop collecting garbage collection statistics */
PHP_FUNCTION(xdebug_stop_gcstats)
{
	if (!XG_GCSTATS(active)) {
		php_error(E_NOTICE, "Garbage Collection statistics was not started");
		RETURN_FALSE;
	}

	xdebug_gc_stats_stop();
	RETURN_STRING(XG_GCSTATS(filename));
}

/* {{{ proto void xdebug_get_gc_run_count()
   Return number of times garbage collection was triggered. */
PHP_FUNCTION(xdebug_get_gc_run_count)
{
#if PHP_VERSION_ID >= 70300
	zend_gc_status status;
#endif

#if PHP_VERSION_ID >= 70300
	zend_gc_get_status(&status);
	RETURN_LONG(status.runs);
#else
    RETURN_LONG(GC_G(gc_runs));
#endif
}

/* {{{ proto void xdebug_get_gc_total_collected_roots()
   Return total number of collected root variables during garbage collection. */
PHP_FUNCTION(xdebug_get_gc_total_collected_roots)
{
#if PHP_VERSION_ID >= 70300
	zend_gc_status status;
#endif

#if PHP_VERSION_ID >= 70300
	zend_gc_get_status(&status);
	RETURN_LONG(status.collected);
#else
    RETURN_LONG(GC_G(collected));
#endif
}

/* {{{ helpers */
void xdebug_gcstats_init_if_requested(zend_op_array* op_array)
{
	RETURN_IF_MODE_IS_NOT(XDEBUG_MODE_GCSTATS);
	if (!xdebug_lib_start_with_request()) {
		return;
	}

	if (XG_GCSTATS(active)) {
		return;
	}

	if (xdebug_gc_stats_init(NULL, op_array->filename) == SUCCESS) {
		XG_GCSTATS(active) = 1;
	}
}

/* }}} */

/* {{{ initialisation */
void xdebug_init_gc_stats_globals(xdebug_gc_stats_globals_t *xg)
{
	xg->file = NULL;
	xg->filename = NULL;
	xg->active = 0;
}

void xdebug_gcstats_minit()
{
	/* Replace garbage collection handler with our own */
	xdebug_old_gc_collect_cycles = gc_collect_cycles;
	gc_collect_cycles = xdebug_gc_collect_cycles;
}

void xdebug_gcstats_mshutdown()
{
	gc_collect_cycles = xdebug_old_gc_collect_cycles;
}

void xdebug_gcstats_rinit()
{
	xdebug_init_gc_stats_globals(&XG(globals).gc_stats);
}

void xdebug_gcstats_post_deactivate()
{
	if (XG_GCSTATS(active)) {
		xdebug_gc_stats_stop();
	}

	if (XG_GCSTATS(filename)) {
		xdfree(XG_GCSTATS(filename));
	}
}
/* }}} */
