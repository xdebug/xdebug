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
 */

#ifndef __XDEBUG_CODE_COVERAGE_PRIVATE_H__
#define __XDEBUG_CODE_COVERAGE_PRIVATE_H__

#include "php.h"

#include "branch_info.h"
#include "code_coverage.h"

#include "lib/compat.h"
#include "lib/hash.h"
#include "lib/mm.h"

typedef struct xdebug_coverage_analysis_function {
	char               *name;
	xdebug_branch_info *branch_info;
} xdebug_coverage_analysis_function;

typedef struct xdebug_coverage_runtime_function {
	char                              *name;
	xdebug_coverage_analysis_function *analysis;
	/* TODO: Change this to a set */
	xdebug_hash                       *hit_paths;

	xdebug_set                        *hit_branch;
} xdebug_coverage_runtime_function;

typedef struct xdebug_coverage_file {
	zend_string        *name;
	struct {
//		xdebug_set         *executable;
//		xdebug_set         *dead_code;
		xdebug_mset        *lines;
		xdebug_hash        *functions; /* Used for branch coverage */
	} analysis;
	struct {
		xdebug_mset        *hit_lines;
		xdebug_hash        *functions;
	} runtime;
	int                 has_branch_info;
} xdebug_coverage_file;

#define COV_BIT_HIT    0
#define COV_BIT_EXEC   1
#define COV_BIT_ACTIVE 2

#define XG_COV(v)      (XG(globals.coverage.v))
#define XINI_COV(v)    (XG(settings.coverage.v))

xdebug_branch_info *xdebug_branch_info_create(unsigned int size);

void xdebug_branch_info_update(xdebug_branch_info *branch_info, unsigned int pos, unsigned int lineno, unsigned int outidx, unsigned int jump_pos);
void xdebug_branch_post_process(zend_op_array *opa, xdebug_branch_info *branch_info);
void xdebug_branch_find_paths(xdebug_branch_info *branch_info);

void xdebug_branch_info_dump(zend_op_array *opa, xdebug_branch_info *branch_info);
void xdebug_branch_info_add_branches_and_paths(xdebug_coverage_file *file, char *function_name, xdebug_branch_info *branch_info);
void xdebug_branch_info_free(xdebug_branch_info *branch_info);

xdebug_path *xdebug_path_new(xdebug_path *old_path);
void xdebug_path_free(xdebug_path *path);

xdebug_path_info *xdebug_path_info_ctor(void);
void xdebug_path_info_dtor(xdebug_path_info *path_info);

void xdebug_path_info_add_path_for_level(xdebug_path_info *path_info, xdebug_path *path, unsigned int level);
xdebug_path *xdebug_path_info_get_path_for_level(xdebug_path_info *path_info, unsigned int level);

void xdebug_create_key_for_path(xdebug_path *path, xdebug_str *str);

void xdebug_branch_info_mark_reached(zend_string *filename, char *function_name, zend_op_array *op_array, long opcode_nr);
void xdebug_branch_info_mark_end_of_function_reached(zend_string *filename, char *function_name, char *key, int key_len);

xdebug_coverage_file *xdebug_coverage_file_ctor(zend_string *filename);

xdebug_coverage_analysis_function *xdebug_coverage_analysis_function_ctor(char *function_name);
void xdebug_coverage_analysis_function_dtor(void *data);

xdebug_coverage_runtime_function *xdebug_coverage_runtime_function_ctor(char *function_name, xdebug_coverage_analysis_function *analysis);
void xdebug_coverage_runtime_function_dtor(void *data);

void xdebug_code_coverage_start_of_function(zend_op_array *op_array, char *function_name);
void xdebug_code_coverage_end_of_function(zend_op_array *op_array, zend_string *file_name, char *function_name);

PHP_FUNCTION(xdebug_start_code_coverage);
PHP_FUNCTION(xdebug_stop_code_coverage);
PHP_FUNCTION(xdebug_get_code_coverage);
PHP_FUNCTION(xdebug_code_coverage_started);

PHP_FUNCTION(xdebug_get_function_count);

#endif

