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
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   +----------------------------------------------------------------------+
 */

#ifndef __HAVE_XDEBUG_HANDLER_PHP3_H__
#define __HAVE_XDEBUG_HANDLER_PHP3_H__

#include "xdebug_handlers.h"


int xdebug_php3_init(xdebug_con *context, int mode, char *magic_cookie);
int xdebug_php3_deinit(xdebug_con *context);
int xdebug_php3_error(xdebug_con *context, int type, char *exception_type, char *message, const char *location, const uint line, xdebug_llist *stack);
char *xdebug_php3_get_revision(void);

#define xdebug_handler_php3 { \
	xdebug_php3_init,         \
	xdebug_php3_deinit,       \
	xdebug_php3_error,        \
	NULL,                     \
	xdebug_php3_get_revision  \
}

#endif

