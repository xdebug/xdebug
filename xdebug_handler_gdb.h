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

#ifndef __HAVE_XDEBUG_HANDLER_GDB_H__
#define __HAVE_XDEBUG_HANDLER_GDB_H__

#include "xdebug_handlers.h"
#include <string.h>

#define XDEBUG_INIT         1
#define XDEBUG_BREAKPOINT   2
#define XDEBUG_RUN          4
#define XDEBUG_DATA         8
#define XDEBUG_STATUS      16

typedef struct xdebug_arg {
	int    c;
	char **args;
} xdebug_arg;

typedef struct xdebug_cmd {
	char *name;
	int   args;
	char *description;
	char *(*handler)(xdebug_con *context, xdebug_arg *args);
} xdebug_cmd;

int xdebug_gdb_init(xdebug_con *context, int mode);
int xdebug_gdb_deinit(xdebug_con *context);
int xdebug_gdb_error(xdebug_con *context, int type, char *message, const char *location, const uint line, xdebug_llist *stack);
int xdebug_gdb_breakpoint(xdebug_con *context, xdebug_llist *stack);

#define xdebug_handler_gdb { \
	xdebug_gdb_init,         \
	xdebug_gdb_deinit,       \
	xdebug_gdb_error,        \
	xdebug_gdb_breakpoint    \
}

#endif

