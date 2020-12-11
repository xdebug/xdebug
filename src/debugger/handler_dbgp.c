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
   |          Shane Caraveo <shanec@ActiveState.com>                      |
   +----------------------------------------------------------------------+
 */

#include <sys/types.h>

#ifndef PHP_WIN32
#include <unistd.h>
#endif

#include "php.h"
#include "SAPI.h"

#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "main/php_version.h"
#include "main/php_network.h"
#include "ext/standard/base64.h"
#include "TSRM.h"

#include "php_globals.h"
#include "php_xdebug.h"

#include "com.h"
#include "handler_dbgp.h"
#include "debugger_private.h"

#include "coverage/code_coverage.h"
#include "develop/stack.h"
#include "lib/compat.h"
#include "lib/hash.h"
#include "lib/llist.h"
#include "lib/log.h"
#include "lib/mm.h"
#include "lib/var_export_xml.h"
#include "lib/vector.h"
#include "lib/xml.h"

#ifdef PHP_WIN32
#include "win32/time.h"
#include <process.h>
#endif
#include <fcntl.h>

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

xdebug_remote_handler xdebug_handler_dbgp = {
	xdebug_dbgp_init,
	xdebug_dbgp_deinit,
	xdebug_dbgp_error,
	xdebug_dbgp_break_on_line,
	xdebug_dbgp_breakpoint,
	xdebug_dbgp_resolve_breakpoints,
	xdebug_dbgp_stream_output,
	xdebug_dbgp_notification,
	xdebug_dbgp_register_eval_id,
};

static char *create_eval_key_id(int id);
static void line_breakpoint_resolve_helper(xdebug_con *context, xdebug_lines_list *lines_list, xdebug_brk_info *brk_info);

/*****************************************************************************
** Constants and strings for statii and reasons
*/

/* Status structure */
#define DBGP_STATUS_STARTING  1
#define DBGP_STATUS_STOPPING  2
#define DBGP_STATUS_STOPPED   3
#define DBGP_STATUS_RUNNING   4
#define DBGP_STATUS_BREAK     5
#define DBGP_STATUS_DETACHED  6

const char *xdebug_dbgp_status_strings[7] =
	{"", "starting", "stopping", "stopped", "running", "break", "detached"};

#define DBGP_REASON_OK        0
#define DBGP_REASON_ERROR     1
#define DBGP_REASON_ABORTED   2
#define DBGP_REASON_EXCEPTION 3

const char *xdebug_dbgp_reason_strings[4] =
	{"ok", "error", "aborted", "exception"};

typedef struct {
	int         code;
	const char *message;
} xdebug_error_entry;

xdebug_error_entry xdebug_error_codes[24] = {
	{   0, "no error" },
	{   1, "parse error in command" },
	{   2, "duplicate arguments in command" },
	{   3, "invalid or missing options" },
	{   4, "unimplemented command" },
	{   5, "command is not available" },
	{ 100, "can not open file" },
	{ 101, "stream redirect failed" },
	{ 200, "breakpoint could not be set" },
	{ 201, "breakpoint type is not supported" },
	{ 202, "invalid breakpoint line" },
	{ 203, "no code on breakpoint line" },
	{ 204, "invalid breakpoint state" },
	{ 205, "no such breakpoint" },
	{ 206, "error evaluating code" },
	{ 207, "invalid expression" },
	{ 300, "can not get property" },
	{ 301, "stack depth invalid" },
	{ 302, "context invalid" },
	{ 800, "profiler not started" },
	{ 900, "encoding not supported" },
	{ 998, "an internal exception in the debugger" },
	{ 999, "unknown error" },
	{  -1, NULL }
};

#define XDEBUG_STR_SWITCH_DECL       char *__switch_variable
#define XDEBUG_STR_SWITCH(s)         __switch_variable = (s);
#define XDEBUG_STR_CASE(s)           if (strcmp(__switch_variable, s) == 0) {
#define XDEBUG_STR_CASE_END          } else
#define XDEBUG_STR_CASE_DEFAULT      {
#define XDEBUG_STR_CASE_DEFAULT_END  }

#define XDEBUG_TYPES_COUNT 8
const char *xdebug_dbgp_typemap[XDEBUG_TYPES_COUNT][3] = {
	/* common, lang, schema */
	{"bool",     "bool",     "xsd:boolean"},
	{"int",      "int",      "xsd:decimal"},
	{"float",    "float",    "xsd:double"},
	{"string",   "string",   "xsd:string"},
	{"null",     "null",     NULL},
	{"hash",     "array",    NULL},
	{"object",   "object",   NULL},
	{"resource", "resource", NULL}
};


typedef struct {
	int         value;
	const char *name;
} xdebug_breakpoint_entry;

#define XDEBUG_BREAKPOINT_TYPES_COUNT 6
xdebug_breakpoint_entry xdebug_breakpoint_types[XDEBUG_BREAKPOINT_TYPES_COUNT] = {
	{ XDEBUG_BREAKPOINT_TYPE_LINE,        "line" },
	{ XDEBUG_BREAKPOINT_TYPE_CONDITIONAL, "conditional" },
	{ XDEBUG_BREAKPOINT_TYPE_CALL,        "call" },
	{ XDEBUG_BREAKPOINT_TYPE_RETURN,      "return" },
	{ XDEBUG_BREAKPOINT_TYPE_EXCEPTION,   "exception" },
	{ XDEBUG_BREAKPOINT_TYPE_WATCH,       "watch" }
};

#define XDEBUG_DBGP_SCAN_RANGE 5

/*****************************************************************************
** Prototypes for debug command handlers
*/

/* DBGP_FUNC(break); */
DBGP_FUNC(breakpoint_get);
DBGP_FUNC(breakpoint_list);
DBGP_FUNC(breakpoint_remove);
DBGP_FUNC(breakpoint_set);
DBGP_FUNC(breakpoint_update);

DBGP_FUNC(context_get);
DBGP_FUNC(context_names);

DBGP_FUNC(eval);
DBGP_FUNC(feature_get);
DBGP_FUNC(feature_set);

DBGP_FUNC(typemap_get);
DBGP_FUNC(property_get);
DBGP_FUNC(property_set);
DBGP_FUNC(property_value);

DBGP_FUNC(source);
DBGP_FUNC(stack_depth);
DBGP_FUNC(stack_get);
DBGP_FUNC(status);

DBGP_FUNC(stderr);
DBGP_FUNC(stdout);

DBGP_FUNC(stop);
DBGP_FUNC(run);
DBGP_FUNC(step_into);
DBGP_FUNC(step_out);
DBGP_FUNC(step_over);
DBGP_FUNC(detach);

/* Non standard comments */
DBGP_FUNC(xcmd_profiler_name_get);
DBGP_FUNC(xcmd_get_executable_lines);

/*****************************************************************************
** Dispatcher tables for supported debug commands
*/

