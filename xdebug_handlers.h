/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000, 2001 The PHP Group             |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <d.rethans@jdimedia.nl>                     |
   +----------------------------------------------------------------------+
 */

#ifndef __HAVE_XDEBUG_HANDLERS_H__
#define __HAVE_XDEBUG_HANDLERS_H__

#include "xdebug_llist.h"

typedef struct _xdebug_remote_handler       xdebug_remote_handler;
typedef struct _xdebug_remote_handler_info  xdebug_remote_handler_info;

struct _xdebug_remote_handler {
	/* Init / deinit */
	int (*remote_init)(int socket);
	int (*remote_deinit)(int socket);

	/* Stack messages */
	int (*remote_error)(int socket, int type, char *message, char *location, unsigned int line, xdebug_llist *stack);
};

struct _xdebug_remote_handler_info {
	char                  *name;
	xdebug_remote_handler  handler;
};

xdebug_remote_handler* xdebug_handler_get (char* mode);

#endif
