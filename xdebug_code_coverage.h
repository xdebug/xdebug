/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
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
#include "xdebug_hash.h"
#include "xdebug_mm.h"

typedef struct xdebug_coverage_line {
	int lineno;
	int count;
	int executable;
} xdebug_coverage_line;

typedef struct xdebug_coverage_file {
	char        *name;
	xdebug_hash *lines;
} xdebug_coverage_file;

/* Needed for code coverage as Zend doesn't always add EXT_STMT when expected */
#define XDEBUG_SET_OPCODE_OVERRIDE_COMMON(oc) \
	zend_set_user_opcode_handler(oc, xdebug_common_override_handler);
#define XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(f,oc) \
	zend_set_user_opcode_handler(oc, xdebug_##f##_handler);


void xdebug_coverage_line_dtor(void *data);
void xdebug_coverage_file_dtor(void *data);

int xdebug_common_override_handler(ZEND_OPCODE_HANDLER_ARGS);

#define XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(f) \
	int xdebug_##f##_handler(ZEND_OPCODE_HANDLER_ARGS)

XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_add);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_sub);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_mul);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_div);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_mod);
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

PHP_FUNCTION(xdebug_get_function_count);

#endif