static xdebug_dbgp_cmd dbgp_commands[] = {
	/* DBGP_FUNC_ENTRY(break) */
	DBGP_FUNC_ENTRY(breakpoint_get,    XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(breakpoint_list,   XDEBUG_DBGP_POST_MORTEM)
	DBGP_FUNC_ENTRY(breakpoint_remove, XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(breakpoint_set,    XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(breakpoint_update, XDEBUG_DBGP_NONE)

	DBGP_FUNC_ENTRY(context_get,       XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(context_names,     XDEBUG_DBGP_POST_MORTEM)

	DBGP_FUNC_ENTRY(eval,              XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(feature_get,       XDEBUG_DBGP_POST_MORTEM)
	DBGP_FUNC_ENTRY(feature_set,       XDEBUG_DBGP_NONE)

	DBGP_FUNC_ENTRY(typemap_get,       XDEBUG_DBGP_POST_MORTEM)
	DBGP_FUNC_ENTRY(property_get,      XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(property_set,      XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(property_value,    XDEBUG_DBGP_NONE)

	DBGP_FUNC_ENTRY(source,            XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(stack_depth,       XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(stack_get,         XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(status,            XDEBUG_DBGP_POST_MORTEM)

	DBGP_FUNC_ENTRY(stderr,            XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(stdout,            XDEBUG_DBGP_NONE)

	DBGP_CONT_FUNC_ENTRY(run,          XDEBUG_DBGP_NONE)
	DBGP_CONT_FUNC_ENTRY(step_into,    XDEBUG_DBGP_NONE)
	DBGP_CONT_FUNC_ENTRY(step_out,     XDEBUG_DBGP_NONE)
	DBGP_CONT_FUNC_ENTRY(step_over,    XDEBUG_DBGP_NONE)

	DBGP_STOP_FUNC_ENTRY(stop,         XDEBUG_DBGP_POST_MORTEM)
	DBGP_STOP_FUNC_ENTRY(detach,       XDEBUG_DBGP_POST_MORTEM)

	/* Non standard functions */
	DBGP_FUNC_ENTRY(xcmd_profiler_name_get,    XDEBUG_DBGP_POST_MORTEM)
	DBGP_FUNC_ENTRY(xcmd_get_executable_lines, XDEBUG_DBGP_NONE)
	{ NULL, NULL, 0 }
};

/*****************************************************************************
** Utility functions
*/

static xdebug_dbgp_cmd* lookup_cmd(char *cmd)
{
	xdebug_dbgp_cmd *ptr = dbgp_commands;

	while (ptr->name) {
		if (strcmp(ptr->name, cmd) == 0) {
			return ptr;
		}
		ptr++;
	}
	return NULL;
}

static xdebug_str *make_message(xdebug_con *context, xdebug_xml_node *message)
{
	xdebug_str  xml_message = XDEBUG_STR_INITIALIZER;
	xdebug_str *ret = xdebug_str_new();

	xdebug_xml_return_node(message, &xml_message);
	xdebug_log(XLOG_CHAN_DEBUG, XLOG_COM, "-> %s\n", xml_message.d);

	xdebug_str_add_fmt(ret, "%d", xml_message.l + sizeof("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n") - 1);
	xdebug_str_addc(ret, '\0');
	xdebug_str_add_literal(ret, "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n");
	xdebug_str_add(ret, xml_message.d, 0);
	xdebug_str_addc(ret, '\0');
	xdebug_str_destroy(&xml_message);

	return ret;
}

static void send_message_ex(xdebug_con *context, xdebug_xml_node *message, int stage)
{
	xdebug_str *tmp;

	/* Sometimes we end up in 'send_message' although the debugging connection
	 * is already closed. In that case, we early return. */
	if (XG_DBG(status) != DBGP_STATUS_STARTING && !xdebug_is_debug_connection_active()) {
		return;
	}

	tmp = make_message(context, message);
	if ((size_t) SSENDL(context->socket, tmp->d, tmp->l) != tmp->l) {
		char *sock_error = php_socket_strerror(php_socket_errno(), NULL, 0);
		char *utime_str = xdebug_nanotime_to_chars(xdebug_get_nanotime(), 6);

		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SENDERR", "%s: There was a problem sending %zd bytes on socket %d: %s.", utime_str, tmp->l, context->socket, sock_error);

		efree(sock_error);
		xdfree(utime_str);
	}
	xdebug_str_free(tmp);
}

static void send_message(xdebug_con *context, xdebug_xml_node *message)
{
	send_message_ex(context, message, 0);
}


static xdebug_xml_node* get_symbol(xdebug_str *name, xdebug_var_export_options *options)
{
	zval                   retval;
	xdebug_xml_node       *tmp_node;

	xdebug_get_php_symbol(&retval, name);

	if (Z_TYPE(retval) == IS_UNDEF) {
		return NULL;
	}

	if (strcmp(name->d, "this") == 0 && Z_TYPE(retval) == IS_NULL) {
		return NULL;
	}

	tmp_node = xdebug_get_zval_value_xml_node(name, &retval, options);
	zval_ptr_dtor_nogc(&retval);

	return tmp_node;
}

static int get_symbol_contents(xdebug_str *name, xdebug_xml_node *node, xdebug_var_export_options *options)
{
	zval retval;
	zval *retval_ptr;

	xdebug_get_php_symbol(&retval, name);

	if (Z_TYPE(retval) == IS_UNDEF) {
		return 0;
	}

	// TODO WTF???
	retval_ptr = &retval;

	xdebug_var_export_xml_node(&retval_ptr, name, node, options, 1);
	zval_ptr_dtor_nogc(&retval);

	return 1;
}

static xdebug_str* return_file_source(zend_string *filename, int begin, int end)
{
	php_stream *stream;
	int    i = begin;
	char  *line = NULL;
	xdebug_str *source = xdebug_str_new();
	char *tmp_filename = NULL;

	if (i < 0) {
		begin = 0;
		i = 0;
	}
	xdebug_str_add_literal(source, "");

	tmp_filename = xdebug_path_from_url(filename);
	stream = php_stream_open_wrapper(tmp_filename, "rb",
			USE_PATH | REPORT_ERRORS,
			NULL);
	xdfree(tmp_filename);

	/* Read until the "begin" line has been read */
	if (!stream) {
		return NULL;
	}

	/* skip to the first requested line */
	while (i > 0 && !php_stream_eof(stream)) {
		if (line) {
			efree(line);
			line = NULL;
		}
		line = php_stream_gets(stream, NULL, 1024);
		i--;
	}
	/* Read until the "end" line has been read */
	do {
		if (line) {
			xdebug_str_add(source, line, 0);
			efree(line);
			line = NULL;
			if (php_stream_eof(stream)) break;
		}
		line = php_stream_gets(stream, NULL, 1024);
		i++;
	} while (i < end + 1 - begin);

	/* Print last line */
	if (line) {
		efree(line);
		line = NULL;
	}
	php_stream_close(stream);
	return source;
}

static xdebug_str* return_eval_source(char *id, int begin, int end)
{
	char             *key;
	xdebug_str       *joined;
	xdebug_eval_info *ei;
	xdebug_arg       *parts;

	if (begin < 0) {
		begin = 0;
	}

	key = create_eval_key_id(atoi(id));

	if (!xdebug_hash_find(XG_DBG(context).eval_id_lookup, key, strlen(key), (void *) &ei)) {
		return NULL;
	}

	parts = xdebug_arg_ctor();
	xdebug_explode("\n", ZSTR_VAL(ei->contents), parts, end + 2);
	joined = xdebug_join("\n", parts, begin, end);
	xdebug_arg_dtor(parts);

	return joined;
}

static int is_dbgp_url(zend_string *filename)
{
	return (strncmp(ZSTR_VAL(filename), "dbgp://", 7) == 0);
}

static xdebug_str* return_source(zend_string *filename, int begin, int end)
{
	if (is_dbgp_url(filename)) {
		return return_eval_source(ZSTR_VAL(filename) + 7, begin, end);
	} else {
		return return_file_source(filename, begin, end);
	}
}


static int check_evaled_code(zend_string *filename_in, char **filename_out)
{
	char *end_marker;
	xdebug_eval_info *ei;

	end_marker = ZSTR_VAL(filename_in) + ZSTR_LEN(filename_in) - strlen("eval()'d code");
	if (end_marker >= ZSTR_VAL(filename_in) && strcmp("eval()'d code", end_marker) == 0) {
		if (xdebug_hash_find(XG_DBG(context).eval_id_lookup, ZSTR_VAL(filename_in), ZSTR_LEN(filename_in), (void *) &ei)) {
			*filename_out = xdebug_sprintf("dbgp://%lu", ei->id);
		}
		return 1;
	}
	return 0;
}

static xdebug_xml_node* return_stackframe(int nr)
{
	function_stack_entry *fse, *fse_prev;
	char                 *tmp_fname;
	char                 *tmp_filename;
	xdebug_xml_node      *tmp;

	fse = xdebug_get_stack_frame(nr);
	fse_prev = xdebug_get_stack_frame(nr - 1);

	tmp_fname = xdebug_show_fname(fse->function, 0, 0);

	tmp = xdebug_xml_node_init("stack");
	xdebug_xml_add_attribute_ex(tmp, "where", xdstrdup(tmp_fname), 0, 1);
	xdebug_xml_add_attribute_ex(tmp, "level", xdebug_sprintf("%ld", nr), 0, 1);
	if (fse_prev) {
		if (check_evaled_code(fse_prev->filename, &tmp_filename)) {
			xdebug_xml_add_attribute_ex(tmp, "type",     xdstrdup("eval"), 0, 1);
			xdebug_xml_add_attribute_ex(tmp, "filename", tmp_filename, 0, 0);
		} else {
			xdebug_xml_add_attribute_ex(tmp, "type",     xdstrdup("file"), 0, 1);
			xdebug_xml_add_attribute_ex(tmp, "filename", xdebug_path_to_url(fse_prev->filename), 0, 1);
		}
		xdebug_xml_add_attribute_ex(tmp, "lineno",   xdebug_sprintf("%lu", fse_prev->lineno), 0, 1);
	} else {
		zend_string *executed_filename = zend_get_executed_filename_ex();
		int          executed_lineno   = zend_get_executed_lineno();
		char        *tmp_filename;

		if (check_evaled_code(executed_filename, &tmp_filename)) {
			xdebug_xml_add_attribute_ex(tmp, "type", xdstrdup("eval"), 0, 1);
			xdebug_xml_add_attribute_ex(tmp, "filename", tmp_filename, 0, 0);
		} else {
			xdebug_xml_add_attribute_ex(tmp, "type", xdstrdup("file"), 0, 1);
			xdebug_xml_add_attribute_ex(tmp, "filename", xdebug_path_to_url(executed_filename), 0, 1);
		}
		xdebug_xml_add_attribute_ex(tmp, "lineno", xdebug_sprintf("%lu", executed_lineno), 0, 1);
	}

	xdfree(tmp_fname);
	return tmp;
}

/*****************************************************************************
** Client command handlers - Breakpoints
*/

/* Helper functions */
static void xdebug_hash_admin_dtor(xdebug_brk_admin *admin)
{
	xdfree(admin->key);
	xdfree(admin);
}

static int breakpoint_admin_add(xdebug_con *context, int type, char *key)
{
	xdebug_brk_admin *admin = xdmalloc(sizeof(xdebug_brk_admin));
	char             *hkey;

	XG_DBG(breakpoint_count)++;
	admin->id   = ((xdebug_get_pid() & 0x1ffff) * 10000) + XG_DBG(breakpoint_count);
	admin->type = type;
	admin->key  = xdstrdup(key);

	hkey = xdebug_sprintf("%lu", admin->id);
	xdebug_hash_add(context->breakpoint_list, hkey, strlen(hkey), (void*) admin);
	xdfree(hkey);

	return admin->id;
}

static int breakpoint_admin_fetch(xdebug_con *context, char *hkey, int *type, char **key)
{
	xdebug_brk_admin *admin;

	if (!xdebug_hash_find(context->breakpoint_list, hkey, strlen(hkey), (void *) &admin)) {
		return FAILURE;
	}

	*type = admin->type;
	*key  = admin->key;

	return SUCCESS;
}

static int breakpoint_admin_remove(xdebug_con *context, char *hkey)
{
	if (!xdebug_hash_delete(context->breakpoint_list, hkey, strlen(hkey))) {
		return FAILURE;
	}

	return SUCCESS;
}

static void breakpoint_brk_info_add_resolved(xdebug_xml_node *xml, xdebug_brk_info *brk_info)
{
	if (!XG_DBG(context).resolved_breakpoints) {
		return;
	}

	if (brk_info->resolved == XDEBUG_BRK_RESOLVED) {
		xdebug_xml_add_attribute(xml, "resolved", "resolved");
	} else {
		xdebug_xml_add_attribute(xml, "resolved", "unresolved");
	}
}

static void breakpoint_brk_info_add(xdebug_xml_node *xml, xdebug_brk_info *brk_info)
{
	xdebug_xml_add_attribute_ex(xml, "type", xdstrdup(XDEBUG_BREAKPOINT_TYPE_NAME(brk_info->brk_type)), 0, 1);
	breakpoint_brk_info_add_resolved(xml, brk_info);
	if (brk_info->filename) {
		if (is_dbgp_url(brk_info->filename)) {
			xdebug_xml_add_attribute_ex(xml, "filename", ZSTR_VAL(brk_info->filename), 0, 0);
		} else {
			xdebug_xml_add_attribute_ex(xml, "filename", xdebug_path_to_url(brk_info->filename), 0, 1);
		}
	}
	if (brk_info->resolved_lineno) {
		xdebug_xml_add_attribute_ex(xml, "lineno", xdebug_sprintf("%lu", brk_info->resolved_lineno), 0, 1);
	}
	if (brk_info->functionname) {
		xdebug_xml_add_attribute_ex(xml, "function", xdstrdup(brk_info->functionname), 0, 1);
	}
	if (brk_info->classname) {
		xdebug_xml_add_attribute_ex(xml, "class", xdstrdup(brk_info->classname), 0, 1);
	}
	if (brk_info->exceptionname) {
		xdebug_xml_add_attribute_ex(xml, "exception", xdstrdup(brk_info->exceptionname), 0, 1);
	}
	if (brk_info->temporary) {
		xdebug_xml_add_attribute(xml, "state", "temporary");
	} else if (brk_info->disabled) {
		xdebug_xml_add_attribute(xml, "state", "disabled");
	} else {
		xdebug_xml_add_attribute(xml, "state", "enabled");
	}
	xdebug_xml_add_attribute_ex(xml, "hit_count", xdebug_sprintf("%lu", brk_info->hit_count), 0, 1);
	switch (brk_info->hit_condition) {
		case XDEBUG_HIT_GREATER_EQUAL:
			xdebug_xml_add_attribute(xml, "hit_condition", ">=");
			break;
		case XDEBUG_HIT_EQUAL:
			xdebug_xml_add_attribute(xml, "hit_condition", "==");
			break;
		case XDEBUG_HIT_MOD:
			xdebug_xml_add_attribute(xml, "hit_condition", "%");
			break;
	}
	if (brk_info->condition) {
		xdebug_xml_node *condition = xdebug_xml_node_init("expression");
		xdebug_xml_add_text_ex(condition, brk_info->condition, strlen(brk_info->condition), 0, 1);
		xdebug_xml_add_child(xml, condition);
	}
	xdebug_xml_add_attribute_ex(xml, "hit_value", xdebug_sprintf("%lu", brk_info->hit_value), 0, 1);
	xdebug_xml_add_attribute_ex(xml, "id", xdebug_sprintf("%lu", brk_info->id), 0, 1);
}

static xdebug_brk_info* breakpoint_brk_info_fetch(int type, char *hkey)
{
	xdebug_llist_element *le;
	xdebug_brk_info      *brk_info = NULL;

	switch (type) {
		case XDEBUG_BREAKPOINT_TYPE_LINE:
		case XDEBUG_BREAKPOINT_TYPE_CONDITIONAL: {
			xdebug_arg *parts;

			/* First we split the key into filename and linenumber */
			parts = xdebug_arg_ctor();
			xdebug_explode("$", hkey, parts, -1);

			/* Second we loop through the list of file/line breakpoints to
			 * look for our thingy */
			for (le = XDEBUG_LLIST_HEAD(XG_DBG(context).line_breakpoints); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
				brk_info = XDEBUG_LLIST_VALP(le);

				if (atoi(parts->args[1]) == brk_info->original_lineno && memcmp(ZSTR_VAL(brk_info->filename), parts->args[0], ZSTR_LEN(brk_info->filename)) == 0) {
					xdebug_arg_dtor(parts);
					return brk_info;
				}
			}

			/* Cleaning up */
			xdebug_arg_dtor(parts);

			break;
		}

		case XDEBUG_BREAKPOINT_TYPE_CALL:
		case XDEBUG_BREAKPOINT_TYPE_RETURN:
			if (xdebug_hash_find(XG_DBG(context).function_breakpoints, hkey, strlen(hkey), (void *) &brk_info)) {
				return brk_info;
			}
			break;

		case XDEBUG_BREAKPOINT_TYPE_EXCEPTION:
			if (xdebug_hash_find(XG_DBG(context).exception_breakpoints, hkey, strlen(hkey), (void *) &brk_info)) {
				return brk_info;
			}
			break;
	}
	return brk_info;
}

static int breakpoint_remove(int type, char *hkey)
{
	xdebug_llist_element *le;
	xdebug_brk_info      *brk_info = NULL;
	int                   retval = FAILURE;

	switch (type) {
		case XDEBUG_BREAKPOINT_TYPE_LINE:
		case XDEBUG_BREAKPOINT_TYPE_CONDITIONAL: {
			xdebug_arg *parts;

			/* First we split the key into filename and linenumber */
			parts = xdebug_arg_ctor();
			xdebug_explode("$", hkey, parts, -1);

			/* Second we loop through the list of file/line breakpoints to
			 * look for our thingy */
			for (le = XDEBUG_LLIST_HEAD(XG_DBG(context).line_breakpoints); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
				brk_info = XDEBUG_LLIST_VALP(le);

				if (atoi(parts->args[1]) == brk_info->original_lineno && memcmp(ZSTR_VAL(brk_info->filename), parts->args[0], ZSTR_LEN(brk_info->filename)) == 0) {
					xdebug_llist_remove(XG_DBG(context).line_breakpoints, le, NULL);
					retval = SUCCESS;
					break;
				}
			}

			/* Cleaning up */
			xdebug_arg_dtor(parts);
			break;
		}

		case XDEBUG_BREAKPOINT_TYPE_CALL:
		case XDEBUG_BREAKPOINT_TYPE_RETURN:
			if (xdebug_hash_delete(XG_DBG(context).function_breakpoints, hkey, strlen(hkey))) {
				retval = SUCCESS;
			}
			break;

		case XDEBUG_BREAKPOINT_TYPE_EXCEPTION:
			if (xdebug_hash_delete(XG_DBG(context).exception_breakpoints, hkey, strlen(hkey))) {
				retval = SUCCESS;
			}
			break;
	}
	return retval;
}

#define BREAKPOINT_ACTION_GET       1
#define BREAKPOINT_ACTION_REMOVE    2
#define BREAKPOINT_ACTION_UPDATE    3

#define BREAKPOINT_CHANGE_STATE() \
	XDEBUG_STR_SWITCH(CMD_OPTION_CHAR('s')) { \
		XDEBUG_STR_CASE("enabled") \
			brk_info->disabled = 0; \
		XDEBUG_STR_CASE_END \
 \
		XDEBUG_STR_CASE("disabled") \
			brk_info->disabled = 1; \
		XDEBUG_STR_CASE_END \
 \
		XDEBUG_STR_CASE_DEFAULT \
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS); \
		XDEBUG_STR_CASE_DEFAULT_END \
	}

#define BREAKPOINT_CHANGE_OPERATOR() \
	XDEBUG_STR_SWITCH(CMD_OPTION_CHAR('o')) { \
		XDEBUG_STR_CASE(">=") \
			brk_info->hit_condition = XDEBUG_HIT_GREATER_EQUAL; \
		XDEBUG_STR_CASE_END \
 \
		XDEBUG_STR_CASE("==") \
			brk_info->hit_condition = XDEBUG_HIT_EQUAL; \
		XDEBUG_STR_CASE_END \
 \
		XDEBUG_STR_CASE("%") \
			brk_info->hit_condition = XDEBUG_HIT_MOD; \
		XDEBUG_STR_CASE_END \
 \
		XDEBUG_STR_CASE_DEFAULT \
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS); \
		XDEBUG_STR_CASE_DEFAULT_END \
	}



static void breakpoint_do_action(DBGP_FUNC_PARAMETERS, int action)
{
	int                   type;
	char                 *hkey;
	xdebug_brk_info      *brk_info;
	xdebug_xml_node      *breakpoint_node;
	XDEBUG_STR_SWITCH_DECL;

	if (!CMD_OPTION_SET('d')) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	/* Lets check if it exists */
	if (breakpoint_admin_fetch(context, CMD_OPTION_CHAR('d'), &type, (char**) &hkey) == FAILURE) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_NO_SUCH_BREAKPOINT)
	}

	/* so it exists, now we're going to find it in the correct hash/list
	 * and return the info we have on it */
	brk_info = breakpoint_brk_info_fetch(type, hkey);

	if (action == BREAKPOINT_ACTION_UPDATE) {
		if (CMD_OPTION_SET('s')) {
			BREAKPOINT_CHANGE_STATE();
		}
		if (CMD_OPTION_SET('n')) {
			brk_info->original_lineno = strtol(CMD_OPTION_CHAR('n'), NULL, 10);
			brk_info->resolved_lineno = brk_info->original_lineno;
		}
		if (CMD_OPTION_SET('h')) {
			brk_info->hit_value = strtol(CMD_OPTION_CHAR('h'), NULL, 10);
		}
		if (CMD_OPTION_SET('o')) {
			BREAKPOINT_CHANGE_OPERATOR();
		}
	}

	breakpoint_node = xdebug_xml_node_init("breakpoint");
	breakpoint_brk_info_add(breakpoint_node, brk_info);
	xdebug_xml_add_child(*retval, breakpoint_node);

	if (action == BREAKPOINT_ACTION_REMOVE) {
		/* Now we remove the crap */
		breakpoint_remove(type, hkey);
		breakpoint_admin_remove(context, CMD_OPTION_CHAR('d'));
	}
}

DBGP_FUNC(breakpoint_get)
{
	breakpoint_do_action(DBGP_FUNC_PASS_PARAMETERS, BREAKPOINT_ACTION_GET);
}

DBGP_FUNC(breakpoint_remove)
{
	breakpoint_do_action(DBGP_FUNC_PASS_PARAMETERS, BREAKPOINT_ACTION_REMOVE);
}

DBGP_FUNC(breakpoint_update)
{
	breakpoint_do_action(DBGP_FUNC_PASS_PARAMETERS, BREAKPOINT_ACTION_UPDATE);
}


static void breakpoint_list_helper(void *xml, xdebug_hash_element *he)
{
	xdebug_xml_node  *xml_node = (xdebug_xml_node*) xml;
	xdebug_xml_node  *child;
	xdebug_brk_admin *admin = (xdebug_brk_admin*) he->ptr;
	xdebug_brk_info  *brk_info;

	child = xdebug_xml_node_init("breakpoint");
	brk_info = breakpoint_brk_info_fetch(admin->type, admin->key);
	breakpoint_brk_info_add(child, brk_info);
	xdebug_xml_add_child(xml_node, child);
}

DBGP_FUNC(breakpoint_list)
{
	xdebug_hash_apply(context->breakpoint_list, (void *) *retval, breakpoint_list_helper);
}

DBGP_FUNC(breakpoint_set)
{
	xdebug_brk_info      *brk_info;
	char                 *tmp_name;
	size_t                new_length = 0;
	XDEBUG_STR_SWITCH_DECL;

	brk_info = xdebug_brk_info_ctor();

	if (!CMD_OPTION_SET('t')) {
		xdebug_brk_info_dtor(brk_info);
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	} else {
		int i;
		int found = 0;

		for (i = 0; i < XDEBUG_BREAKPOINT_TYPES_COUNT; i++) {
			if (strcmp(xdebug_breakpoint_types[i].name, CMD_OPTION_CHAR('t')) == 0) {
				brk_info->brk_type = xdebug_breakpoint_types[i].value;
				found = 1;
				break;
			}
		}

		if (!found) {
			xdebug_brk_info_dtor(brk_info);
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
		}
	}

	if (CMD_OPTION_SET('s')) {
		BREAKPOINT_CHANGE_STATE();
		xdebug_xml_add_attribute_ex(*retval, "state", xdstrdup(CMD_OPTION_CHAR('s')), 0, 1);
	}
	if (CMD_OPTION_SET('o') && CMD_OPTION_SET('h')) {
		BREAKPOINT_CHANGE_OPERATOR();
		brk_info->hit_value = strtol(CMD_OPTION_CHAR('h'), NULL, 10);
	}
	if (CMD_OPTION_SET('r')) {
		brk_info->temporary = strtol(CMD_OPTION_CHAR('r'), NULL, 10);
	}

	if ((strcmp(CMD_OPTION_CHAR('t'), "line") == 0) || (strcmp(CMD_OPTION_CHAR('t'), "conditional") == 0)) {
		if (!CMD_OPTION_SET('n')) {
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
		}
		brk_info->original_lineno = strtol(CMD_OPTION_CHAR('n'), NULL, 10);
		brk_info->resolved_lineno = brk_info->original_lineno;

		/* If no filename is given, we use the current one */
		if (!CMD_OPTION_SET('f')) {
			function_stack_entry *fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));

			if (!fse) {
				RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
			} else {
				char *tmp_path = xdebug_path_from_url(fse->filename);
				brk_info->filename = zend_string_init(tmp_path, strlen(tmp_path), 0);
			}
		} else {
			char         realpath_file[MAXPATHLEN];
			zend_string *tmp_f = zend_string_init(CMD_OPTION_CHAR('f'), CMD_OPTION_LEN('f'), 0);
			char        *tmp_path = xdebug_path_from_url(tmp_f);

			brk_info->filename = zend_string_init(tmp_path, strlen(tmp_path), 0);

			/* Now we do some real path checks to resolve symlinks. */
			if (VCWD_REALPATH(ZSTR_VAL(brk_info->filename), realpath_file)) {
				zend_string_release(brk_info->filename);
				brk_info->filename = zend_string_init(realpath_file, strlen(realpath_file), 0);
			}

			zend_string_release(tmp_f);
			xdfree(tmp_path);
		}

		/* Perhaps we have a break condition */
		if (CMD_OPTION_SET('-')) {
			brk_info->condition = (char*) xdebug_base64_decode((unsigned char*) CMD_OPTION_CHAR('-'), CMD_OPTION_LEN('-'), &new_length);
		}

		tmp_name = xdebug_sprintf("%s$%lu", ZSTR_VAL(brk_info->filename), brk_info->original_lineno);
		if (strcmp(CMD_OPTION_CHAR('t'), "line") == 0) {
			brk_info->id = breakpoint_admin_add(context, XDEBUG_BREAKPOINT_TYPE_LINE, tmp_name);
		} else {
			brk_info->id = breakpoint_admin_add(context, XDEBUG_BREAKPOINT_TYPE_CONDITIONAL, tmp_name);
		}
		xdfree(tmp_name);
		xdebug_llist_insert_next(context->line_breakpoints, XDEBUG_LLIST_TAIL(context->line_breakpoints), (void*) brk_info);

		if (XG_DBG(context).resolved_breakpoints) {
			xdebug_lines_list *lines_list;

			if (xdebug_hash_find(XG_DBG(breakable_lines_map), ZSTR_VAL(brk_info->filename), ZSTR_LEN(brk_info->filename), (void *) &lines_list)) {
				line_breakpoint_resolve_helper(context, lines_list, brk_info);
			}
		}
	} else

	if ((strcmp(CMD_OPTION_CHAR('t'), "call") == 0) || (strcmp(CMD_OPTION_CHAR('t'), "return") == 0)) {
		if (strcmp(CMD_OPTION_CHAR('t'), "call") == 0) {
			brk_info->function_break_type = XDEBUG_BREAKPOINT_TYPE_CALL;
		} else {
			brk_info->function_break_type = XDEBUG_BREAKPOINT_TYPE_RETURN;
		}

		if (!CMD_OPTION_SET('m')) {
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
		}
		brk_info->functionname = xdstrdup(CMD_OPTION_CHAR('m'));
		if (CMD_OPTION_SET('a')) {
			int   res;

			brk_info->classname = xdstrdup(CMD_OPTION_CHAR('a'));
			tmp_name = xdebug_sprintf("%s::%s", CMD_OPTION_CHAR('a'), CMD_OPTION_CHAR('m'));
			res = xdebug_hash_add(context->function_breakpoints, tmp_name, strlen(tmp_name), (void*) brk_info);
			if (brk_info->function_break_type == XDEBUG_BREAKPOINT_TYPE_CALL) {
				brk_info->id = breakpoint_admin_add(context, XDEBUG_BREAKPOINT_TYPE_CALL, tmp_name);
			} else {
				brk_info->id = breakpoint_admin_add(context, XDEBUG_BREAKPOINT_TYPE_RETURN, tmp_name);
			}
			xdfree(tmp_name);

			if (!res) {
				RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
			}
		} else {
			if (!xdebug_hash_add(context->function_breakpoints, CMD_OPTION_CHAR('m'), CMD_OPTION_LEN('m'), (void*) brk_info)) {
				RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_BREAKPOINT_NOT_SET);
			} else {
				if (brk_info->function_break_type == XDEBUG_BREAKPOINT_TYPE_CALL) {
					brk_info->id = breakpoint_admin_add(context, XDEBUG_BREAKPOINT_TYPE_CALL, CMD_OPTION_CHAR('m'));
				} else {
					brk_info->id = breakpoint_admin_add(context, XDEBUG_BREAKPOINT_TYPE_RETURN, CMD_OPTION_CHAR('m'));
				}
			}
		}

		brk_info->resolved = XDEBUG_BRK_RESOLVED;
	} else

	if (strcmp(CMD_OPTION_CHAR('t'), "exception") == 0) {
		if (!CMD_OPTION_SET('x')) {
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
		}
		brk_info->exceptionname = xdstrdup(CMD_OPTION_CHAR('x'));
		if (!xdebug_hash_add(context->exception_breakpoints, CMD_OPTION_CHAR('x'), CMD_OPTION_LEN('x'), (void*) brk_info)) {
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_BREAKPOINT_NOT_SET);
		} else {
			brk_info->id = breakpoint_admin_add(context, XDEBUG_BREAKPOINT_TYPE_EXCEPTION, CMD_OPTION_CHAR('x'));
		}

		brk_info->resolved = XDEBUG_BRK_RESOLVED;
	} else

	if (strcmp(CMD_OPTION_CHAR('t'), "watch") == 0) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_BREAKPOINT_TYPE_NOT_SUPPORTED);
	}

	xdebug_xml_add_attribute_ex(*retval, "id", xdebug_sprintf("%lu", brk_info->id), 0, 1);
	breakpoint_brk_info_add_resolved(*retval, brk_info);
}

DBGP_FUNC(eval)
{
	char            *eval_string;
	xdebug_xml_node *ret_xml;
	zval             ret_zval;
	size_t           new_length = 0;
	int              res;
	xdebug_var_export_options *options;

	if (!CMD_OPTION_SET('-')) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	options = (xdebug_var_export_options*) context->options;

	if (CMD_OPTION_SET('p')) {
		options->runtime[0].page = strtol(CMD_OPTION_CHAR('p'), NULL, 10);
	} else {
		options->runtime[0].page = 0;
	}

	/* base64 decode eval string */
	eval_string = (char*) xdebug_base64_decode((unsigned char*) CMD_OPTION_CHAR('-'), CMD_OPTION_LEN('-'), &new_length);

	res = xdebug_do_eval(eval_string, &ret_zval);

	xdfree(eval_string);

	/* Handle result */
	if (res == FAILURE) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_EVALUATING_CODE);
	} else {
		ret_xml = xdebug_get_zval_value_xml_node(NULL, &ret_zval, options);
		xdebug_xml_add_child(*retval, ret_xml);
		zval_ptr_dtor(&ret_zval);
	}
}

