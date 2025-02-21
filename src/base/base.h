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
 */

#ifndef __XDEBUG_BASE_H__
#define __XDEBUG_BASE_H__

#include "zend_extensions.h"

void xdebug_base_minit(INIT_FUNC_ARGS);
void xdebug_base_mshutdown();

void xdebug_base_post_startup();

void xdebug_base_rinit();
void xdebug_base_post_deactivate();
void xdebug_base_rshutdown();

void xdebug_func_dtor_by_ref(xdebug_func *elem); /* TODO: Remove this API */
void xdebug_func_dtor(xdebug_func *elem);

void xdebug_build_fname(xdebug_func *tmp, zend_execute_data *edata);

void xdebug_print_info(void);

void xdebug_enable_debugger_if_disabled();
void xdebug_disable_debugger_if_enabled();
void xdebug_enable_debugger_and_rebuild_stack_if_disabled();
void xdebug_rebuild_stack_if_disabled();
void xdebug_save_statement_handler(zend_extension *extension, statement_handler_func_t statement_handler);

#endif // __XDEBUG_BASE_H__
