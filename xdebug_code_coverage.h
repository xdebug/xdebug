/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 Derick Rethans                              |
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

void xdebug_coverage_line_dtor(void *data);
void xdebug_coverage_file_dtor(void *data);

void xdebug_count_line(char *file, int lineno, int executable TSRMLS_DC);
void xdebug_prefil_code_coverage(function_stack_entry *fse, zend_op_array *op_array TSRMLS_DC);

PHP_FUNCTION(xdebug_start_code_coverage);
PHP_FUNCTION(xdebug_stop_code_coverage);
PHP_FUNCTION(xdebug_get_code_coverage);

PHP_FUNCTION(xdebug_get_function_count);

#endif
