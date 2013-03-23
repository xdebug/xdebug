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

#ifndef __HAVE_XDEBUG_HANDLER_DBGP_H__
#define __HAVE_XDEBUG_HANDLER_DBGP_H__

#include <string.h>
#include "xdebug_handlers.h"
#include "xdebug_xml.h"

#define DBGP_VERSION "1.0"

typedef struct xdebug_dbgp_result {
	int status;
	int reason;
	int code;
} xdebug_dbgp_result;

#define ADD_REASON_MESSAGE(c) { \
	xdebug_xml_node *message = xdebug_xml_node_init("message"); \
	xdebug_error_entry *error_entry = &xdebug_error_codes[0]; \
	\
	while (error_entry->message) { \
		if ((c) == error_entry->code) { \
			xdebug_xml_add_text(message, xdstrdup(error_entry->message)); \
			xdebug_xml_add_child(error, message); \
		} \
		error_entry++; \
	} \
}

#define RETURN_RESULT(s, r, c) { \
	xdebug_xml_node *error = xdebug_xml_node_init("error"); \
	xdebug_xml_node *message = xdebug_xml_node_init("message"); \
	xdebug_error_entry *error_entry = &xdebug_error_codes[0]; \
	\
	xdebug_xml_add_attribute(*retval, "status", xdebug_dbgp_status_strings[(s)]); \
	xdebug_xml_add_attribute(*retval, "reason", xdebug_dbgp_reason_strings[(r)]); \
	xdebug_xml_add_attribute_ex(error, "code", xdebug_sprintf("%u", (c)), 0, 1); \
	\
	while (error_entry->message) { \
		if ((c) == error_entry->code) { \
			xdebug_xml_add_text(message, xdstrdup(error_entry->message)); \
			xdebug_xml_add_child(error, message); \
		} \
		error_entry++; \
	} \
	xdebug_xml_add_child(*retval, error); \
	return; \
}

/* Argument structure */
typedef struct xdebug_dbgp_arg {
	char *value[27]; /* one extra for - */
} xdebug_dbgp_arg;

#define DBGP_FUNC_PARAMETERS        xdebug_xml_node **retval, xdebug_con *context, xdebug_dbgp_arg *args TSRMLS_DC
#define DBGP_FUNC_PASS_PARAMETERS   retval, context, args TSRMLS_CC
#define DBGP_FUNC(name)             static void xdebug_dbgp_handle_##name(DBGP_FUNC_PARAMETERS)
#define DBGP_FUNC_ENTRY(name,flags)       { #name, xdebug_dbgp_handle_##name, 0, flags },
#define DBGP_CONT_FUNC_ENTRY(name,flags)  { #name, xdebug_dbgp_handle_##name, 1, flags },
#define DBGP_STOP_FUNC_ENTRY(name,flags)  { #name, xdebug_dbgp_handle_##name, 2, flags },

#define XDEBUG_DBGP_NONE          0x00
#define XDEBUG_DBGP_POST_MORTEM   0x01

typedef struct xdebug_dbgp_cmd {
	char *name;
	void (*handler)(DBGP_FUNC_PARAMETERS);
	int  cont;
	int  flags;
} xdebug_dbgp_cmd;

#define CMD_OPTION(opt)    (opt == '-' ? args->value[26] : args->value[(opt) - 'a'])

int xdebug_dbgp_init(xdebug_con *context, int mode);
int xdebug_dbgp_deinit(xdebug_con *context);
int xdebug_dbgp_error(xdebug_con *context, int type, char *exception_type, char *message, const char *location, const uint line, xdebug_llist *stack);
int xdebug_dbgp_breakpoint(xdebug_con *context, xdebug_llist *stack, char *file, long lineno, int type, char *exception, char *message);
int xdebug_dbgp_stream_output(const char *string, unsigned int length TSRMLS_DC);
int xdebug_dbgp_register_eval_id(xdebug_con *context, function_stack_entry *fse);
char *xdebug_dbgp_get_revision(void);

#define xdebug_handler_dbgp {       \
	xdebug_dbgp_init,               \
	xdebug_dbgp_deinit,             \
	xdebug_dbgp_error,              \
	xdebug_dbgp_breakpoint,         \
	xdebug_dbgp_stream_output,      \
	xdebug_dbgp_register_eval_id,   \
	xdebug_dbgp_get_revision        \
}

#endif