/* these functions interupt PHP's output functions, so we can
   redirect to our remote debugger! */
static void xdebug_send_stream(const char *name, const char *str, unsigned int str_length)
{
	/* create an xml document to send as the stream */
	xdebug_xml_node *message;

	if (!xdebug_is_debug_connection_active()) {
		return;
	}

	message = xdebug_xml_node_init("stream");
	xdebug_xml_add_attribute(message, "xmlns", "urn:debugger_protocol_v1");
	xdebug_xml_add_attribute(message, "xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");
	xdebug_xml_add_attribute_ex(message, "type", (char *)name, 0, 0);
	xdebug_xml_add_text_encodel(message, xdstrndup(str, str_length), str_length);
	send_message(&XG_DBG(context), message);
	xdebug_xml_node_dtor(message);

	return;
}


DBGP_FUNC(stderr)
{
	xdebug_xml_add_attribute(*retval, "success", "0");
}

DBGP_FUNC(stdout)
{
	int mode = 0;
	const char *success = "0";

	if (!CMD_OPTION_SET('c')) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	mode = strtol(CMD_OPTION_CHAR('c'), NULL, 10);
	XG_DBG(stdout_mode) = mode;
	success = "1";

	xdebug_xml_add_attribute_ex(*retval, "success", xdstrdup(success), 0, 1);
}

