/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2020 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <derick@xdebug.org>                          |
   +----------------------------------------------------------------------+
 */

#ifndef __HAVE_XDEBUG_HANDLER_DBGP_H__
#define __HAVE_XDEBUG_HANDLER_DBGP_H__

#include <string.h>
#include "handlers.h"
#include "lib/xml.h"

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
	xdebug_str *value[27]; /* one extra for - */
} xdebug_dbgp_arg;

#define DBGP_FUNC_PARAMETERS        xdebug_xml_node **retval, xdebug_con *context, xdebug_dbgp_arg *args
#define DBGP_FUNC_PASS_PARAMETERS   retval, context, args
#define DBGP_FUNC(name)             static void xdebug_dbgp_handle_##name(DBGP_FUNC_PARAMETERS)
#define DBGP_FUNC_ENTRY(name,flags)       { #name, xdebug_dbgp_handle_##name, 0, flags },
#define DBGP_CONT_FUNC_ENTRY(name,flags)  { #name, xdebug_dbgp_handle_##name, 1, flags },
#define DBGP_STOP_FUNC_ENTRY(name,flags)  { #name, xdebug_dbgp_handle_##name, 2, flags },

#define XDEBUG_DBGP_NONE          0x00
#define XDEBUG_DBGP_POST_MORTEM   0x01

typedef struct xdebug_dbgp_cmd {
	const char *name;
	void (*handler)(DBGP_FUNC_PARAMETERS);
	int  cont;
	int  flags;
} xdebug_dbgp_cmd;

typedef struct xdebug_dbgp_resolve_context {
	xdebug_con           *context;
	zend_string          *filename;
	xdebug_lines_list    *lines_list;
} xdebug_dbgp_resolve_context;

#define CMD_OPTION_SET(opt)        (!!(opt == '-' ? args->value[26] : args->value[(opt) - 'a']))
#define CMD_OPTION_CHAR(opt)       (opt == '-' ? args->value[26]->d : args->value[(opt) - 'a']->d)
#define CMD_OPTION_LEN(opt)        (opt == '-' ? args->value[26]->l : args->value[(opt) - 'a']->l)
#define CMD_OPTION_XDEBUG_STR(opt) (opt == '-' ? args->value[26] : args->value[(opt) - 'a'])

int xdebug_dbgp_init(xdebug_con *context, int mode);
int xdebug_dbgp_deinit(xdebug_con *context);
int xdebug_dbgp_error(xdebug_con *context, int type, char *exception_type, char *message, const char *location, const unsigned int line, xdebug_llist *stack);
int xdebug_dbgp_break_on_line(xdebug_con *context, xdebug_brk_info *brk, const char *file, int file_len, int lineno);
int xdebug_dbgp_breakpoint(xdebug_con *context, xdebug_llist *stack, char *file, long lineno, int type, char *exception, char *code, char *message);
int xdebug_dbgp_resolve_breakpoints(xdebug_con *context, zend_string *filename);
int xdebug_dbgp_stream_output(const char *string, unsigned int length);
int xdebug_dbgp_notification(xdebug_con *context, const char *file, long lineno, int type, char *type_string, char *message);
void XDEBUG_ATTRIBUTE_FORMAT(printf, 2, 3) xdebug_dbgp_log(int log_level, const char *fmt, ...);
int xdebug_dbgp_register_eval_id(xdebug_con *context, function_stack_entry *fse);

extern xdebug_remote_handler xdebug_handler_dbgp;

#endif
