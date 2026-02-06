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

/*
 * FrankenPHP Worker Mode Support
 *
 * In worker mode, RINIT/RSHUTDOWN are only called once per worker, not per
 * request. This file provides SAPI hooks to manage debug connection lifecycle.
 */

#include "lib/php-header.h"
#include "SAPI.h"
#include "php_xdebug.h"
#include "debugger.h"
#include "com.h"
#include "frankenphp.h"

#include <string.h>

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static int (*original_sapi_activate)(void) = NULL;
static int (*original_sapi_deactivate)(void) = NULL;
static int is_frankenphp = 0;

/* Check for XDEBUG_SESSION or XDEBUG_TRIGGER in a key=value string */
static int has_trigger_in_string(const char *str, char delim)
{
	const char *triggers[] = {"XDEBUG_SESSION", "XDEBUG_TRIGGER", NULL};

	if (!str) return 0;

	for (const char **t = triggers; *t; t++) {
		size_t len = strlen(*t);
		const char *p = str;
		while ((p = strstr(p, *t))) {
			/* Match only if at start of key and followed by '=' */
			if ((p == str || p[-1] == delim || p[-1] == ' ') && p[len] == '=') {
				return 1;
			}
			p += len;
		}
	}
	return 0;
}

/* Check for debug triggers in raw request data (before superglobals are set) */
static int has_debug_trigger(void)
{
	return has_trigger_in_string(SG(request_info).cookie_data, ';') ||
	       has_trigger_in_string(SG(request_info).query_string, '&');
}

/* Called at start of each worker request */
static int frankenphp_sapi_activate(void)
{
	int result = original_sapi_activate ? original_sapi_activate() : SUCCESS;

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		/* Reset per-request debugger state */
		XG_DBG(detached) = 0;
		XG_DBG(no_exec) = 0;
		XG_DBG(breakpoints_allowed) = 1;
		XG_DBG(context).do_break = 0;
		XG_DBG(context).do_step = 0;
		XG_DBG(context).do_next = 0;
		XG_DBG(context).do_finish = 0;
		XG_DBG(context).do_connect_to_client = 0;

		/* Connect to debugger if trigger is present */
		if (has_debug_trigger()) {
			XG_DBG(context).do_connect_to_client = 1;
		}
	}
	return result;
}

/* Called at end of each worker request */
static int frankenphp_sapi_deactivate(void)
{
	/* Close debug connection so next request gets a fresh session */
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG) && xdebug_is_debug_connection_active()) {
		XG_DBG(context).handler->remote_deinit(&(XG_DBG(context)));
		xdebug_mark_debug_connection_not_active();
	}
	return original_sapi_deactivate ? original_sapi_deactivate() : SUCCESS;
}

/* Called from xdebug_debugger_minit() */
void xdebug_frankenphp_minit(void)
{
	if (strcmp(sapi_module.name, "frankenphp") == 0) {
		is_frankenphp = 1;
		original_sapi_activate = sapi_module.activate;
		sapi_module.activate = frankenphp_sapi_activate;
		original_sapi_deactivate = sapi_module.deactivate;
		sapi_module.deactivate = frankenphp_sapi_deactivate;
	}
}

void xdebug_frankenphp_mshutdown(void)
{
	if (is_frankenphp) {
		sapi_module.activate = original_sapi_activate;
		sapi_module.deactivate = original_sapi_deactivate;
		original_sapi_activate = NULL;
		original_sapi_deactivate = NULL;
		is_frankenphp = 0;
	}
}
