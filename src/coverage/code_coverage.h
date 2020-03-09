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
   +----------------------------------------------------------------------+
 */

#ifndef __XDEBUG_CODE_COVERAGE_H__
#define __XDEBUG_CODE_COVERAGE_H__

#include "lib/lib.h"

typedef struct xdebug_coverage_file {
	char               *name;
	xdebug_hash        *lines;
	xdebug_hash        *functions; /* Used for branch coverage */
	int                 has_branch_info;
} xdebug_coverage_file;

typedef struct _xdebug_coverage_globals_t {
	zend_bool     code_coverage_active; /* Whether code coverage is currently running */
	xdebug_hash  *code_coverage_info;   /* Stores code coverage information */
	zend_bool     code_coverage_unused;
	zend_bool     code_coverage_dead_code_analysis;
	zend_bool     code_coverage_branch_check;
	int           dead_code_analysis_tracker_offset;
	long          dead_code_last_start_id;
	long          code_coverage_filter_offset;
	size_t        prefill_function_count;
	size_t        prefill_class_count;
	char                 *previous_filename;
	xdebug_coverage_file *previous_file;
	char                 *previous_mark_filename;
	xdebug_coverage_file *previous_mark_file;
	xdebug_path_info     *paths_stack;
	xdebug_hash          *visited_branches;
	struct {
		unsigned int  size;
		int *last_branch_nr;
	} branches;
} xdebug_coverage_globals_t;

typedef struct _xdebug_coverage_settings_t {
	zend_bool     enable; /* Flag to enable code coverage (and opcode overloading) */
} xdebug_coverage_settings_t;

void xdebug_init_coverage_globals(xdebug_coverage_globals_t *xg);
void xdebug_coverage_count_line_if_active(zend_op_array *op_array, char *file, int lineno);
void xdebug_coverage_count_line_if_branch_check_active(zend_op_array *op_array, char *file, int lineno);
void xdebug_coverage_record_assign_if_active(zend_execute_data *execute_data, zend_op_array *op_array, int do_cc);
void xdebug_coverage_record_include_if_active(zend_execute_data *execute_data, zend_op_array *op_array);
void xdebug_coverage_record_silence_if_active(zend_execute_data *execute_data, zend_op_array *op_array);
void xdebug_coverage_compile_file(zend_op_array *op_array);

int  xdebug_coverage_execute_ex(function_stack_entry *fse, zend_op_array *op_array, char **tmp_file_name, char **tmp_function_name);
void xdebug_coverage_execute_ex_end(function_stack_entry *fse, zend_op_array *op_array, char *tmp_file_name, char *tmp_function_name);
void xdebug_coverage_init_oparray(zend_op_array *op_array);

void xdebug_coverage_minit(INIT_FUNC_ARGS);
void xdebug_coverage_mshutdown(void);
void xdebug_coverage_rinit(void);
void xdebug_coverage_post_deactivate(void);

PHP_FUNCTION(xdebug_start_code_coverage);
PHP_FUNCTION(xdebug_stop_code_coverage);
PHP_FUNCTION(xdebug_get_code_coverage);
PHP_FUNCTION(xdebug_code_coverage_started);

PHP_FUNCTION(xdebug_get_function_count);

#endif
