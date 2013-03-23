/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
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

#ifndef __HAVE_XDEBUG_HANDLERS_H__
#define __HAVE_XDEBUG_HANDLERS_H__

#include "php_xdebug.h"
#include "xdebug_com.h"
#include "xdebug_llist.h"
#include "xdebug_hash.h"
#include "xdebug_private.h"
#include "usefulstuff.h"

typedef struct _xdebug_brk_admin            xdebug_brk_admin;
typedef struct _xdebug_brk_info             xdebug_brk_info;
typedef struct _xdebug_eval_info            xdebug_eval_info;
typedef struct _xdebug_con                  xdebug_con;
typedef struct _xdebug_debug_list           xdebug_debug_list;
typedef struct _xdebug_remote_handler       xdebug_remote_handler;
typedef struct _xdebug_remote_handler_info  xdebug_remote_handler_info;

struct _xdebug_debug_list {
	char *last_file;
	int   last_line;
};

#define BREAKPOINT_TYPE_LINE      1
#define BREAKPOINT_TYPE_FUNCTION  2
#define BREAKPOINT_TYPE_EXCEPTION 4

struct _xdebug_brk_admin {
	int   id;
	int   type;
	char *key;
};

struct _xdebug_con {
	int                    socket;
	void                  *options;
	xdebug_remote_handler *handler;
	fd_buf                *buffer;
	char                  *program_name;
	xdebug_hash           *breakpoint_list;
	xdebug_hash           *function_breakpoints;
	xdebug_hash           *eval_id_lookup;
	int                    eval_id_sequence;
	xdebug_llist          *line_breakpoints;
	xdebug_hash           *exception_breakpoints;
	xdebug_debug_list      list;
	int                    do_break;

	int                    do_step;
	int                    do_next;
	int                    do_finish;
	int                    next_level;
};

#define XDEBUG_HIT_DISABLED       0
#define XDEBUG_HIT_GREATER_EQUAL  1
#define XDEBUG_HIT_EQUAL          2
#define XDEBUG_HIT_MOD            3

#define XDEBUG_BRK_FUNC_CALL      1
#define XDEBUG_BRK_FUNC_RETURN    2

struct _xdebug_brk_info {
	char                 *type;
	char                 *classname;
	char                 *functionname;
	char                 *exceptionname;
	int                   function_break_type; /* XDEBUG_BRK_FUNC_* */
	char                 *file;
	int                   file_len;
	int                   lineno;
	char                 *condition;
	int                   disabled;
	int                   temporary;
	int                   hit_count;
	int                   hit_value;
	int                   hit_condition;
};

struct _xdebug_eval_info {
	int   id;
	int   refcount;
	char *contents;
};

struct _xdebug_remote_handler {
	/* Init / deinit */
	int (*remote_init)(xdebug_con *h, int mode);
	int (*remote_deinit)(xdebug_con *h);

	/* Stack messages */
	int (*remote_error)(xdebug_con *h, int type, char *exception_type, char *message, const char *location, const uint line, xdebug_llist *stack);

	/* Breakpoints */
	int (*remote_breakpoint)(xdebug_con *h, xdebug_llist *stack, char *file, long lineno, int type, char *exception, char *message);

	/* Output redirection */
	int (*remote_stream_output)(const char *string, unsigned int length TSRMLS_DC);

	/* Eval ID registration and removal */
	int (*register_eval_id)(xdebug_con *h, function_stack_entry *fse);

	/* Information */
	char *(*get_revision)(void);
};

struct _xdebug_remote_handler_info {
	char                  *name;
	char                  *description;
	xdebug_remote_handler  handler;
};

xdebug_remote_handler* xdebug_handler_get(char* mode);
xdebug_remote_handler_info* xdebug_handlers_get(void);

void xdebug_brk_info_dtor(xdebug_brk_info *brk);
void xdebug_llist_brk_dtor(void *dummy, xdebug_brk_info *brk);
void xdebug_hash_brk_dtor(xdebug_brk_info *brk);
void xdebug_hash_eval_info_dtor(xdebug_eval_info *ei);

#endif
