/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2022 Derick Rethans                               |
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
#ifndef XDEBUG_FILTER_H
#define XDEBUG_FILTER_H

#include "lib/php-header.h"
#include "php_xdebug.h"

int xdebug_is_stack_frame_filtered(int filter_type, function_stack_entry *fse);
int xdebug_is_top_stack_frame_filtered(int filter_type);
void xdebug_filter_register_constants(INIT_FUNC_ARGS);
void xdebug_filter_run(function_stack_entry *fse);
void xdebug_filter_run_code_coverage(zend_op_array *op_array);
void xdebug_filter_run_internal(function_stack_entry *fse, int group, unsigned char *filtered_flag, int type, xdebug_llist *filters);

#define XDEBUG_FILTER_NONE           0x000
#define XDEBUG_FILTER_CODE_COVERAGE  0x100
#define XDEBUG_FILTER_STACK          0x200
#define XDEBUG_FILTER_TRACING        0x300

#define XDEBUG_PATH_INCLUDE        0x01
#define XDEBUG_PATH_EXCLUDE        0x02
#define XDEBUG_NAMESPACE_INCLUDE   0x11
#define XDEBUG_NAMESPACE_EXCLUDE   0x12

PHP_FUNCTION(xdebug_set_filter);

#endif

