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

#ifndef __HAVE_XDEBUG_HANDLER_GDB_H__
#define __HAVE_XDEBUG_HANDLER_GDB_H__

#include <string.h>
#include "xdebug_handlers.h"
#include "xdebug_mm.h"

#define XDEBUG_INIT         1
#define XDEBUG_BREAKPOINT   2
#define XDEBUG_RUN          4
#define XDEBUG_RUNTIME      8
#define XDEBUG_DATA        16
#define XDEBUG_STATUS      32

#define XDEBUG_ALL         63

typedef struct xdebug_gdb_cmd {
	char *name;
	int   args;
	char *description;
	char *(*handler)(xdebug_con *context, xdebug_arg *args);
	int   show;
	char *help;
} xdebug_gdb_cmd;


#define XDEBUG_D                         0
#define XDEBUG_D_BREAKPOINT_SET          XDEBUG_D |    1
#define XDEBUG_D_BREAKPOINT_REMOVED      XDEBUG_D |    2

#define XDEBUG_E                         1024
#define XDEBUG_E_INVALID_FORMAT          XDEBUG_E |    1
#define XDEBUG_E_BREAKPOINT_NOT_SET      XDEBUG_E |    2
#define XDEBUG_E_BREAKPOINT_NOT_REMOVED  XDEBUG_E |    3
#define XDEBUG_E_EVAL                    XDEBUG_E |    4
#define XDEBUG_E_TOO_MANY_ARGUMENTS      XDEBUG_E |    5
#define XDEBUG_E_NO_INFO                 XDEBUG_E |    6
#define XDEBUG_E_UNDEFINED_COMMAND       XDEBUG_E |    7
#define XDEBUG_E_SYMBOL_NOT_FOUND        XDEBUG_E |    8
#define XDEBUG_E_NOT_USER_DEFINED        XDEBUG_E |    9
#define XDEBUG_E_UNKNOWN_OPTION          XDEBUG_E |   10

#define XDEBUG_RESPONSE_NORMAL   0
#define XDEBUG_RESPONSE_XML      1

#define XDEBUG_FRAME_NORMAL      0
#define XDEBUG_FRAME_FULL        1

typedef struct xdebug_gdb_options {
	int response_format;
	int dump_superglobals;
} xdebug_gdb_options;


int xdebug_gdb_init(xdebug_con *context, int mode, char *magic_cookie);
int xdebug_gdb_deinit(xdebug_con *context);
int xdebug_gdb_error(xdebug_con *context, int type, char *message, const char *file, const uint lineno, xdebug_llist *stack);
int xdebug_gdb_breakpoint(xdebug_con *context, xdebug_llist *stack, char *file, long lineno, int type);
char *xdebug_gdb_get_revision(void);

#define xdebug_handler_gdb { \
	xdebug_gdb_init,         \
	xdebug_gdb_deinit,       \
	xdebug_gdb_error,        \
	xdebug_gdb_breakpoint,   \
	xdebug_gdb_get_revision  \
}

#endif

