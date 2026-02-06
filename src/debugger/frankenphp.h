/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2024 Derick Rethans                               |
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

#ifndef __XDEBUG_DEBUGGER_FRANKENPHP_H__
#define __XDEBUG_DEBUGGER_FRANKENPHP_H__

/*
 * FrankenPHP Worker Mode Support
 *
 * In FrankenPHP worker mode, RINIT/RSHUTDOWN are only called once per worker,
 * not per request. This module provides hooks to synchronize breakpoints
 * between worker requests using sapi_module.activate callback.
 *
 * When the IDE adds/removes breakpoints while the worker is running, those
 * DBGP commands are sent to the socket but not processed until the next
 * breakpoint is hit. By hooking sapi_module.activate, we can poll for pending
 * commands at the start of each worker request.
 */

void xdebug_frankenphp_minit(void);
void xdebug_frankenphp_mshutdown(void);
void xdebug_debugger_poll_pending_commands(void);

#endif /* __XDEBUG_DEBUGGER_FRANKENPHP_H__ */