DBGP_FUNC(stop)
{
	XG_DBG(status) = DBGP_STATUS_STOPPED;
	xdebug_xml_add_attribute(*retval, "status", xdebug_dbgp_status_strings[XG_DBG(status)]);
	xdebug_xml_add_attribute(*retval, "reason", xdebug_dbgp_reason_strings[XG_DBG(reason)]);
}

DBGP_FUNC(run)
{
	xdebug_xml_add_attribute_ex(*retval, "filename", ZSTR_VAL(context->program_name), 0, 0);
}

DBGP_FUNC(step_into)
{
	XG_DBG(context).do_next   = 0;
	XG_DBG(context).do_step   = 1;
	XG_DBG(context).do_finish = 0;
}

DBGP_FUNC(step_out)
{
	function_stack_entry *fse;

	XG_DBG(context).do_next   = 0;
	XG_DBG(context).do_step   = 0;
	XG_DBG(context).do_finish = 1;

	if ((fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack)))) {
		XG_DBG(context).finish_level = fse->level;
		XG_DBG(context).finish_func_nr = fse->function_nr;
	} else {
		XG_DBG(context).finish_level = -1;
		XG_DBG(context).finish_func_nr = -1;
	}
}

DBGP_FUNC(step_over)
{
	function_stack_entry *fse;

	XG_DBG(context).do_next   = 1;
	XG_DBG(context).do_step   = 0;
	XG_DBG(context).do_finish = 0;

	if ((fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack)))) {
		XG_DBG(context).next_level = fse->level;
	} else {
		XG_DBG(context).next_level = 0;
	}
}

DBGP_FUNC(detach)
{
	XG_DBG(status) = DBGP_STATUS_DETACHED;

	xdebug_xml_add_attribute(*retval, "status", xdebug_dbgp_status_strings[DBGP_STATUS_STOPPED]);
	xdebug_xml_add_attribute(*retval, "reason", xdebug_dbgp_reason_strings[XG_DBG(reason)]);
	XG_DBG(context).handler->remote_deinit(&(XG_DBG(context)));
	xdebug_mark_debug_connection_not_active();
	XG_DBG(stdout_mode) = 0;
	XG_DBG(detached) = 1;
}


DBGP_FUNC(source)
{
	xdebug_str *source;
	int   begin = 0, end = 999999;
	zend_string *filename;
	function_stack_entry *fse;

	if (!CMD_OPTION_SET('f')) {
		if ((fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack)))) {
			filename = zend_string_copy(fse->filename);
		} else {
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
		}
	} else {
		filename = zend_string_init(CMD_OPTION_CHAR('f'), CMD_OPTION_LEN('f'), 0);
	}

	if (CMD_OPTION_SET('b')) {
		begin = strtol(CMD_OPTION_CHAR('b'), NULL, 10);
	}
	if (CMD_OPTION_SET('e')) {
		end = strtol(CMD_OPTION_CHAR('e'), NULL, 10);
	}

	/* return_source allocates memory for source */
	XG_DBG(breakpoints_allowed) = 0;
	source = return_source(filename, begin, end);
	XG_DBG(breakpoints_allowed) = 1;

	zend_string_release(filename);

	if (!source) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_CANT_OPEN_FILE);
	} else {
		xdebug_xml_add_text_ex(*retval, xdstrdup(source->d), source->l, 1, 1);
		xdebug_str_free(source);
	}
}

