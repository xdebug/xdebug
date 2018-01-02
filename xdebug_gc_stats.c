/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2018 Derick Rethans                               |
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
   +----------------------------------------------------------------------+
 */

#include "php_xdebug.h"
#include "xdebug_gc_stats.h"
#include "xdebug_stack.h"
#include "zend_builtin_functions.h"
#include "SAPI.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static void xdebug_gc_stats_print_run(xdebug_gc_run *run);
static void xdebug_gc_stats_run_free(xdebug_gc_run *run);
int (*xdebug_old_gc_collect_cycles)(void);

int xdebug_gc_collect_cycles(void)
{
	int                ret;
	uint32_t           collected;
	xdebug_gc_run     *run;
	zend_execute_data *execute_data;
	long int           memory;
	double             start;
	xdebug_func        tmp;

	if (!XG(gc_stats_enabled)) {
		return xdebug_old_gc_collect_cycles();
	}

	execute_data = EG(current_execute_data);

	collected = GC_G(collected);
	start = xdebug_get_utime();
	memory = zend_memory_usage(0);

	ret = xdebug_old_gc_collect_cycles();

	run = xdmalloc(sizeof(xdebug_gc_run));
	run->function_name = NULL;
	run->class_name = NULL;

	run->collected = GC_G(collected) - collected;
	run->duration = xdebug_get_utime() - start;
	run->memory_before = memory;
	run->memory_after = zend_memory_usage(0);

	xdebug_build_fname(&tmp, execute_data);

	run->function_name = tmp.function ? xdstrdup(tmp.function) : NULL;
	run->class_name = tmp.class ? xdstrdup(tmp.class) : NULL;

	xdebug_gc_stats_print_run(run);

	xdebug_gc_stats_run_free(run);

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

int xdebug_gc_stats_init(char *fname, char *script_name)
{
	char *filename = NULL;

	if (fname && strlen(fname)) {
		filename = xdstrdup(fname);
	} else {
		if (!strlen(XG(gc_stats_output_name)) ||
			xdebug_format_output_filename(&fname, XG(gc_stats_output_name), script_name) <= 0)
		{
			return FAILURE;
		}

		if (IS_SLASH(XG(gc_stats_output_dir)[strlen(XG(gc_stats_output_dir)) - 1])) {
			filename = xdebug_sprintf("%s%s", XG(gc_stats_output_dir), fname);
		} else {
			filename = xdebug_sprintf("%s%c%s", XG(gc_stats_output_dir), DEFAULT_SLASH, fname);
		}
		xdfree(fname);
	}

	XG(gc_stats_file) = xdebug_fopen(filename, "w", NULL, &XG(gc_stats_filename));
	xdfree(filename);

	if (!XG(gc_stats_file)) {
		return FAILURE;
	}

	fprintf(XG(gc_stats_file), "Garbage Collection Report\n");
	fprintf(XG(gc_stats_file), "version: 1\ncreator: xdebug %s (PHP %s)\n\n", XDEBUG_VERSION, PHP_VERSION);

	fprintf(XG(gc_stats_file), "Collected | Efficiency%% | Duration | Memory Before | Memory After | Reduction%% | Function\n");
	fprintf(XG(gc_stats_file), "----------+-------------+----------+---------------+--------------+------------+---------\n");

	fflush(XG(gc_stats_file));

	return SUCCESS;
}

void xdebug_gc_stats_stop()
{
	XG(gc_stats_enabled) = 0;

	if (XG(gc_stats_file)) {
		fclose(XG(gc_stats_file));
		XG(gc_stats_file) = NULL;
	}
}

static void xdebug_gc_stats_print_run(xdebug_gc_run *run)
{
	double reduction = (1 - (float)run->memory_after / (float)run->memory_before) * 100.0;

	if (!XG(gc_stats_file)) {
		return;
	}

	if (!run->function_name) {
		fprintf(XG(gc_stats_file),
			"%9lu | %9.2f %% | %5.2f ms | %13lu | %12lu | %8.2f %% | -\n",
			run->collected,
			(run->collected / 10000.0) * 100.0,
			run->duration / 1000.0,
			run->memory_before,
			run->memory_after,
			reduction
		);
	} else if (!run->class_name && run->function_name) {
		fprintf(XG(gc_stats_file),
			"%9lu | %9.2f %% | %5.2f ms | %13lu | %12lu | %8.2f %% | %s\n",
			run->collected,
			(run->collected / 10000.0) * 100.0,
			run->duration / 1000.0,
			run->memory_before,
			run->memory_after,
			reduction,
			run->function_name
		);
	} else if (run->class_name && run->function_name) {
		fprintf(XG(gc_stats_file),
			"%9lu | %9.2f %% | %5.2f ms | %13lu | %12lu | %8.2f %% | %s::%s\n",
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

	fflush(XG(gc_stats_file));
}

/* {{{ proto void xdebug_get_gcstats_filename()
   Returns the name of the current garbage collection statistics report file */
PHP_FUNCTION(xdebug_get_gcstats_filename)
{
	if (XG(gc_stats_filename)) {
		RETURN_STRING(XG(gc_stats_filename));
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

	if (XG(gc_stats_enabled) == 0) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &fname, &fname_len) == FAILURE) {
			return;
		}

		fse = xdebug_get_stack_frame(0 TSRMLS_CC);

		if (xdebug_gc_stats_init(fname, fse->filename) == SUCCESS) {
			XG(gc_stats_enabled) = 1;
			RETVAL_STRING(XG(gc_stats_filename));
			return;
		} else {
			php_error(E_NOTICE, "Garbage Collection statistics could not be started");
		}

		XG(gc_stats_enabled) = 0;
		RETURN_FALSE;
	} else {
		php_error(E_NOTICE, "Garbage Collection statistics are already being collected.");
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto void xdebug_stop_gcstats()
   Stop collecting garbage collection statistics */
PHP_FUNCTION(xdebug_stop_gcstats)
{
	if (XG(gc_stats_enabled) == 1) {
		RETVAL_STRING(XG(gc_stats_filename));
		xdebug_gc_stats_stop();
	} else {
		RETVAL_FALSE;
		php_error(E_NOTICE, "Garbage Collection statistics was not started");
	}
}

/* {{{ proto void xdebug_get_gc_run_count()
   Return number of times garbage collection was triggered. */
PHP_FUNCTION(xdebug_get_gc_run_count)
{
    RETURN_LONG(GC_G(gc_runs));
}

/* {{{ proto void xdebug_get_gc_total_collected_roots()
   Return total number of collected root variables during garbage collection. */
PHP_FUNCTION(xdebug_get_gc_total_collected_roots)
{
    RETURN_LONG(GC_G(collected));
}
