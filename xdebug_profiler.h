/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000, 2001 The PHP Group             |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Ilia Alshanetsky <ilia@prohost.org>                         |
   +----------------------------------------------------------------------+
 */

#include "php.h"
#include "TSRM.h"
#include "php_xdebug.h"

PHP_FUNCTION(xdebug_start_profiling);
PHP_FUNCTION(xdebug_stop_profiling);
PHP_FUNCTION(xdebug_dump_function_profile);
PHP_FUNCTION(xdebug_get_function_profile);

#define XDEBUG_PROFILER_LBL    0   /* line by line */
#define XDEBUG_PROFILER_CPU    1   /* sorted by execution time */
#define XDEBUG_PROFILER_NC     2   /* number of function calls */
#define XDEBUG_PROFILER_FS_AV  3   /* sorted by avg. exection time */
#define XDEBUG_PROFILER_FS_SUM 4   /* sorted by total time taken by each function */
#define XDEBUG_PROFILER_FS_NC  5   /* sorted by total number of function calls */
#define XDEBUG_PROFILER_SD_LBL 6   /* hierarchical view of the functions, sorted by line numbers */
#define XDEBUG_PROFILER_SD_CPU 7   /* hierarchical view of the functions, sorted by cpu usage */
#define XDEBUG_PROFILER_SD_NC  8   /* hierarchical view of the functions, sorted by function calls */
#define XDEBUG_PROFILER_FC_SUM 9   /* hierarchical view of the functions, sorted by function calls */

#define XDEBUG_PROFILER_MODES  10

#define XDEBUG_PROFILER_LBL_D    "Execution Time Profile (sorted by line numbers)"
#define XDEBUG_PROFILER_CPU_D    "Execution Time Profile (sorted by execution time)"
#define XDEBUG_PROFILER_NC_D     "Execution Time Profile (sorted by number of calls to each function)"
#define XDEBUG_PROFILER_FS_AV_D  "Function Summary Profile (sorted by avg. execution time)"
#define XDEBUG_PROFILER_FS_SUM_D "Function Summary Profile (sorted by total execution time)"
#define XDEBUG_PROFILER_FS_NC_D  "Function Summary Profile (sorted by number of function calls)"
#define XDEBUG_PROFILER_SD_LBL_D "Stack-Dump Profile (sorted by line numbers)"
#define XDEBUG_PROFILER_SD_CPU_D "Stack-Dump Profile (sorted by execution time)"
#define XDEBUG_PROFILER_SD_NC_D  "Stack-Dump Profile (sorted by number of calls to each function)"
#define XDEBUG_PROFILER_FC_SUM_D "Function Execution Profile"

typedef struct xdebug_tree_p {
	int n_func;
	function_stack_entry **subf;
} xdebug_tree_p;

typedef struct xdebug_tree_out {
	int nelem;
	int nelem_p;
	int pos;
	double time;
	struct xdebug_tree_out *parent; 
	struct function_stack_entry *fse;
	struct xdebug_tree_out **children;
} xdebug_tree_out;

typedef struct xdebug_fs {
	int nelem_c;
	int nelem_p;
	double time;
	struct function_stack_entry *fse;
	struct function_stack_entry **children;
	struct function_stack_entry **parents;
} xdebug_fs;

/* need to be export, since these are used by the xdebug.c code */
void print_profile(int html, int mode TSRMLS_DC);
inline double get_mtimestamp();