DBGP_FUNC(feature_get)
{
	xdebug_var_export_options *options;
	XDEBUG_STR_SWITCH_DECL;

	options = (xdebug_var_export_options*) context->options;

	if (!CMD_OPTION_SET('n')) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}
	xdebug_xml_add_attribute_ex(*retval, "feature_name", xdstrdup(CMD_OPTION_CHAR('n')), 0, 1);

	XDEBUG_STR_SWITCH(CMD_OPTION_CHAR('n')) {
		XDEBUG_STR_CASE("breakpoint_languages")
			xdebug_xml_add_attribute(*retval, "supported", "0");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("breakpoint_types")
			xdebug_xml_add_text(*retval, xdstrdup("line conditional call return exception"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("data_encoding")
			xdebug_xml_add_attribute(*retval, "supported", "0");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("encoding")
			xdebug_xml_add_text(*retval, xdstrdup("iso-8859-1"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("language_name")
			xdebug_xml_add_text(*retval, xdstrdup("PHP"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("language_supports_threads")
			xdebug_xml_add_text(*retval, xdstrdup("0"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("language_version")
			xdebug_xml_add_text(*retval, xdstrdup(PHP_VERSION));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("max_children")
			xdebug_xml_add_text(*retval, xdebug_sprintf("%ld", options->max_children));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("max_data")
			xdebug_xml_add_text(*retval, xdebug_sprintf("%ld", options->max_data));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("max_depth")
			xdebug_xml_add_text(*retval, xdebug_sprintf("%ld", options->max_depth));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("protocol_version")
			xdebug_xml_add_text(*retval, xdstrdup(DBGP_VERSION));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("supported_encodings")
			xdebug_xml_add_text(*retval, xdstrdup("iso-8859-1"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("supports_async")
			xdebug_xml_add_text(*retval, xdstrdup("0"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("supports_postmortem")
			xdebug_xml_add_text(*retval, xdstrdup("1"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("show_hidden")
			xdebug_xml_add_text(*retval, xdebug_sprintf("%ld", options->show_hidden));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("extended_properties")
			xdebug_xml_add_text(*retval, xdebug_sprintf("%ld", options->extended_properties));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("notify_ok")
			xdebug_xml_add_text(*retval, xdebug_sprintf("%ld", XG_DBG(context).send_notifications));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("resolved_breakpoints")
			xdebug_xml_add_text(*retval, xdebug_sprintf("%ld", XG_DBG(context).resolved_breakpoints));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE_DEFAULT
			xdebug_xml_add_text(*retval, xdstrdup(lookup_cmd(CMD_OPTION_CHAR('n')) ? "1" : "0"));
			xdebug_xml_add_attribute(*retval, "supported", lookup_cmd(CMD_OPTION_CHAR('n')) ? "1" : "0");
		XDEBUG_STR_CASE_DEFAULT_END
	}
}

DBGP_FUNC(feature_set)
{
	xdebug_var_export_options *options;
	XDEBUG_STR_SWITCH_DECL;

	options = (xdebug_var_export_options*) context->options;

	if (!CMD_OPTION_SET('n') || !CMD_OPTION_SET('v')) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	XDEBUG_STR_SWITCH(CMD_OPTION_CHAR('n')) {

		XDEBUG_STR_CASE("encoding")
			if (strcmp(CMD_OPTION_CHAR('v'), "iso-8859-1") != 0) {
				RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_ENCODING_NOT_SUPPORTED);
			}
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("max_children")
			options->max_children = strtol(CMD_OPTION_CHAR('v'), NULL, 10);
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("max_data")
			options->max_data = strtol(CMD_OPTION_CHAR('v'), NULL, 10);
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("max_depth")
			int i;
			options->max_depth = strtol(CMD_OPTION_CHAR('v'), NULL, 10);

			/* Reallocating page structure */
			xdfree(options->runtime);
			options->runtime = (xdebug_var_runtime_page*) xdmalloc(options->max_depth * sizeof(xdebug_var_runtime_page));
			for (i = 0; i < options->max_depth; i++) {
				options->runtime[i].page = 0;
				options->runtime[i].current_element_nr = 0;
			}
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("show_hidden")
			options->show_hidden = strtol(CMD_OPTION_CHAR('v'), NULL, 10);
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("multiple_sessions")
			/* FIXME: Add new boolean option check / struct field for this */
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("extended_properties")
			options->extended_properties = strtol(CMD_OPTION_CHAR('v'), NULL, 10);
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("notify_ok")
			XG_DBG(context).send_notifications = strtol(CMD_OPTION_CHAR('v'), NULL, 10);
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("resolved_breakpoints")
			XG_DBG(context).resolved_breakpoints = strtol(CMD_OPTION_CHAR('v'), NULL, 10);
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE_DEFAULT
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
		XDEBUG_STR_CASE_DEFAULT_END
	}
	xdebug_xml_add_attribute_ex(*retval, "feature", xdstrdup(CMD_OPTION_CHAR('n')), 0, 1);
	xdebug_xml_add_attribute_ex(*retval, "success", "1", 0, 0);
}

DBGP_FUNC(typemap_get)
{
	int              i;
	xdebug_xml_node *type;

	xdebug_xml_add_attribute(*retval, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	xdebug_xml_add_attribute(*retval, "xmlns:xsd", "http://www.w3.org/2001/XMLSchema");

	/* Add our basic types */
	for (i = 0; i < XDEBUG_TYPES_COUNT; i++) {
		type = xdebug_xml_node_init("map");
		xdebug_xml_add_attribute(type, "name", xdebug_dbgp_typemap[i][1]);
		xdebug_xml_add_attribute(type, "type", xdebug_dbgp_typemap[i][0]);
		if (xdebug_dbgp_typemap[i][2]) {
			xdebug_xml_add_attribute(type, "xsi:type", xdebug_dbgp_typemap[i][2]);
		}
		xdebug_xml_add_child(*retval, type);
	}
}

static int add_constant_node(xdebug_xml_node *node, xdebug_str *name, zval *const_val, xdebug_var_export_options *options)
{
	xdebug_xml_node *contents;

	contents = xdebug_get_zval_value_xml_node_ex(name, const_val, XDEBUG_VAR_TYPE_CONSTANT, options);
	if (!contents) {
		return FAILURE;
	}

	xdebug_xml_add_attribute(contents, "facet", "constant");
	xdebug_xml_add_child(node, contents);

	return SUCCESS;
}

static int add_variable_node(xdebug_xml_node *node, xdebug_str *name, int var_only, int non_null, int no_eval, xdebug_var_export_options *options)
{
	xdebug_xml_node *contents;

	contents = get_symbol(name, options);
	if (!contents) {
		return FAILURE;
	}

	xdebug_xml_add_child(node, contents);

	return SUCCESS;
}

static int xdebug_get_constant(xdebug_str *val, zval *const_val)
{
	zval *tmp_const = NULL;
	tmp_const = zend_get_constant_str(val->d, val->l);

	if (!tmp_const) {
		return 0;
	}

	*const_val = *tmp_const;

	return 1;
}

DBGP_FUNC(property_get)
{
	int                        depth = 0;
	int                        context_nr = 0;
	function_stack_entry      *fse;
	int                        old_max_data;
	xdebug_var_export_options *options = (xdebug_var_export_options*) context->options;

	if (!CMD_OPTION_SET('n')) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	if (CMD_OPTION_SET('d')) {
		depth = strtol(CMD_OPTION_CHAR('d'), NULL, 10);
	}

	if (CMD_OPTION_SET('c')) {
		context_nr = strtol(CMD_OPTION_CHAR('c'), NULL, 10);
	}

	/* Set the symbol table corresponding with the requested stack depth */
	if (context_nr == 0) { /* locals */
		if ((fse = xdebug_get_stack_frame(depth))) {
			function_stack_entry *old_fse = xdebug_get_stack_frame(depth - 1);

			if (depth > 0) {
				xdebug_lib_set_active_data(old_fse->execute_data);
			} else {
				xdebug_lib_set_active_data(EG(current_execute_data));
			}
			xdebug_lib_set_active_stack_entry(fse);
			xdebug_lib_set_active_symbol_table(fse->symbol_table);
		} else {
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
		}
	} else if (context_nr == 1) { /* superglobals */
		xdebug_lib_set_active_symbol_table(&EG(symbol_table));
	} else if (context_nr == 2) { /* constants */
		/* Do nothing */
	} else {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	if (CMD_OPTION_SET('p')) {
		options->runtime[0].page = strtol(CMD_OPTION_CHAR('p'), NULL, 10);
	} else {
		options->runtime[0].page = 0;
	}

	/* Override max data size if necessary */
	old_max_data = options->max_data;
	if (CMD_OPTION_SET('m')) {
		options->max_data= strtol(CMD_OPTION_CHAR('m'), NULL, 10);
	}

	if (context_nr == 2) { /* constants */
		zval const_val;

		if (!xdebug_get_constant(CMD_OPTION_XDEBUG_STR('n'), &const_val)) {
			options->max_data = old_max_data;
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_PROPERTY_NON_EXISTENT);
		}
		if (add_constant_node(*retval, CMD_OPTION_XDEBUG_STR('n'), &const_val, options) == FAILURE) {
			options->max_data = old_max_data;
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_PROPERTY_NON_EXISTENT);
		}
	} else {
		int add_var_retval;

		XG_DBG(context).inhibit_notifications = 1;
		add_var_retval = add_variable_node(*retval, CMD_OPTION_XDEBUG_STR('n'), 1, 0, 0, options);
		XG_DBG(context).inhibit_notifications = 0;

		if (add_var_retval) {
			options->max_data = old_max_data;
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_PROPERTY_NON_EXISTENT);
		}
	}
	options->max_data = old_max_data;
}

DBGP_FUNC(property_set)
{
	unsigned char             *new_value;
	size_t                     new_length = 0;
	int                        depth = 0;
	int                        context_nr = 0;
	int                        res;
	char                      *eval_string;
	const char                *cast_as;
	zval                       ret_zval;
	function_stack_entry      *fse;
	xdebug_var_export_options *options = (xdebug_var_export_options*) context->options;
	zend_execute_data         *original_execute_data;
	XDEBUG_STR_SWITCH_DECL;

	if (!CMD_OPTION_SET('n')) { /* name */
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	if (!CMD_OPTION_SET('-')) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	if (CMD_OPTION_SET('d')) { /* depth */
		depth = strtol(CMD_OPTION_CHAR('d'), NULL, 10);
	}

	if (CMD_OPTION_SET('c')) { /* context_id */
		context_nr = strtol(CMD_OPTION_CHAR('c'), NULL, 10);
	}

	/* Set the symbol table corresponding with the requested stack depth */
	if (context_nr == 0) { /* locals */
		if ((fse = xdebug_get_stack_frame(depth))) {
			function_stack_entry *old_fse = xdebug_get_stack_frame(depth - 1);

			if (depth > 0) {
				xdebug_lib_set_active_data(old_fse->execute_data);
			} else {
				xdebug_lib_set_active_data(EG(current_execute_data));
			}
			xdebug_lib_set_active_stack_entry(fse);
			xdebug_lib_set_active_symbol_table(fse->symbol_table);
		} else {
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
		}
	} else { /* superglobals */
		xdebug_lib_set_active_symbol_table(&EG(symbol_table));
	}

	if (CMD_OPTION_SET('p')) {
		options->runtime[0].page = strtol(CMD_OPTION_CHAR('p'), NULL, 10);
	} else {
		options->runtime[0].page = 0;
	}

	new_value = xdebug_base64_decode((unsigned char*) CMD_OPTION_CHAR('-'), CMD_OPTION_LEN('-'), &new_length);

	/* Set a cast, if requested through the 't' option */
	cast_as = "";

	if (CMD_OPTION_SET('t')) {
		XDEBUG_STR_SWITCH(CMD_OPTION_CHAR('t')) {
			XDEBUG_STR_CASE("bool")
				cast_as = "(bool) ";
			XDEBUG_STR_CASE_END

			XDEBUG_STR_CASE("int")
				cast_as = "(int) ";
			XDEBUG_STR_CASE_END

			XDEBUG_STR_CASE("float")
				cast_as = "(float) ";
			XDEBUG_STR_CASE_END

			XDEBUG_STR_CASE("string")
				cast_as = "(string) ";
			XDEBUG_STR_CASE_END

			XDEBUG_STR_CASE_DEFAULT
				xdebug_xml_add_attribute(*retval, "success", "0");
			XDEBUG_STR_CASE_DEFAULT_END
		}
	}

	/* backup executor state */
	if (depth > 0) {
		original_execute_data = EG(current_execute_data);

		EG(current_execute_data) = xdebug_lib_get_active_data();
	}

	/* Do the eval */
	eval_string = xdebug_sprintf("%s = %s %s", CMD_OPTION_CHAR('n'), cast_as, new_value);
	res = xdebug_do_eval(eval_string, &ret_zval);

	/* restore executor state */
	if (depth > 0) {
		EG(current_execute_data) = original_execute_data;
	}

	/* Free data */
	xdfree(eval_string);
	xdfree(new_value);

	/* Handle result */
	if (res == FAILURE) {
		/* don't send an error, send success = zero */
		xdebug_xml_add_attribute(*retval, "success", "0");
	} else {
		zval_dtor(&ret_zval);
		xdebug_xml_add_attribute(*retval, "success", "1");
	}
}

static int add_variable_contents_node(xdebug_xml_node *node, xdebug_str *name, int var_only, int non_null, int no_eval, xdebug_var_export_options *options)
{
	int contents_found;

	contents_found = get_symbol_contents(name, node, options);

	if (!contents_found) {
		return FAILURE;
	}

	return SUCCESS;
}

DBGP_FUNC(property_value)
{
	int                        depth = 0;
	int                        context_nr = 0;
	function_stack_entry      *fse;
	int                        old_max_data;
	xdebug_var_export_options *options = (xdebug_var_export_options*) context->options;

	if (!CMD_OPTION_SET('n')) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	if (CMD_OPTION_SET('d')) {
		depth = strtol(CMD_OPTION_CHAR('d'), NULL, 10);
	}

	if (CMD_OPTION_SET('c')) {
		context_nr = strtol(CMD_OPTION_CHAR('c'), NULL, 10);
	}

	/* Set the symbol table corresponding with the requested stack depth */
	if (context_nr == 0) { /* locals */
		if ((fse = xdebug_get_stack_frame(depth))) {
			function_stack_entry *old_fse = xdebug_get_stack_frame(depth - 1);

			if (depth > 0) {
				xdebug_lib_set_active_data(old_fse->execute_data);
			} else {
				xdebug_lib_set_active_data(EG(current_execute_data));
			}
			xdebug_lib_set_active_stack_entry(fse);
			xdebug_lib_set_active_symbol_table(fse->symbol_table);
		} else {
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
		}
	} else { /* superglobals */
		xdebug_lib_set_active_symbol_table(&EG(symbol_table));
	}

	if (CMD_OPTION_SET('p')) {
		options->runtime[0].page = strtol(CMD_OPTION_CHAR('p'), NULL, 10);
	} else {
		options->runtime[0].page = 0;
	}

	/* Override max data size if necessary */
	old_max_data = options->max_data;
	if (CMD_OPTION_SET('m')) {
		options->max_data = strtol(CMD_OPTION_CHAR('m'), NULL, 10);
	}
	if (options->max_data < 0) {
		options->max_data = old_max_data;
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}
	if (add_variable_contents_node(*retval, CMD_OPTION_XDEBUG_STR('n'), 1, 0, 0, options) == FAILURE) {
		options->max_data = old_max_data;
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_PROPERTY_NON_EXISTENT);
	}
	options->max_data = old_max_data;
}

static void attach_declared_var_with_contents(void *xml, xdebug_hash_element* he, void *options)
{
	xdebug_str         *name = (xdebug_str*) he->ptr;
	xdebug_xml_node    *node = (xdebug_xml_node *) xml;
	xdebug_xml_node    *contents;

	contents = get_symbol(name, options);
	if (!contents) {
		xdebug_var_xml_attach_uninitialized_var(options, node, name);
		return;
	}

	xdebug_xml_add_child(node, contents);
}

static int xdebug_add_filtered_symboltable_var(zval *symbol, int num_args, va_list args, zend_hash_key *hash_key)
{
	xdebug_hash *tmp_hash;

	tmp_hash = va_arg(args, xdebug_hash *);

	/* We really ought to deal properly with non-associate keys for symbol
	 * tables, but for now, we'll just ignore them. */
	if (!hash_key->key) { return 0; }

	if (hash_key->key->val[0] == '\0') { return 0; }

	if (strcmp("argc", hash_key->key->val) == 0) { return 0; }
	if (strcmp("argv", hash_key->key->val) == 0) { return 0; }
	if (hash_key->key->val[0] == '_') {
		if (strcmp("_COOKIE", hash_key->key->val) == 0) { return 0; }
		if (strcmp("_ENV", hash_key->key->val) == 0) { return 0; }
		if (strcmp("_FILES", hash_key->key->val) == 0) { return 0; }
		if (strcmp("_GET", hash_key->key->val) == 0) { return 0; }
		if (strcmp("_POST", hash_key->key->val) == 0) { return 0; }
		if (strcmp("_REQUEST", hash_key->key->val) == 0) { return 0; }
		if (strcmp("_SERVER", hash_key->key->val) == 0) { return 0; }
		if (strcmp("_SESSION", hash_key->key->val) == 0) { return 0; }
	}
	if (hash_key->key->val[0] == 'H') {
		if (strcmp("HTTP_COOKIE_VARS", hash_key->key->val) == 0) { return 0; }
		if (strcmp("HTTP_ENV_VARS", hash_key->key->val) == 0) { return 0; }
		if (strcmp("HTTP_GET_VARS", hash_key->key->val) == 0) { return 0; }
		if (strcmp("HTTP_POST_VARS", hash_key->key->val) == 0) { return 0; }
		if (strcmp("HTTP_POST_FILES", hash_key->key->val) == 0) { return 0; }
		if (strcmp("HTTP_RAW_POST_DATA", hash_key->key->val) == 0) { return 0; }
		if (strcmp("HTTP_SERVER_VARS", hash_key->key->val) == 0) { return 0; }
		if (strcmp("HTTP_SESSION_VARS", hash_key->key->val) == 0) { return 0; }
	}
	if (strcmp("GLOBALS", hash_key->key->val) == 0) { return 0; }

	xdebug_hash_add(tmp_hash, (char*) hash_key->key->val, hash_key->key->len, xdebug_str_create(hash_key->key->val, hash_key->key->len));

	return 0;
}

static int attach_context_vars(xdebug_xml_node *node, xdebug_var_export_options *options, long context_id, long depth, void (*func)(void *, xdebug_hash_element*, void*))
{
	function_stack_entry *fse;
	char                 *var_name;

	/* right now, we only have zero, one, or two with one being globals, which
	 * is always the head of the stack */
	if (context_id == 1) {
		/* add super globals */
		xdebug_lib_set_active_symbol_table(&EG(symbol_table));
		xdebug_lib_set_active_data(NULL);
		add_variable_node(node, XDEBUG_STR_WRAP_CHAR("_COOKIE"),  1, 1, 0, options);
		add_variable_node(node, XDEBUG_STR_WRAP_CHAR("_ENV"),     1, 1, 0, options);
		add_variable_node(node, XDEBUG_STR_WRAP_CHAR("_FILES"),   1, 1, 0, options);
		add_variable_node(node, XDEBUG_STR_WRAP_CHAR("_GET"),     1, 1, 0, options);
		add_variable_node(node, XDEBUG_STR_WRAP_CHAR("_POST"),    1, 1, 0, options);
		add_variable_node(node, XDEBUG_STR_WRAP_CHAR("_REQUEST"), 1, 1, 0, options);
		add_variable_node(node, XDEBUG_STR_WRAP_CHAR("_SERVER"),  1, 1, 0, options);
		add_variable_node(node, XDEBUG_STR_WRAP_CHAR("_SESSION"), 1, 1, 0, options);
		add_variable_node(node, XDEBUG_STR_WRAP_CHAR("GLOBALS"),  1, 1, 0, options);
		xdebug_lib_set_active_symbol_table(NULL);
		return 0;
	}

	/* add user defined constants */
	if (context_id == 2) {
		zend_constant *val;

		ZEND_HASH_FOREACH_PTR(EG(zend_constants), val) {
			xdebug_str *tmp_name;

			if (!val->name) {
				/* skip special constants */
				continue;
			}

			if (XDEBUG_ZEND_CONSTANT_MODULE_NUMBER(val) != PHP_USER_CONSTANT) {
				/* we're only interested in user defined constants */
				continue;
			}

			tmp_name = xdebug_str_create(val->name->val, val->name->len);
			add_constant_node(node, tmp_name, &(val->value), options);
			xdebug_str_free(tmp_name);
		} ZEND_HASH_FOREACH_END();

		return 0;
	}

	/* Here the context_id is 0 */
	if ((fse = xdebug_get_stack_frame(depth))) {
		function_stack_entry *old_fse = xdebug_get_stack_frame(depth - 1);

		if (depth > 0) {
			xdebug_lib_set_active_data(old_fse->execute_data);
		} else {
			xdebug_lib_set_active_data(EG(current_execute_data));
		}
		xdebug_lib_set_active_symbol_table(fse->symbol_table);

		/* Only show vars when they are scanned */
		if (fse->declared_vars) {
			xdebug_hash *tmp_hash;

			/* Get a hash from all the used vars (which can have duplicates) */
			tmp_hash = xdebug_declared_var_hash_from_llist(fse->declared_vars);

			/* Check for dynamically defined variables, but make sure we don't already
			 * have them. Also exclude superglobals and argv/argc */
			if (xdebug_lib_has_active_symbol_table()) {
				zend_hash_apply_with_arguments(xdebug_lib_get_active_symbol_table(), (apply_func_args_t) xdebug_add_filtered_symboltable_var, 1, tmp_hash);
			}

			/* Add all the found variables to the node */
			xdebug_hash_apply_with_argument(tmp_hash, (void *) node, func, (void *) options);

			/* Zend engine 2 does not give us $this, eval so we can get it */
			if (!xdebug_hash_find(tmp_hash, "this", 4, (void *) &var_name)) {
				add_variable_node(node, XDEBUG_STR_WRAP_CHAR("this"), 1, 1, 0, options);
			}

			xdebug_hash_destroy(tmp_hash);
		}

		/* Check for static variables and constants, but only if it's a static
		 * method call as we attach constants and static properties to "this"
		 * too normally. */
		if (fse->function.type == XFUNC_STATIC_MEMBER) {
			zend_class_entry *ce = zend_fetch_class(fse->function.class_name, ZEND_FETCH_CLASS_DEFAULT);

#if PHP_VERSION_ID >= 70400
			if (ce->type == ZEND_INTERNAL_CLASS || (ce->ce_flags & ZEND_ACC_IMMUTABLE)) {
				zend_class_init_statics(ce);
			}
#endif

			xdebug_var_xml_attach_static_vars(node, options, ce);
		}

		xdebug_lib_set_active_data(NULL);
		xdebug_lib_set_active_symbol_table(NULL);
		return 0;
	}

	return 1;
}

DBGP_FUNC(stack_depth)
{
	xdebug_xml_add_attribute_ex(*retval, "depth", xdebug_sprintf("%lu", XG_BASE(level)), 0, 1);
}

DBGP_FUNC(stack_get)
{
	xdebug_xml_node *stackframe;
	long             depth;

	if (CMD_OPTION_SET('d')) {
		depth = strtol(CMD_OPTION_CHAR('d'), NULL, 10);
		if (depth >= 0 && depth < (long) XG_BASE(level)) {
			stackframe = return_stackframe(depth);
			xdebug_xml_add_child(*retval, stackframe);
		} else {
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
		}
	} else {
		function_stack_entry *fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));
		int                   i = 0;

		for (i = 0; i < XDEBUG_VECTOR_COUNT(XG_BASE(stack)); i++, fse--) {
			stackframe = return_stackframe(i);
			xdebug_xml_add_child(*retval, stackframe);
		}
	}
}

DBGP_FUNC(status)
{
	xdebug_xml_add_attribute(*retval, "status", xdebug_dbgp_status_strings[XG_DBG(status)]);
	xdebug_xml_add_attribute(*retval, "reason", xdebug_dbgp_reason_strings[XG_DBG(reason)]);
}


DBGP_FUNC(context_names)
{
	xdebug_xml_node *child;

	child = xdebug_xml_node_init("context");
	xdebug_xml_add_attribute(child, "name", "Locals");
	xdebug_xml_add_attribute(child, "id", "0");
	xdebug_xml_add_child(*retval, child);

	child = xdebug_xml_node_init("context");
	xdebug_xml_add_attribute(child, "name", "Superglobals");
	xdebug_xml_add_attribute(child, "id", "1");
	xdebug_xml_add_child(*retval, child);

	child = xdebug_xml_node_init("context");
	xdebug_xml_add_attribute(child, "name", "User defined constants");
	xdebug_xml_add_attribute(child, "id", "2");
	xdebug_xml_add_child(*retval, child);
}

DBGP_FUNC(context_get)
{
	int                        res;
	int                        context_id = 0;
	int                        depth = 0;
	xdebug_var_export_options *options = (xdebug_var_export_options*) context->options;

	if (CMD_OPTION_SET('c')) {
		context_id = atol(CMD_OPTION_CHAR('c'));
	}
	if (CMD_OPTION_SET('d')) {
		depth = atol(CMD_OPTION_CHAR('d'));
	}
	/* Always reset to page = 0, as it might have been modified by property_get or property_value */
	options->runtime[0].page = 0;

	res = attach_context_vars(*retval, options, context_id, depth, attach_declared_var_with_contents);
	switch (res) {
		case 1:
			RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
			break;
	}

	xdebug_xml_add_attribute_ex(*retval, "context", xdebug_sprintf("%d", context_id), 0, 1);
}

DBGP_FUNC(xcmd_profiler_name_get)
{
	char *filename = xdebug_get_profiler_filename();

	if (!filename) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_PROFILING_NOT_STARTED);
	}

	xdebug_xml_add_text(*retval, xdstrdup(filename));
}

DBGP_FUNC(xcmd_get_executable_lines)
{
	function_stack_entry *fse;
	unsigned int          i;
	long                  depth;
	xdebug_xml_node      *lines, *line;

	if (!CMD_OPTION_SET('d')) {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	depth = strtol(CMD_OPTION_CHAR('d'), NULL, 10);
	if (depth >= 0 && depth < (long) XG_BASE(level)) {
		fse = xdebug_get_stack_frame(depth);
	} else {
		RETURN_RESULT(XG_DBG(status), XG_DBG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
	}

	lines = xdebug_xml_node_init("xdebug:lines");
	for (i = 0; i < fse->op_array->last; i++ ) {
		if (fse->op_array->opcodes[i].opcode == ZEND_EXT_STMT ) {
			line = xdebug_xml_node_init("xdebug:line");
			xdebug_xml_add_attribute_ex(line, "lineno", xdebug_sprintf("%lu", fse->op_array->opcodes[i].lineno), 0, 1);
			xdebug_xml_add_child(lines, line);
		}
	}
	xdebug_xml_add_child(*retval, lines);
}


/*****************************************************************************
** Parsing functions
*/

/* {{{ Constants for state machine */
#define STATE_NORMAL                   0
#define STATE_QUOTED                   1
#define STATE_OPT_FOLLOWS              2
#define STATE_SEP_FOLLOWS              3
#define STATE_VALUE_FOLLOWS_FIRST_CHAR 4
#define STATE_VALUE_FOLLOWS            5
#define STATE_SKIP_CHAR                6
#define STATE_ESCAPED_CHAR_FOLLOWS     7
/* }}} */

static void xdebug_dbgp_arg_dtor(xdebug_dbgp_arg *arg)
{
	int i;

	for (i = 0; i < 27; i++) {
		if (arg->value[i]) {
			xdebug_str_free(arg->value[i]);
		}
	}
	xdfree(arg);
}

static int xdebug_dbgp_parse_cmd(char *line, char **cmd, xdebug_dbgp_arg **ret_args)
{
	xdebug_dbgp_arg *args = NULL;
	char *ptr;
	int   state;
	int   charescaped = 0;
	char  opt = ' ', *value_begin = NULL;

	args = xdmalloc(sizeof (xdebug_dbgp_arg));
	memset(args->value, 0, sizeof(args->value));
	*cmd = NULL;

	/* Find the end of the command, this is always on the first space */
	ptr = strchr(line, ' ');
	if (!ptr) {
		/* No space found. If the line is not empty, return the line
		 * and assume it only consists of the command name. If the line
		 * is 0 chars long, we return a failure. */
		if (strlen(line)) {
			*cmd = strdup(line);
			*ret_args = args;
			return XDEBUG_ERROR_OK;
		} else {
			goto parse_error;
		}
	} else {
		/* A space was found, so we copy everything before it
		 * into the cmd parameter. */
		*cmd = xdcalloc(1, ptr - line + 1);
		memcpy(*cmd, line, ptr - line);
	}
	/* Now we loop until we find the end of the string, which is the \0
	 * character */
	state = STATE_NORMAL;
	do {
		ptr++;
		switch (state) {
			case STATE_NORMAL:
				if (*ptr != '-') {
					goto parse_error;
				} else {
					state = STATE_OPT_FOLLOWS;
				}
				break;
			case STATE_OPT_FOLLOWS:
				opt = *ptr;
				state = STATE_SEP_FOLLOWS;
				break;
			case STATE_SEP_FOLLOWS:
				if (*ptr != ' ') {
					goto parse_error;
				} else {
					state = STATE_VALUE_FOLLOWS_FIRST_CHAR;
					value_begin = ptr + 1;
				}
				break;
			case STATE_VALUE_FOLLOWS_FIRST_CHAR:
				if (*ptr == '"' && opt != '-') {
					value_begin = ptr + 1;
					state = STATE_QUOTED;
				} else {
					state = STATE_VALUE_FOLLOWS;
				}
				break;
			case STATE_VALUE_FOLLOWS:
				if ((*ptr == ' ' && opt != '-') || *ptr == '\0') {
					int opt_index = opt - 'a';

					if (opt == '-') {
						opt_index = 26;
					}

					if (!args->value[opt_index]) {
						args->value[opt_index] = xdebug_str_create(value_begin, ptr - value_begin);
						state = STATE_NORMAL;
					} else {
						goto duplicate_opts;
					}
				}
				break;
			case STATE_QUOTED:
				if (*ptr == '\\') {
					state = STATE_ESCAPED_CHAR_FOLLOWS;
				} else
				if (*ptr == '"') {
					int opt_index = opt - 'a';

					if (charescaped) {
						charescaped = 0;
						break;
					}
					if (opt == '-') {
						opt_index = 26;
					}

					if (!args->value[opt_index]) {
						int len = ptr - value_begin;

						args->value[opt_index] = xdebug_str_create(value_begin, len);
						xdebug_stripcslashes(args->value[opt_index]->d, &len);
						args->value[opt_index]->l = len;

						state = STATE_SKIP_CHAR;
					} else {
						goto duplicate_opts;
					}
				}
				break;
			case STATE_SKIP_CHAR:
				state = STATE_NORMAL;
				break;
			case STATE_ESCAPED_CHAR_FOLLOWS:
				state = STATE_QUOTED;
				break;

		}
	} while (*ptr);
	*ret_args = args;
	return XDEBUG_ERROR_OK;

parse_error:
	*ret_args = args;
	return XDEBUG_ERROR_PARSE;

duplicate_opts:
	*ret_args = args;
	return XDEBUG_ERROR_DUP_ARG;
}

static int xdebug_dbgp_parse_option(xdebug_con *context, char* line, int flags, xdebug_xml_node *retval)
{
	char *cmd = NULL;
	int res, ret = 0;
	xdebug_dbgp_arg *args;
	xdebug_dbgp_cmd *command;
	xdebug_xml_node *error;

	xdebug_log(XLOG_CHAN_DEBUG, XLOG_COM, "<- %s", line);
	res = xdebug_dbgp_parse_cmd(line, (char**) &cmd, (xdebug_dbgp_arg**) &args);

	/* Add command name to return packet */
	if (cmd) {
		/* if no cmd res will be XDEBUG_ERROR_PARSE */
		xdebug_xml_add_attribute_ex(retval, "command", xdstrdup(cmd), 0, 1);
	}

	/* Handle missing transaction ID, and if it exist add it to the result */
	if (!CMD_OPTION_SET('i')) {
		/* we need the transaction_id even for errors in parse_cmd, but if
		   we error out here, just force the error to happen below */
		res = XDEBUG_ERROR_INVALID_ARGS;
	} else {
		xdebug_xml_add_attribute_ex(retval, "transaction_id", xdstrdup(CMD_OPTION_CHAR('i')), 0, 1);
	}

	/* Handle parse errors */
	/* FIXME: use RETURN_RESULT here too */
	if (res != XDEBUG_ERROR_OK) {
		error = xdebug_xml_node_init("error");
		xdebug_xml_add_attribute_ex(error, "code", xdebug_sprintf("%lu", res), 0, 1);
		xdebug_xml_add_child(retval, error);
		ADD_REASON_MESSAGE(res);
	} else {

		/* Execute commands and stuff */
		command = lookup_cmd(cmd);

		if (command) {
			if (command->cont) {
				XG_DBG(status) = DBGP_STATUS_RUNNING;
				XG_DBG(reason) = DBGP_REASON_OK;
			}
			XG_DBG(lastcmd) = command->name;
			if (XG_DBG(lasttransid)) {
				xdfree(XG_DBG(lasttransid));
			}
			XG_DBG(lasttransid) = xdstrdup(CMD_OPTION_CHAR('i'));
			if (
				XG_DBG(status) != DBGP_STATUS_STOPPING
				||
				(XG_DBG(status) == DBGP_STATUS_STOPPING && command->flags & XDEBUG_DBGP_POST_MORTEM))
			{
				command->handler((xdebug_xml_node**) &retval, context, args);
				ret = command->cont;
			} else {
				error = xdebug_xml_node_init("error");
				xdebug_xml_add_attribute_ex(error, "code", xdebug_sprintf("%lu", XDEBUG_ERROR_COMMAND_UNAVAILABLE), 0, 1);
				ADD_REASON_MESSAGE(XDEBUG_ERROR_COMMAND_UNAVAILABLE);
				xdebug_xml_add_child(retval, error);

				ret = -1;
			}
		} else {
			error = xdebug_xml_node_init("error");
			xdebug_xml_add_attribute_ex(error, "code", xdebug_sprintf("%lu", XDEBUG_ERROR_UNIMPLEMENTED), 0, 1);
			ADD_REASON_MESSAGE(XDEBUG_ERROR_UNIMPLEMENTED);
			xdebug_xml_add_child(retval, error);

			ret = -1;
		}
	}

	xdfree(cmd);
	xdebug_dbgp_arg_dtor(args);
	return ret;
}

/*****************************************************************************
** Handlers for debug functions
*/
#define READ_BUFFER_SIZE 128

#define FD_RL_FILE    0
#define FD_RL_SOCKET  1


static char* xdebug_fd_read_line_delim(int socketfd, fd_buf *context, int type, unsigned char delim, int *length)
{
	int size = 0, newl = 0, nbufsize = 0;
	char *tmp;
	char *tmp_buf = NULL;
	char *ptr;
	char buffer[READ_BUFFER_SIZE + 1];

	if (!context->buffer) {
		context->buffer = calloc(1,1);
		context->buffer_size = 0;
	}

	while (context->buffer_size < 1 || context->buffer[context->buffer_size - 1] != delim) {
		ptr = context->buffer + context->buffer_size;
		if (type == FD_RL_FILE) {
			newl = read(socketfd, buffer, READ_BUFFER_SIZE);
		} else {
			newl = recv(socketfd, buffer, READ_BUFFER_SIZE, 0);
		}
		if (newl > 0) {
			context->buffer = realloc(context->buffer, context->buffer_size + newl + 1);
			memcpy(context->buffer + context->buffer_size, buffer, newl);
			context->buffer_size += newl;
			context->buffer[context->buffer_size] = '\0';
		} else if (newl == -1 && errno == EINTR) {
			continue;
		} else {
			free(context->buffer);
			context->buffer = NULL;
			context->buffer_size = 0;
			return NULL;
		}
	}

	ptr = memchr(context->buffer, delim, context->buffer_size);
	size = ptr - context->buffer;
	/* Copy that line into tmp */
	tmp = malloc(size + 1);
	tmp[size] = '\0';
	memcpy(tmp, context->buffer, size);
	/* Rewrite existing buffer */
	if ((nbufsize = context->buffer_size - size - 1)  > 0) {
		tmp_buf = malloc(nbufsize + 1);
		memcpy(tmp_buf, ptr + 1, nbufsize);
		tmp_buf[nbufsize] = 0;
	}
	free(context->buffer);
	context->buffer = tmp_buf;
	context->buffer_size = context->buffer_size - (size + 1);

	/* Return normal line */
	if (length) {
		*length = size;
	}
	return tmp;
}

static int xdebug_dbgp_cmdloop(xdebug_con *context, int bail)
{
	char *option;
	int   length;
	int   ret;
	xdebug_xml_node *response;

	do {
		length = 0;

		option = xdebug_fd_read_line_delim(context->socket, context->buffer, FD_RL_SOCKET, '\0', &length);
		if (!option) {
			return 0;
		}

		response = xdebug_xml_node_init("response");
		xdebug_xml_add_attribute(response, "xmlns", "urn:debugger_protocol_v1");
		xdebug_xml_add_attribute(response, "xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");
		ret = xdebug_dbgp_parse_option(context, option, 0, response);
		if (ret != 1) {
			send_message(context, response);
		}
		xdebug_xml_node_dtor(response);

		free(option);
	} while (0 == ret);

	if (bail && XG_DBG(status) == DBGP_STATUS_STOPPED) {
		_zend_bailout((char*)__FILE__, __LINE__);
	}
	return ret;

}

int xdebug_dbgp_init(xdebug_con *context, int mode)
{
	xdebug_var_export_options *options;
	xdebug_xml_node *response, *child;
	int i;

	/* initialize our status information */
	if (mode == XDEBUG_REQ) {
		XG_DBG(status) = DBGP_STATUS_STARTING;
		XG_DBG(reason) = DBGP_REASON_OK;
	} else if (mode == XDEBUG_JIT) {
		XG_DBG(status) = DBGP_STATUS_BREAK;
		XG_DBG(reason) = DBGP_REASON_ERROR;
	}
	XG_DBG(lastcmd) = NULL;
	XG_DBG(lasttransid) = NULL;

	response = xdebug_xml_node_init("init");
	xdebug_xml_add_attribute(response, "xmlns", "urn:debugger_protocol_v1");
	xdebug_xml_add_attribute(response, "xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");

/* {{{ XML Init Stuff*/
	child = xdebug_xml_node_init("engine");
	xdebug_xml_add_attribute(child, "version", XDEBUG_VERSION);
	xdebug_xml_add_text(child, xdstrdup(XDEBUG_NAME));
	xdebug_xml_add_child(response, child);

	child = xdebug_xml_node_init("author");
	xdebug_xml_add_text(child, xdstrdup(XDEBUG_AUTHOR));
	xdebug_xml_add_child(response, child);

	child = xdebug_xml_node_init("url");
	xdebug_xml_add_text(child, xdstrdup(XDEBUG_URL));
	xdebug_xml_add_child(response, child);

	child = xdebug_xml_node_init("copyright");
	xdebug_xml_add_text(child, xdstrdup(XDEBUG_COPYRIGHT));
	xdebug_xml_add_child(response, child);

	if (zend_string_equals_literal(context->program_name, "-") || zend_string_equals_literal(context->program_name, "Command line code")) {
		xdebug_xml_add_attribute_ex(response, "fileuri", xdstrdup("dbgp://stdin"), 0, 1);
	} else {
		xdebug_xml_add_attribute_ex(response, "fileuri", xdebug_path_to_url(context->program_name), 0, 1);
	}
	xdebug_xml_add_attribute_ex(response, "language", "PHP", 0, 0);
	xdebug_xml_add_attribute_ex(response, "xdebug:language_version", PHP_VERSION, 0, 0);
	xdebug_xml_add_attribute_ex(response, "protocol_version", DBGP_VERSION, 0, 0);
	xdebug_xml_add_attribute_ex(response, "appid", xdebug_sprintf(ZEND_ULONG_FMT, xdebug_get_pid()), 0, 1);

	if (getenv("DBGP_COOKIE")) {
		xdebug_xml_add_attribute_ex(response, "session", xdstrdup(getenv("DBGP_COOKIE")), 0, 1);
	}

	if (XINI_DBG(cloud_id) && *XINI_DBG(cloud_id)) {
		xdebug_xml_add_attribute_ex(response, "xdebug:userid", xdstrdup(XINI_DBG(cloud_id)), 0, 1);
	}
	if (XG_DBG(ide_key) && *XG_DBG(ide_key)) {
		xdebug_xml_add_attribute_ex(response, "idekey", xdstrdup(XG_DBG(ide_key)), 0, 1);
	}

	context->buffer = xdmalloc(sizeof(fd_buf));
	context->buffer->buffer = NULL;
	context->buffer->buffer_size = 0;

	send_message_ex(context, response, DBGP_STATUS_STARTING);
	xdebug_xml_node_dtor(response);
/* }}} */

	context->options = xdmalloc(sizeof(xdebug_var_export_options));
	options = (xdebug_var_export_options*) context->options;
	options->max_children = 32;
	options->max_data     = 1024;
	options->max_depth    = 1;
	options->show_hidden  = 0;
	options->extended_properties         = 0;
	options->encode_as_extended_property = 0;
	options->runtime = (xdebug_var_runtime_page*) xdmalloc((options->max_depth + 1) * sizeof(xdebug_var_runtime_page));
	for (i = 0; i < options->max_depth; i++) {
		options->runtime[i].page = 0;
		options->runtime[i].current_element_nr = 0;
	}

	context->breakpoint_list = xdebug_hash_alloc(64, (xdebug_hash_dtor_t) xdebug_hash_admin_dtor);
	context->function_breakpoints = xdebug_hash_alloc(64, (xdebug_hash_dtor_t) xdebug_hash_brk_dtor);
	context->exception_breakpoints = xdebug_hash_alloc(64, (xdebug_hash_dtor_t) xdebug_hash_brk_dtor);
	context->line_breakpoints = xdebug_llist_alloc((xdebug_llist_dtor) xdebug_llist_brk_dtor);
	context->eval_id_lookup = xdebug_hash_alloc(64, (xdebug_hash_dtor_t) xdebug_hash_eval_info_dtor);
	context->eval_id_sequence = 0;
	context->send_notifications = 0;
	context->inhibit_notifications = 0;
	context->resolved_breakpoints = 0;

	xdebug_mark_debug_connection_active();
	xdebug_dbgp_cmdloop(context, XDEBUG_CMDLOOP_BAIL);

	return 1;
}

int xdebug_dbgp_deinit(xdebug_con *context)
{
	xdebug_xml_node           *response;
	xdebug_var_export_options *options;
	int                        detaching = (XG_DBG(status) == DBGP_STATUS_DETACHED);

	if (xdebug_is_debug_connection_active()) {
		XG_DBG(status) = DBGP_STATUS_STOPPING;
		XG_DBG(reason) = DBGP_REASON_OK;
		response = xdebug_xml_node_init("response");
		xdebug_xml_add_attribute(response, "xmlns", "urn:debugger_protocol_v1");
		xdebug_xml_add_attribute(response, "xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");
		/* lastcmd and lasttransid are not always set (for example when the
		 * connection is severed before the first command is send) */
		if (XG_DBG(lastcmd) && XG_DBG(lasttransid)) {
			xdebug_xml_add_attribute_ex(response, "command", XG_DBG(lastcmd), 0, 0);
			xdebug_xml_add_attribute_ex(response, "transaction_id", XG_DBG(lasttransid), 0, 0);
		}
		xdebug_xml_add_attribute_ex(response, "status", xdebug_dbgp_status_strings[XG_DBG(status)], 0, 0);
		xdebug_xml_add_attribute_ex(response, "reason", xdebug_dbgp_reason_strings[XG_DBG(reason)], 0, 0);

		send_message(context, response);
		xdebug_xml_node_dtor(response);

		if (!detaching) {
			xdebug_dbgp_cmdloop(context, XDEBUG_CMDLOOP_NONBAIL);
		}
	}

	if (xdebug_is_debug_connection_active()) {
		options = (xdebug_var_export_options*) context->options;
		xdfree(options->runtime);
		xdfree(context->options);
		xdebug_hash_destroy(context->function_breakpoints);
		xdebug_hash_destroy(context->exception_breakpoints);
		xdebug_hash_destroy(context->eval_id_lookup);
		xdebug_llist_destroy(context->line_breakpoints, NULL);
		xdebug_hash_destroy(context->breakpoint_list);
		xdfree(context->buffer);
		context->buffer = NULL;
	}

	if (XG_DBG(lasttransid)) {
		xdfree(XG_DBG(lasttransid));
		XG_DBG(lasttransid) = NULL;
	}
	xdebug_mark_debug_connection_not_active();
	return 1;
}

int xdebug_dbgp_error(xdebug_con *context, int type, char *exception_type, char *message, const char *location, const unsigned int line, xdebug_vector *stack)
{
	char               *errortype;
	xdebug_xml_node     *response, *error;

	if (exception_type) {
		errortype = exception_type;
	} else {
		errortype = xdebug_error_type(type);
	}

	if (exception_type) {
		XG_DBG(status) = DBGP_STATUS_BREAK;
		XG_DBG(reason) = DBGP_REASON_EXCEPTION;
	} else {
		switch (type) {
			case E_CORE_ERROR:
			/* no break - intentionally */
			case E_ERROR:
			/*case E_PARSE: the parser would return 1 (failure), we can bail out nicely */
			case E_COMPILE_ERROR:
			case E_USER_ERROR:
				XG_DBG(status) = DBGP_STATUS_STOPPING;
				XG_DBG(reason) = DBGP_REASON_ABORTED;
				break;
			default:
				XG_DBG(status) = DBGP_STATUS_BREAK;
				XG_DBG(reason) = DBGP_REASON_ERROR;
		}
	}
/*
	runtime_allowed = (
		(type != E_ERROR) &&
		(type != E_CORE_ERROR) &&
		(type != E_COMPILE_ERROR) &&
		(type != E_USER_ERROR)
	) ? XDEBUG_BREAKPOINT | XDEBUG_RUNTIME : 0;
*/

	response = xdebug_xml_node_init("response");
	xdebug_xml_add_attribute(response, "xmlns", "urn:debugger_protocol_v1");
	xdebug_xml_add_attribute(response, "xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");
	/* lastcmd and lasttransid are not always set (for example when the
	 * connection is severed before the first command is send) */
	if (XG_DBG(lastcmd) && XG_DBG(lasttransid)) {
		xdebug_xml_add_attribute_ex(response, "command", XG_DBG(lastcmd), 0, 0);
		xdebug_xml_add_attribute_ex(response, "transaction_id", XG_DBG(lasttransid), 0, 0);
	}
	xdebug_xml_add_attribute(response, "status", xdebug_dbgp_status_strings[XG_DBG(status)]);
	xdebug_xml_add_attribute(response, "reason", xdebug_dbgp_reason_strings[XG_DBG(reason)]);

	error = xdebug_xml_node_init("error");
	xdebug_xml_add_attribute_ex(error, "code", xdebug_sprintf("%lu", type), 0, 1);
	xdebug_xml_add_attribute_ex(error, "exception", xdstrdup(errortype), 0, 1);
	xdebug_xml_add_text(error, xdstrdup(message));
	xdebug_xml_add_child(response, error);

	send_message(context, response);
	xdebug_xml_node_dtor(response);
	if (!exception_type) {
		xdfree(errortype);
	}

	xdebug_dbgp_cmdloop(context, XDEBUG_CMDLOOP_BAIL);

	return 1;
}

int xdebug_dbgp_break_on_line(xdebug_con *context, xdebug_brk_info *brk, zend_string *filename, int lineno)
{
	char *tmp_file     = ZSTR_VAL(filename);
	int   tmp_file_len = ZSTR_LEN(filename);

	xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "Checking whether to break on %s:%d.", ZSTR_VAL(brk->filename), brk->resolved_lineno);

	if (brk->disabled) {
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "R: Breakpoint is disabled.");
		return 0;
	}

	xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "I: Current location: %s:%d.", tmp_file, lineno);

	if (is_dbgp_url(brk->filename) && check_evaled_code(filename, &tmp_file)) {
		tmp_file_len = strlen(tmp_file);
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "I: Found eval code for '%s': %s.", ZSTR_VAL(filename), tmp_file);
	}

	xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "I: Matching breakpoint '%s:%d' against location '%s:%d'.", ZSTR_VAL(brk->filename), brk->resolved_lineno, tmp_file, lineno);

	if (ZSTR_LEN(brk->filename) != tmp_file_len) {
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "R: File name length (%d) doesn't match with breakpoint (%zd).", tmp_file_len, ZSTR_LEN(brk->filename));
		return 0;
	}

	if (brk->resolved_lineno != lineno) {
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "R: Line number (%d) doesn't match with breakpoint (%d).", lineno, brk->resolved_lineno);
		return 0;
	}

	if (strncasecmp(ZSTR_VAL(brk->filename), tmp_file, ZSTR_LEN(brk->filename)) == 0) {
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "F: File names match (%s).", ZSTR_VAL(brk->filename));
		return 1;
	}

	xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "R: File names (%s) doesn't match with breakpoint (%s).", tmp_file, ZSTR_VAL(brk->filename));
	return 0;
}

