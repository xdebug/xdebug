/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2021 Derick Rethans                               |
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

#ifndef __XDEBUG_DEBUGGER_PRIVATE_H__
#define __XDEBUG_DEBUGGER_PRIVATE_H__

#include "debugger.h"

#include "lib/lib.h"

typedef struct _fd_buf fd_buf;

struct _fd_buf {
	char *buffer;
	int   buffer_size;
};

typedef struct _xdebug_function_lines_map_item xdebug_function_lines_map_item;

struct _xdebug_function_lines_map_item {
	size_t      line_start;
	size_t      line_end;
	size_t      line_span;
	xdebug_set *lines_breakable;
};

typedef struct _xdebug_lines_list xdebug_lines_list;

struct _xdebug_lines_list {
	size_t count; /* How many function/line mappings are in the list */
	size_t size;  /* How many function/line mappings are allocated */
	xdebug_function_lines_map_item **functions;
};

#define XG_DBG(v)      (XG(globals.debugger.v))
#define XINI_DBG(v)    (XG(settings.debugger.v))

#endif
