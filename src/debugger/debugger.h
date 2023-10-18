/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2023 Derick Rethans                               |
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

#ifndef __XDEBUG_DEBUGGER_H__
#define __XDEBUG_DEBUGGER_H__

#include "com.h"

typedef struct _xdebug_debugger_globals_t {
	int           status;
	int           reason;
	const char   *lastcmd;
	char         *lasttransid;
	zval         *current_return_value;

	zend_bool     remote_connection_enabled;
	zend_ulong    remote_connection_pid;
	zend_bool     breakpoints_allowed;
	zend_bool     suppress_return_value_step;
	zend_bool     detached;
	xdebug_con    context;
	unsigned int  breakpoint_count;
	unsigned int  no_exec;
	char         *ide_key; /* As Xdebug uses it, from environment, USER, USERNAME or empty */

	/* breakpoint resolving */
	size_t        function_count;
	size_t        class_count;
	xdebug_hash  *breakable_lines_map;

	/* output redirection */
	int           stdout_mode;
} xdebug_debugger_globals_t;

typedef struct _xdebug_debugger_settings_t {
	/* Cloud */
	char         *cloud_id;
	char         *cloud_shared_key;

	/* Step Debugger */
	zend_long     client_port;    /* 9003 */
	char         *client_host;    /* localhost */
	zend_bool     discover_client_host; /* (try to) connect back to the HTTP requestor */
	char         *client_discovery_header; /* User configured header to check for forwarded IP address */
	zend_long     connect_timeout_ms; /* Timeout in MS for remote connections */

	char         *ide_key_setting; /* Set through php.ini and friends */
} xdebug_debugger_settings_t;

PHP_INI_MH(OnUpdateDebugMode);

void xdebug_init_debugger_globals(xdebug_debugger_globals_t *xg);

#define XDEBUG_RETURN_VALUE_VAR_NAME "__RETURN_VALUE"

void xdebug_debugger_reset_ide_key(char *envval);
int xdebug_debugger_bailout_if_no_exec_requested(void);
void xdebug_debugger_set_program_name(zend_string *filename);
void xdebug_debugger_register_eval(function_stack_entry *fse);
void xdebug_debugger_restart_if_pid_changed(void);

xdebug_set *xdebug_debugger_get_breakable_lines_from_oparray(zend_op_array *opa);
int xdebug_do_eval(char *eval_string, zval *ret_zval, zend_string **return_message);
bool xdebug_debugger_check_evaled_code(zend_string *filename_in, zend_string **filename_out);
void xdebug_debugger_set_has_line_breakpoints(function_stack_entry *fse);

void xdebug_debugger_statement_call(zend_string *filename, int lineno);
void xdebug_debugger_throw_exception_hook(zend_object *exception, zval *file, zval *line, zval *code, char *code_str, zval *message);
void xdebug_debugger_error_cb(zend_string *error_filename, int error_lineno, int type, char *error_type_str, char *buffer);
void xdebug_debugger_handle_breakpoints(function_stack_entry *fse, int breakpoint_type, zval *return_value);

void xdebug_debugger_zend_startup(void);
void xdebug_debugger_zend_shutdown(void);
void xdebug_debugger_minit(void);
void xdebug_debugger_minfo(void);
void xdebug_debugger_rinit(void);
void xdebug_debugger_post_deactivate(void);

void xdebug_debugger_compile_file(zend_op_array *op_array);

PHP_FUNCTION(xdebug_break);


#endif