int xdebug_dbgp_breakpoint(xdebug_con *context, xdebug_vector *stack, zend_string *filename, long lineno, int type, char *exception, char *code, const char *message)
{
	xdebug_xml_node *response, *error_container;

	XG_DBG(status) = DBGP_STATUS_BREAK;
	XG_DBG(reason) = DBGP_REASON_OK;

	response = xdebug_xml_node_init("response");
	xdebug_xml_add_attribute(response, "xmlns", "urn:debugger_protocol_v1");
	xdebug_xml_add_attribute(response, "xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");
	/* lastcmd and lasttransid are not always set (for example when the
	 * connection is severed before the first command is send) */
	if (XG_DBG(lastcmd) && XG_DBG(lasttransid)) {
		xdebug_xml_add_attribute_ex(response, "command", XG_DBG(lastcmd), 0, 0);
		xdebug_xml_add_attribute_ex(response, "transaction_id", XG_DBG(lasttransid), 0, 0);
	}
	xdebug_xml_add_attribute(response, "status", xdebug_dbgp_status_strings[XG_DBG(status)]);
	xdebug_xml_add_attribute(response, "reason", xdebug_dbgp_reason_strings[XG_DBG(reason)]);

	error_container = xdebug_xml_node_init("xdebug:message");
	if (filename) {
		char *tmp_filename = NULL;

		if (check_evaled_code(filename, &tmp_filename)) {
			xdebug_xml_add_attribute_ex(error_container, "filename", tmp_filename, 0, 0);
		} else {
			xdebug_xml_add_attribute_ex(error_container, "filename", xdebug_path_to_url(filename), 0, 1);
		}
	}
	if (lineno) {
		xdebug_xml_add_attribute_ex(error_container, "lineno", xdebug_sprintf("%lu", lineno), 0, 1);
	}
	if (exception) {
		xdebug_xml_add_attribute_ex(error_container, "exception", xdstrdup(exception), 0, 1);
	}
	if (code) {
		xdebug_xml_add_attribute_ex(error_container, "code", xdstrdup(code), 0, 1);
	}
	if (message) {
		xdebug_xml_add_text(error_container, xdstrdup(message));
	}
	xdebug_xml_add_child(response, error_container);

	send_message(context, response);
	xdebug_xml_node_dtor(response);

	XG_DBG(lastcmd) = NULL;
	if (XG_DBG(lasttransid)) {
		xdfree(XG_DBG(lasttransid));
		XG_DBG(lasttransid) = NULL;
	}

	xdebug_dbgp_cmdloop(context, XDEBUG_CMDLOOP_BAIL);

	return xdebug_is_debug_connection_active();
}

