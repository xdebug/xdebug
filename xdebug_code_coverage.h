/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 The PHP Group                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <d.rethans@jdimedia.nl>                      |
   +----------------------------------------------------------------------+
 */

#ifndef __HAVE_XDEBUG_CODE_COVERAGE_H__
#define __HAVE_XDEBUG_CODE_COVERAGE_H__

#include "php.h"
#include "xdebug_hash.h"

typedef struct xdebug_coverage_line {
	int lineno;
	int count;
} xdebug_coverage_line;

typedef struct xdebug_coverage_file {
	char        *name;
	xdebug_hash *lines;
} xdebug_coverage_file;

void xdebug_coverage_line_dtor(void *data);
void xdebug_coverage_file_dtor(void *data);

void xdebug_count_line(char *file, int lineno TSRMLS_DC);

PHP_FUNCTION(xdebug_start_code_coverage);
PHP_FUNCTION(xdebug_stop_code_coverage);
PHP_FUNCTION(xdebug_get_code_coverage);


#endif
