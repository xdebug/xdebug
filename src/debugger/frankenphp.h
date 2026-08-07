/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2026 Derick Rethans                               |
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
 * FrankenPHP worker mode support.
 *
 * In FrankenPHP worker mode, MINIT/RINIT/RSHUTDOWN/MSHUTDOWN run once per
 * worker script, not per request. Per-request setup goes through
 * sapi_module.activate / sapi_module.deactivate instead. We hook those to
 * drive a per-request debugger lifecycle inside the worker loop: each
 * request re-runs the normal activation checks (triggers, shared secret,
 * xdebug.start_with_request) and closes its debug session on completion.
 *
 * If the SAPI is not "frankenphp", these functions are no-ops.
 */

bool xdebug_sapi_is_frankenphp(void);
void xdebug_frankenphp_minit(void);
void xdebug_frankenphp_mshutdown(void);
void xdebug_frankenphp_request_reinit(void);

/* Cheap check for the per-function-call hot path in
 * xdebug_execute_user_code_begin(): non-zero only when the FrankenPHP
 * activate hook armed a re-initialisation for the current worker request.
 * The flag is only ever set by the FrankenPHP SAPI hooks, and it is cleared
 * by RINIT, so it can only be seen armed inside a worker loop. */
#define xdebug_frankenphp_reinit_pending() (XG(globals.debugger.context.do_request_reinit))

#endif /* __XDEBUG_DEBUGGER_FRANKENPHP_H__ */