static int xdebug_dbgp_resolved_breakpoint_notification(xdebug_con *context, xdebug_brk_info *brk_info)
{
	xdebug_xml_node *response, *child;

	if (!context->send_notifications) {
		return 0;
	}

	response = xdebug_xml_node_init("notify");
	xdebug_xml_add_attribute(response, "xmlns", "urn:debugger_protocol_v1");
	xdebug_xml_add_attribute(response, "xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");
	xdebug_xml_add_attribute(response, "name", "breakpoint_resolved");

	child = xdebug_xml_node_init("breakpoint");
	breakpoint_brk_info_add(child, brk_info);
	xdebug_xml_add_child(response, child);

	send_message(context, response);
	xdebug_xml_node_dtor(response);

	return 1;
}

/*
static void function_breakpoint_resolve_helper(void *rctxt, xdebug_brk_info *brk_info, xdebug_hash_element *he)
{
	xdebug_dbgp_resolve_context *ctxt = (xdebug_dbgp_resolve_context*) rctxt;

	if (brk_info->resolved == XDEBUG_BRK_RESOLVED) {
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "R: %s breakpoint for '%s' has already been resolved.",
			XDEBUG_BREAKPOINT_TYPE_NAME(brk_info->brk_type), ctxt->fse->function.function);
		return;
	}

	if (ctxt->fse->function.type == XFUNC_NORMAL) {
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "I: '%s' is a normal function (%02x).", ctxt->fse->function.function, ctxt->fse->function.type);

		if (strcmp(ctxt->fse->function.function, brk_info->functionname) == 0) {
			xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "F: Breakpoint function (%s) matches current function (%s).", brk_info->functionname, ctxt->fse->function.function);
			brk_info->resolved = XDEBUG_BRK_RESOLVED;
			xdebug_dbgp_resolved_breakpoint_notification(ctxt->context, brk_info);
			return;
		}
	} else if (ctxt->fse->function.type == XFUNC_MEMBER || ctxt->fse->function.type == XFUNC_STATIC_MEMBER) {
		char  *tmp_name = NULL;
		size_t tmp_len = 0;

		tmp_len = strlen(ctxt->fse->function.class) + strlen(ctxt->fse->function.function) + 3;
		tmp_name = xdmalloc(tmp_len);
		snprintf(tmp_name, tmp_len, "%s::%s", ctxt->fse->function.class, ctxt->fse->function.function);

		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "I: '%s::%s' is a normal method (%02x).", ctxt->fse->function.class, ctxt->fse->function.function, ctxt->fse->function.type);

		if (strcmp(tmp_name, brk_info->functionname) == 0) {
			xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "F: Breakpoint method (%s) matches current method (%s).", brk_info->functionname, tmp_name);
			brk_info->resolved = XDEBUG_BRK_RESOLVED;
			xdebug_dbgp_resolved_breakpoint_notification(ctxt->context, brk_info);
			xdfree(tmp_name);
			return;
		}

		xdfree(tmp_name);
	} else {
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "R: We don't handle this function type (%02x) yet.", ctxt->fse->function.type);
		return;
	}
}
*/

