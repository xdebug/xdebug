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

#ifndef __XDEBUG_PROFILER_H__
#define __XDEBUG_PROFILER_H__

#include "php.h"
#include "TSRM.h"
#include "php_xdebug.h"
#include "xdebug_private.h"

int xdebug_profiler_init(char *script_name TSRMLS_DC);
void xdebug_profiler_deinit(TSRMLS_D);
int xdebug_profiler_output_aggr_data(const char *prefix TSRMLS_DC);

void xdebug_profiler_add_function_details_user(function_stack_entry *fse, zend_op_array *op_array TSRMLS_DC);
void xdebug_profiler_add_function_details_internal(function_stack_entry *fse TSRMLS_DC);
void xdebug_profiler_free_function_details(function_stack_entry *fse TSRMLS_DC);

void xdebug_profiler_function_begin(function_stack_entry *fse TSRMLS_DC);
void xdebug_profiler_function_end(function_stack_entry *fse TSRMLS_DC);

void xdebug_profile_call_entry_dtor(void *dummy, void *elem);
void xdebug_profile_aggr_call_entry_dtor(void *elem);

#endif
