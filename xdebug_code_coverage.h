/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2016 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <derick@xdebug.org>                          |
   +----------------------------------------------------------------------+
 */

#ifndef __HAVE_XDEBUG_CODE_COVERAGE_H__
#define __HAVE_XDEBUG_CODE_COVERAGE_H__

#include "php.h"
#include "xdebug_compat.h"
#include "xdebug_hash.h"
#include "xdebug_mm.h"
#include "xdebug_branch_info.h"

typedef struct xdebug_coverage_line {
	int lineno;
	int count;
	int executable;
} xdebug_coverage_line;

typedef struct xdebug_coverage_file {
	char               *name;
	xdebug_hash        *lines;
	xdebug_hash        *functions; /* Used for branch coverage */
	int                 has_branch_info;
} xdebug_coverage_file;

typedef struct xdebug_coverage_function {
	char               *name;
	xdebug_branch_info *branch_info;
} xdebug_coverage_function;

/* Needed for code coverage as Zend doesn't always add EXT_STMT when expected */
#define XDEBUG_SET_OPCODE_OVERRIDE_COMMON(oc) \
	zend_set_user_opcode_handler(oc, xdebug_common_override_handler);
#define XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(f,oc) \
	zend_set_user_opcode_handler(oc, xdebug_##f##_handler);


void xdebug_coverage_line_dtor(void *data);

xdebug_coverage_file *xdebug_coverage_file_ctor(char *filename);
void xdebug_coverage_file_dtor(void *data);

char* xdebug_func_format(xdebug_func *func TSRMLS_DC);
void xdebug_build_fname_from_oparray(xdebug_func *tmp, zend_op_array *opa TSRMLS_DC);

xdebug_coverage_function *xdebug_coverage_function_ctor(char *function_name);
void xdebug_coverage_function_dtor(void *data);
void xdebug_print_opcode_info(char type, zend_execute_data *execute_data, const zend_op *cur_opcode TSRMLS_DC);
void xdebug_code_coverage_start_of_function(zend_op_array *op_array, char *function_name TSRMLS_DC);
void xdebug_code_coverage_end_of_function(zend_op_array *op_array, char *file_name, char *function_name TSRMLS_DC);

int xdebug_check_branch_entry_handler(ZEND_USER_OPCODE_HANDLER_ARGS);
int xdebug_common_override_handler(ZEND_USER_OPCODE_HANDLER_ARGS);

#define XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(f) \
	int xdebug_##f##_handler(ZEND_USER_OPCODE_HANDLER_ARGS)

XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(qm_assign);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_add);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_sub);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_mul);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_div);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_mod);
#if PHP_VERSION_ID >= 50600
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_pow);
#endif
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_sl);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_sr);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(pre_inc);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(post_inc);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(pre_dec);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(post_dec);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(pre_inc_obj);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(post_inc_obj);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(pre_dec_obj);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(post_dec_obj);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_concat);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_bw_or);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_bw_and);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_bw_xor);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_dim);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_obj);

void xdebug_count_line(char *file, int lineno, int executable, int deadcode TSRMLS_DC);
void xdebug_prefill_code_coverage(zend_op_array *op_array TSRMLS_DC);

PHP_FUNCTION(xdebug_start_code_coverage);
PHP_FUNCTION(xdebug_stop_code_coverage);
PHP_FUNCTION(xdebug_get_code_coverage);
PHP_FUNCTION(xdebug_code_coverage_started);

PHP_FUNCTION(xdebug_get_function_count);

#endif