static void line_breakpoint_resolve_helper(xdebug_con *context, xdebug_lines_list *lines_list, xdebug_brk_info *brk_info)
{
//	xdebug_func        func;
	int                             i;
	xdebug_function_lines_map_item *found_item = NULL;
	int                             found_item_span = XDEBUG_RESOLVED_SPAN_MAX;

	/* Loop over all definitions in lines_list for file */
	for (i = 0; i < lines_list->count; i++) {
		xdebug_function_lines_map_item *item = lines_list->functions[i];

		/* Loop over all the file/line list entries to find the best fitting one for 'brk_info->original_lineno' */
		if (brk_info->original_lineno < item->line_start || brk_info->original_lineno > item->line_end) {
			xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "R: Line number (%d) out of range (%zd-%zd).", brk_info->original_lineno, item->line_start, item->line_end);
			continue;
		}

		if (item->line_span < found_item_span) {
			found_item = item;
			found_item_span = item->line_span;
		}
	}

	if (!found_item) {
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "R: Could not find any file/line entry in lines list.");
		return;
	}

	xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "R: Line number (%d) in smallest range of range (%zd-%zd).", brk_info->original_lineno, found_item->line_start, found_item->line_end);

	/* If the breakpoint's line number is in the set, mark as resolved */
	if (xdebug_set_in(found_item->lines_breakable, brk_info->original_lineno)) {
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "F: Breakpoint line (%d) found in set of executable lines.", brk_info->original_lineno);
		brk_info->resolved_lineno = brk_info->original_lineno;
		brk_info->resolved = XDEBUG_BRK_RESOLVED;
		xdebug_dbgp_resolved_breakpoint_notification(context, brk_info);
		return;
	} else {
		int tmp_lineno;

		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "I: Breakpoint line (%d) NOT found in set of executable lines.", brk_info->original_lineno);

		/* Check for a following line in the function */
		tmp_lineno = brk_info->original_lineno;
		do {
			tmp_lineno++;

			if (xdebug_set_in(found_item->lines_breakable, tmp_lineno)) {
				xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "  F: Line (%d) in set.", tmp_lineno);

				brk_info->resolved_lineno = tmp_lineno;
				brk_info->resolved = XDEBUG_BRK_RESOLVED;
				xdebug_dbgp_resolved_breakpoint_notification(context, brk_info);
				return;
			} else {
				xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "  I: Line (%d) not in set.", tmp_lineno);
			}
		} while (tmp_lineno < found_item->line_end && (tmp_lineno < brk_info->original_lineno + XDEBUG_DBGP_SCAN_RANGE));

		/* Check for a previous line in the function */
		tmp_lineno = brk_info->original_lineno;
		do {
			tmp_lineno--;

			if (xdebug_set_in(found_item->lines_breakable, tmp_lineno)) {
				xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "  F: Line (%d) in set.", tmp_lineno);

				brk_info->resolved_lineno = tmp_lineno;
				brk_info->resolved = XDEBUG_BRK_RESOLVED;
				xdebug_dbgp_resolved_breakpoint_notification(context, brk_info);
				return;
			} else {
				xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "  I: Line (%d) not in set.", tmp_lineno);
			}
		} while (tmp_lineno > found_item->line_start && (tmp_lineno > brk_info->original_lineno - XDEBUG_DBGP_SCAN_RANGE));
	}
}

static void breakpoint_resolve_helper(void *rctxt, xdebug_hash_element *he)
{
	xdebug_dbgp_resolve_context *ctxt = (xdebug_dbgp_resolve_context*) rctxt;
	xdebug_brk_admin            *admin = (xdebug_brk_admin*) he->ptr;
	xdebug_brk_info             *brk_info;

	brk_info = breakpoint_brk_info_fetch(admin->type, admin->key);

	xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "Breakpoint %d (type: %s).", admin->id, XDEBUG_BREAKPOINT_TYPE_NAME(brk_info->brk_type));

	/* Bail early if it's already resolved */
	if (brk_info->resolved == XDEBUG_BRK_RESOLVED) {
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "D: Breakpoint %d (type: %s) is already resolved.", admin->id, XDEBUG_BREAKPOINT_TYPE_NAME(brk_info->brk_type));
		return;
	}

	switch (brk_info->brk_type) {
		case XDEBUG_BREAKPOINT_TYPE_LINE:
		case XDEBUG_BREAKPOINT_TYPE_CONDITIONAL:
			if (!zend_string_equals(brk_info->filename, ctxt->filename)) {
				xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "R: File name (%s) does not match breakpoint to resolve (%s).", ZSTR_VAL(ctxt->filename), ZSTR_VAL(brk_info->filename));
				return;
			}

			line_breakpoint_resolve_helper(ctxt->context, ctxt->lines_list, brk_info);
			return;
/*
		case XDEBUG_BREAKPOINT_TYPE_CALL:
		case XDEBUG_BREAKPOINT_TYPE_RETURN:
			function_breakpoint_resolve_helper(rctxt, brk_info, he);
			return;
*/
		default:
			xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "R: The breakpoint type '%s' can not be resolved.", XDEBUG_BREAKPOINT_TYPE_NAME(brk_info->brk_type));
			return;
	}
}

/* Fetches the lines list for 'filename', and loops over all breakpoints to try
 * to resolve them at run-time */
int xdebug_dbgp_resolve_breakpoints(xdebug_con *context, zend_string *filename)
{
	xdebug_dbgp_resolve_context resolv_ctxt;
	xdebug_lines_list *lines_list;

	/* Get the lines list for the current file */
	if (!XG_DBG(breakable_lines_map) || !xdebug_hash_find(XG_DBG(breakable_lines_map), ZSTR_VAL(filename), ZSTR_LEN(filename), (void *) &lines_list)) {
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_DEBUG, "E: Lines list for '%s' does not exist.", ZSTR_VAL(filename));
		return 0;
	}

	resolv_ctxt.context = context;
	resolv_ctxt.filename = filename;
	resolv_ctxt.lines_list = lines_list;
	xdebug_hash_apply(context->breakpoint_list, (void *) &resolv_ctxt, breakpoint_resolve_helper);

	return 1;
}

int xdebug_dbgp_stream_output(const char *string, unsigned int length)
{
	if ((XG_DBG(stdout_mode) == 1 || XG_DBG(stdout_mode) == 2) && length) {
		xdebug_send_stream("stdout", string, length);
	}

	if (XG_DBG(stdout_mode) == 0 || XG_DBG(stdout_mode) == 1) {
		return 0;
	}
	return -1;
}

int xdebug_dbgp_notification(xdebug_con *context, zend_string *filename, long lineno, int type, char *type_string, char *message)
{
	xdebug_xml_node *response, *error_container;

	response = xdebug_xml_node_init("notify");
	xdebug_xml_add_attribute(response, "xmlns", "urn:debugger_protocol_v1");
	xdebug_xml_add_attribute(response, "xmlns:xdebug", "https://xdebug.org/dbgp/xdebug");
	xdebug_xml_add_attribute(response, "name", "error");

	error_container = xdebug_xml_node_init("xdebug:message");
	if (filename) {
		char *tmp_filename = NULL;

		if (check_evaled_code(filename, &tmp_filename)) {
			xdebug_xml_add_attribute_ex(error_container, "filename", tmp_filename, 0, 0);
		} else {
			xdebug_xml_add_attribute_ex(error_container, "filename", xdebug_path_to_url(filename), 0, 1);
		}
	}
	if (lineno) {
		xdebug_xml_add_attribute_ex(error_container, "lineno", xdebug_sprintf("%lu", lineno), 0, 1);
	}
	if (type_string) {
		xdebug_xml_add_attribute_ex(error_container, "type", xdstrdup(type_string), 0, 1);
	}
	if (message) {
		char *tmp_buf;

		if (type == E_ERROR && ((tmp_buf = xdebug_strip_php_stack_trace(message)) != NULL)) {
			xdebug_xml_add_text(error_container, tmp_buf);
		} else {
			xdebug_xml_add_text(error_container, xdstrdup(message));
		}
	}
	xdebug_xml_add_child(response, error_container);

	send_message(context, response);
	xdebug_xml_node_dtor(response);

	return 1;
}

static char *create_eval_key_file(zend_string *filename, int lineno)
{
	return xdebug_sprintf("%s(%d) : eval()'d code", ZSTR_VAL(filename), lineno);
}

static char *create_eval_key_id(int id)
{
	return xdebug_sprintf("%04x", id);
}

int xdebug_dbgp_register_eval_id(xdebug_con *context, function_stack_entry *fse)
{
	char             *key;
	xdebug_eval_info *ei;

	context->eval_id_sequence++;

	ei = xdcalloc(sizeof(xdebug_eval_info), 1);
	ei->id = context->eval_id_sequence;
	ei->contents = zend_string_copy(fse->include_filename);
	ei->refcount = 2;

	key = create_eval_key_file(fse->filename, fse->lineno);
	xdebug_hash_add(context->eval_id_lookup, key, strlen(key), (void*) ei);

	key = create_eval_key_id(ei->id);
	xdebug_hash_add(context->eval_id_lookup, key, strlen(key), (void*) ei);

	return ei->id;
}
