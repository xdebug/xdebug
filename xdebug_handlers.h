/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 The PHP Group                               |
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

#include "php_xdebug.h"
#include "xdebug_com.h"
#include "xdebug_llist.h"
#include "xdebug_hash.h"
#include "usefulstuff.h"

typedef struct _xdebug_brk_info             xdebug_brk_info;
typedef struct _xdebug_con                  xdebug_con;
typedef struct _xdebug_debug_list           xdebug_debug_list;
typedef struct _xdebug_remote_handler       xdebug_remote_handler;
typedef struct _xdebug_remote_handler_info  xdebug_remote_handler_info;

struct _xdebug_debug_list {
	char *last_file;
	int   last_line;
};

struct _xdebug_con {
	int                    socket;
	void                  *options;
	xdebug_remote_handler *handler;
	fd_buf                *buffer;
	char                  *program_name;
	xdebug_hash           *function_breakpoints;
	xdebug_hash           *class_breakpoints;
	xdebug_llist          *line_breakpoints;
	xdebug_debug_list      list;
	int                    do_break;

	int                    do_step;
	int                    do_next;
	int                    do_finish;
	int                    next_level;
};

struct _xdebug_brk_info {
	char                 *file;
	int                   file_len;
	int                   lineno;
	char                 *condition;
};

struct _xdebug_remote_handler {
	/* Init / deinit */
	int (*remote_init)(xdebug_con *h, int mode);
	int (*remote_deinit)(xdebug_con *h);

	/* Stack messages */
	int (*remote_error)(xdebug_con *h, int type, char *message, const char *location, const uint line, xdebug_llist *stack);

	/* Breakpoints */
	int (*remote_breakpoint)(xdebug_con *h, xdebug_llist *stack, char *file, int lineno, int type);
};

struct _xdebug_remote_handler_info {
	char                  *name;
	xdebug_remote_handler  handler;
};

xdebug_remote_handler* xdebug_handler_get(char* mode);

void xdebug_brk_dtor(void *dummy, xdebug_brk_info *brk);

#endif
