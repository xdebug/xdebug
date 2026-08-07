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

#include <string.h>

#include "lib/php-header.h"
#include "SAPI.h"

#include "php_xdebug.h"
#include "com.h"
#include "debugger.h"
#include "frankenphp.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static int (*original_sapi_activate)(void)   = NULL;
static int (*original_sapi_deactivate)(void) = NULL;
static bool hooks_installed = false;

/* Called at the start of each FrankenPHP worker request, at the end of
 * sapi_activate(). Resets per-request debugger state and arms
 * 'do_request_reinit', which xdebug_execute_user_code_begin() consumes on the
 * first user code executed for the new request. The actual activation checks
 * are not done here: at this point FrankenPHP has not yet re-armed the
 * superglobals for the new request. */
static int xdebug_frankenphp_sapi_activate(void)
{
	int result = original_sapi_activate ? original_sapi_activate() : SUCCESS;

	if (!XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		return result;
	}

	/* The full RINIT path only ran once for the worker script; undo the state
	 * that should not leak from one worker request into the next. */
	XG_DBG(detached)            = 0;
	XG_DBG(no_exec)             = 0;
	XG_DBG(breakpoints_allowed) = 1;

	XG_DBG(context).do_break             = 0;
	XG_DBG(context).do_step              = 0;
	XG_DBG(context).do_next              = 0;
	XG_DBG(context).do_finish            = 0;
	XG_DBG(context).do_connect_to_client = 0;

	XG_DBG(context).do_request_reinit    = 1;

	return result;
}

/* Called at the end of each FrankenPHP worker request: tears down an active
 * debug session so that the next request starts a fresh one, just like
 * a regular per-request SAPI does through RSHUTDOWN. */
static int xdebug_frankenphp_sapi_deactivate(void)
{
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		XG_DBG(context).do_request_reinit = 0;

		if (xdebug_is_debug_connection_active()) {
			XG_DBG(context).handler->remote_deinit(&(XG_DBG(context)));
			xdebug_mark_debug_connection_not_active();
		}
	}

	return original_sapi_deactivate ? original_sapi_deactivate() : SUCCESS;
}

/* Called by xdebug_execute_user_code_begin() on the first user code executed
 * for a new worker request, when xdebug_frankenphp_reinit_pending() says a
 * re-initialisation was armed. Runs the same activation checks that
 * xdebug_debug_init_if_requested_at_startup() runs for a regular request:
 * triggers (including xdebug.trigger_value validation), XDEBUG_SESSION_START/
 * XDEBUG_SESSION_STOP handling, and xdebug.start_with_request=yes.
 *
 * The superglobals were re-armed by FrankenPHP for the new request, but are
 * built lazily; force them first so that the trigger detection sees the
 * current request's data (mirrors xdebug_init_auto_globals() in RINIT). */
void xdebug_frankenphp_request_reinit(void)
{
	XG_DBG(context).do_request_reinit = 0;

	zend_is_auto_global_str((char*) ZEND_STRL("_GET"));
	zend_is_auto_global_str((char*) ZEND_STRL("_POST"));
	zend_is_auto_global_str((char*) ZEND_STRL("_COOKIE"));
	zend_is_auto_global_str((char*) ZEND_STRL("_SERVER"));
	zend_is_auto_global_str((char*) ZEND_STRL("_ENV"));

	xdebug_debug_init_if_requested_at_startup();
}

bool xdebug_sapi_is_frankenphp(void)
{
	return sapi_module.name && strcmp(sapi_module.name, "frankenphp") == 0;
}

void xdebug_frankenphp_minit(void)
{
	if (hooks_installed) {
		return;
	}

	hooks_installed = true;

	original_sapi_activate   = sapi_module.activate;
	sapi_module.activate     = xdebug_frankenphp_sapi_activate;

	original_sapi_deactivate = sapi_module.deactivate;
	sapi_module.deactivate   = xdebug_frankenphp_sapi_deactivate;
}

void xdebug_frankenphp_mshutdown(void)
{
	if (!hooks_installed) {
		return;
	}

	sapi_module.activate     = original_sapi_activate;
	sapi_module.deactivate   = original_sapi_deactivate;
	original_sapi_activate   = NULL;
	original_sapi_deactivate = NULL;
	hooks_installed          = false;
}
