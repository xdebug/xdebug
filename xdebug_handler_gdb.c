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

#include "php.h"
#include "TSRM.h"
#include "php_globals.h"
#include "php_xdebug.h"
#include "xdebug_com.h"
#include "xdebug_llist.h"
#include "xdebug_handler_gdb.h"
#include "xdebug_var.h"

#ifdef PHP_WIN32
#include "win32/time.h"
#include <process.h>
#endif

ZEND_EXTERN_MODULE_GLOBALS(xdebug)
/*****************************************************************************
** Prototypes for debug command handlers
*/

char *xdebug_handle_backtrace(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_breakpoint(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_cont(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_option(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_quit(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_run(xdebug_con *context, xdebug_arg *args);

/*****************************************************************************
** Dispatcher tables for supported debug commands
*/

static xdebug_cmd commands_init[] = {
	{ "option", 2, "option [setting] [value]", xdebug_handle_option, 1, "Set a debug session option" },
	{ "run",    0, "run",                      xdebug_handle_run,    1, "Start the script" },
	{ NULL, 0, NULL, NULL, 0, NULL }
};

static xdebug_cmd commands_breakpoint[] = {
	{ "break",      1, "bre(ak) [functionname|filename:linenumber]", xdebug_handle_breakpoint, 1,
		"Set breakpoint at specified line or function.\n"
		"             Argument may be filename and linenumber or function name."
	},
	{ "bre",        1, "bre(ak) [functionname|filename:linenumber]", xdebug_handle_breakpoint, 0,
		"Set breakpoint at specified line or function.\n"
		"             Argument may be filename and linenumber or function name."
	},
	{ NULL, 0, NULL, NULL, 0, NULL }
};

static xdebug_cmd commands_run[] = {
	{ "quit",       0, "quit",                                       xdebug_handle_quit,       1, "Close the debug session" },
	{ NULL, 0, NULL, NULL, 0, NULL }
};

static xdebug_cmd commands_runtime[] = {
	{ "backtrace",  0, "backtrace [count]",                          xdebug_handle_backtrace,  0,
		"Print backtrace of all stack frames, or innermost COUNT frames.\n"
		"             Use of the 'full' qualifier also prints the values of the local\n"
		"             variables."
	},
	{ "bt",         0, "bt [count]",                                 xdebug_handle_backtrace,  1,
		"Print backtrace of all stack frames, or innermost COUNT frames.\n"
		"             Use of the 'full' qualifier also prints the values of the local\n"
		"             variables."
	},

	{ "cont",       0, "cont(inue)",                                 xdebug_handle_cont,       0,
		"Continue script being debugged, after error or breakpoint."
	},
	{ "continue",   0, "cont(inue)",                                 xdebug_handle_cont,       1,
		"Continue script being debugged, after error or breakpoint."
	},
};

/*****************************************************************************
** Utility functions
*/

static inline char* show_fname (struct function_stack_entry* entry TSRMLS_DC)
{
	char *tmp;
	xdebug_func f;

	f = entry->function;

	switch (f.type) {
		case XFUNC_NORMAL:
			return xdstrdup (f.function);
			break;

		case XFUNC_NEW:
			if (!f.class) {
				f.class = "?";
			}
			if (!f.function) {
				f.function = "?";
			}
			tmp = xdmalloc (strlen (f.class) + 4 + 1);
			sprintf (tmp, "new %s", f.class);
			return tmp;
			break;

		case XFUNC_STATIC_MEMBER:
			if (!f.class) {
				f.class = "?";
			}
			if (!f.function) {
				f.function = "?";
			}
			tmp = xdmalloc (strlen (f.function) + strlen (f.class) + 2 + 1);
			sprintf (tmp, "%s::%s", f.class, f.function);
			return tmp;
			break;

		case XFUNC_MEMBER:
			if (!f.class) {
				f.class = "?";
			}
			if (!f.function) {
				f.function = "?";
			}
			tmp = xdmalloc (strlen (f.function) + strlen (f.class) + 2 + 1);
			sprintf (tmp, "%s->%s", f.class, f.function);
			return tmp;
			break;

		case XFUNC_EVAL:
			return xdstrdup ("eval");
			break;

		case XFUNC_INCLUDE:
			return xdstrdup ("include");
			break;

		case XFUNC_INCLUDE_ONCE:
			return xdstrdup ("include_once");
			break;

		case XFUNC_REQUIRE:
			return xdstrdup ("require");
			break;

		case XFUNC_REQUIRE_ONCE:
			return xdstrdup ("require_once");
			break;

		default:
			return xdstrdup ("{unknown, please report}");
	}
}

static xdebug_cmd* scan_cmd(xdebug_cmd *ptr, char *line)
{
	while (ptr->name) {
		if (strcmp (ptr->name, line) == 0) {
			return ptr;
		}
		*ptr++;
	}
	return NULL;
}


static inline char* xdebug_memnstr(char *haystack, char *needle, int needle_len, char *end)
{
	char *p = haystack;
	char first = *needle;

	/* let end point to the last character where needle may start */
	end -= needle_len;
	
	while (p <= end) {
		while (*p != first)
			if (++p > end)
				return NULL;
		if (memcmp(p, needle, needle_len) == 0)
			return p;
		p++;
	}
	return NULL;
}

void xdebug_explode(char *delim, char *str, xdebug_arg *args, int limit) 
{
	char *p1, *p2, *endp;

	endp = str + strlen(str);

	p1 = str;
	p2 = xdebug_memnstr(str, delim, strlen(delim), endp);

	if (p2 == NULL) {
		args->c++;
		args->args = (char**) xdrealloc(args->args, sizeof(char*) * args->c);
		args->args[args->c - 1] = (char*) xdmalloc(strlen(str) + 1);
		memcpy(args->args[args->c - 1], p1, strlen(str));
		args->args[args->c - 1][strlen(str)] = '\0';
	} else {
		do {
			args->c++;
			args->args = (char**) xdrealloc(args->args, sizeof(char*) * args->c);
			args->args[args->c - 1] = (char*) xdmalloc(p2 - p1 + 1);
			memcpy(args->args[args->c - 1], p1, p2 - p1);
			args->args[args->c - 1][p2 - p1] = '\0';
			p1 = p2 + strlen(delim);
		} while ((p2 = xdebug_memnstr(p1, delim, strlen(delim), endp)) != NULL && (limit == -1 || --limit > 1));

		if (p1 <= endp) {
			args->c++;
			args->args = (char**) xdrealloc(args->args, sizeof(char*) * args->c);
			args->args[args->c - 1] = (char*) xdmalloc(endp - p1 + 1);
			memcpy(args->args[args->c - 1], p1, endp - p1);
			args->args[args->c - 1][endp - p1] = '\0';
		}
	}
}

/*****************************************************************************
** Helpers for looking up commands
*/

static inline xdebug_cmd* lookup_cmd_in_group(char *line, xdebug_cmd *group, int flag, int test_flag)
{
	xdebug_cmd *ptr;

	if (flag & test_flag) {
		ptr = scan_cmd(group, line);
		if (ptr) {
			return (ptr);
		}
	}
	return NULL;
}

static xdebug_cmd* lookup_cmd(char *line, int flag)
{
	xdebug_cmd *ptr;
	
	if ((ptr = lookup_cmd_in_group(line, commands_init,       flag, XDEBUG_INIT)) != NULL)       return ptr;
	if ((ptr = lookup_cmd_in_group(line, commands_breakpoint, flag, XDEBUG_BREAKPOINT)) != NULL) return ptr;
	if ((ptr = lookup_cmd_in_group(line, commands_run,        flag, XDEBUG_RUN)) != NULL)        return ptr;
	if ((ptr = lookup_cmd_in_group(line, commands_runtime,    flag, XDEBUG_RUNTIME)) != NULL)    return ptr;
#if 0
	if ((ptr = lookup_cmd_in_group(line, commands_data,       flag, XDEBUG_DATA)) != NULL)       return ptr;
	if ((ptr = lookup_cmd_in_group(line, commands_status,     flag, XDEBUG_STATUS)) != NULL)     return ptr;
#endif
	return NULL;
}

/*****************************************************************************
** Helpers for the "help" command
*/

static inline void show_available_commands_in_group(xdebug_con *h, int flag, int test_flag, xdebug_cmd *ptr)
{
	if (flag & test_flag ) {
    	while (ptr->name) {
			if (ptr->show && ptr->help) {
				SENDMSG(h->socket, xdebug_sprintf("%-12s %s\n", ptr->name, ptr->help));
			}
	        *ptr++;
	    }
	}
}

static void show_available_commands(xdebug_con *h, int flag)
{
	show_available_commands_in_group(h, flag, XDEBUG_INIT,       commands_init);
	show_available_commands_in_group(h, flag, XDEBUG_BREAKPOINT, commands_breakpoint);
	show_available_commands_in_group(h, flag, XDEBUG_RUN,        commands_run);
	show_available_commands_in_group(h, flag, XDEBUG_RUNTIME,    commands_runtime);
#if 0
	show_available_commands_in_group(h, flag, XDEBUG_DATA,       commands_data);
	show_available_commands_in_group(h, flag, XDEBUG_STATUS,     commands_status);
#endif
}

static void show_command_info(xdebug_con *h, xdebug_cmd* cmd)
{
	if (cmd) {
		SENDMSG(h->socket, xdebug_sprintf("Syntax: %s\n%12s %s\n", cmd->description, " ", cmd->help));
	} else {
		SSEND(h->socket, "No information available for this command.\n");
	}
}

/*****************************************************************************
** Data printing functions
*/

static print_breakpoint(xdebug_con *h, function_stack_entry *i)
{
	int c = 0; /* Comma flag */
	int j = 0; /* Counter */
/*
* Breakpoint 2, xdebug_execute (op_array=0x82caf50)
*     at /dat/dev/php/xdebug/xdebug.c:361
*/
	SENDMSG(h->socket, xdebug_sprintf("Breakpoint, %s (", i->function.function));

	/* Printing vars */
	for (j = 0; j < i->varc; j++) {
		if (c) {
			SSEND(h->socket, ", ");
		} else {
			c = 1;
		}

		if (i->vars[j].name) {
		   SENDMSG(h->socket, xdebug_sprintf ("$%s = ", i->vars[j].name));
		}
		SSEND(h->socket, i->vars[j].value);
	}

	SENDMSG(h->socket, xdebug_sprintf(")\n\tat %s:%d\n", i->filename, i->lineno));
}

static print_stackframe(xdebug_con *h, int nr, function_stack_entry *i)
{
	int c = 0; /* Comma flag */
	int j = 0; /* Counter */
/*
* 0x4001af2e in xdebug_compile_file (file_handle=0xbffff960, type=2)
*     at /dat/dev/php/xdebug/xdebug.c:901
*         
*/
	if (nr) {
		SENDMSG(h->socket, xdebug_sprintf("#%-2d %s (", nr, i->function.function));
	} else {
		SENDMSG(h->socket, xdebug_sprintf("%s (", i->function.function));
	}

	/* Printing vars */
	for (j = 0; j < i->varc; j++) {
		if (c) {
			SSEND(h->socket, ", ");
		} else {
			c = 1;
		}

		if (i->vars[j].name) {
		   SENDMSG(h->socket, xdebug_sprintf ("$%s = ", i->vars[j].name));
		}
		SSEND(h->socket, i->vars[j].value);
	}

	SENDMSG(h->socket, xdebug_sprintf(")\n    at %s:%d\n", i->filename, i->lineno));
}

/*****************************************************************************
** Client command handlers
*/

char *xdebug_handle_backtrace(xdebug_con *context, xdebug_arg *args)
{
	xdebug_llist_element *le;
	int                   counter = 1;
	TSRMLS_FETCH();

	for (le = XDEBUG_LLIST_TAIL(XG(stack)); le != NULL; le = XDEBUG_LLIST_PREV(le)) {
		print_stackframe(context, counter++, XDEBUG_LLIST_VALP(le));
	}

	return NULL;
}

char *xdebug_handle_breakpoint(xdebug_con *context, xdebug_arg *args)
{
	if (strstr(args->args[0], "::")) { /* class::method */
		return xdstrdup("Class::method breakpoints are not yet supported.");
	} else if (strstr(args->args[0], ":")) { /* file:line */
		return xdstrdup("File:line breakpoints are not yet supported.");
	} else { /* function */
		if (!xdebug_hash_add(context->function_breakpoints, args->args[0], strlen(args->args[0]), (void*) args->args[0])) {
			return xdstrdup("Breakpoint could not be set.");
		} else {
			SENDMSG(context->socket, xdebug_sprintf("Breakpoint on %s.\n", args->args[0]));
		}
	}
	return NULL;
}

char *xdebug_handle_cont(xdebug_con *context, xdebug_arg *args)
{
	SSEND(context->socket, "Continuing.\n");
	return NULL;
}

char *xdebug_handle_option(xdebug_con *context, xdebug_arg *args)
{
	return NULL;
}

char *xdebug_handle_quit(xdebug_con *context, xdebug_arg *args)
{
	return NULL;
}

char *xdebug_handle_run(xdebug_con *context, xdebug_arg *args)
{
	SENDMSG(context->socket, xdebug_sprintf("Starting program: %s\n", context->program_name));
	return NULL;
}


/*****************************************************************************
** Parsing functions
*/

int xdebug_gdb_parse_option(xdebug_con *context, char* line, int flags, char *end_cmd, char **error)
{
	char *ptr;
	xdebug_cmd *cmd;
	int i;
	int retval;
	char *ret_err = NULL;
	
	xdebug_arg *args = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));
	args->c = 0;
	args->args = NULL;

	*error = NULL;

	/* Try to find command */
	ptr = strchr(line, ' ');
	if (!ptr) { /* No separator found */
		/* Check for the special case "help" */
		if (strcmp(line, "help") == 0) {
			show_available_commands(context, flags);
			retval = 0;
			goto cleanup;
		}
		if (!(cmd = lookup_cmd(line, flags))) {
			*error = xdebug_sprintf("Undefined command: \"%s\".  Try \"help\".", line);
			retval = -1;
			goto cleanup;
		}
	} else {
		char *tmp = (char*) xdmalloc(ptr - line + 1);
		memcpy(tmp, line, ptr - line);
		tmp[ptr - line] = '\0';

		/* Check for the special case "help [command]" */
		if (strcmp(tmp, "help") == 0) {
			xdebug_explode(" ", ptr + 1, args, -1); 
			if (args->c > 0) {
				show_command_info(context, lookup_cmd(args->args[0], XDEBUG_ALL));
				retval = 0;
			} else {
				*error = xdebug_sprintf("Undefined command: \"%s\".  Try \"help\".", tmp);
				retval = -1;
			}
			xdfree(tmp);
			goto cleanup;
		}

		/* Scan for valid commands */
		if (cmd = lookup_cmd(tmp, flags)) {
			xdfree(tmp);
			xdebug_explode(" ", ptr + 1, args, -1); 
		} else {
			*error = xdebug_sprintf("Undefined command: \"%s\".  Try \"help\".", tmp);
			xdfree(tmp);
			retval = -1;
			goto cleanup;
		}
	}

	retval = 0;

	/* Default in continue mode */
	if (args->c >= cmd->args) {
		ret_err = cmd->handler(context, args);
		if (ret_err) {
			*error = xdstrdup(ret_err);
			xdfree(ret_err);
			retval = -1;
			goto cleanup;
		}
	} else {
		*error = xdstrdup(cmd->description);
		/* Oopsie, error */
		retval = -1;
		goto cleanup;
	}
	/* If the end command is reached, or the command is quit, set the return
	 * value to 1 (continue) */
	if (strcmp(cmd->name, end_cmd) == 0) {
		retval = 1;
	}
cleanup:
	for (i = 0; i < args->c; i++) {
		xdfree(args->args[i]);
	}
	xdfree(args->args);
	xdfree(args);
	return retval;
}

static void xdebug_gdb_option_result(xdebug_con *context, int ret, char *error)
{
	if (error || ret == -1) {
		SSEND(context->socket, "-ERROR");
		if (error) {
			SSEND(context->socket, ": ");
			SSEND(context->socket, error);
		}
		SSEND(context->socket, "\n");
	} else {
		SSEND(context->socket, "+OK\n");
	}
}

/*****************************************************************************
** Handlers for debug functions
*/

int xdebug_gdb_init(xdebug_con *context, int mode)
{
	char *option;
	int   ret;
	char *error = NULL;

	SENDMSG(context->socket, xdebug_sprintf("This is Xdebug version %s.\n", XDEBUG_VERSION));
	SSEND(context->socket, "Copyright 2002 by Derick Rethans, JDI Media Solutions.\n");
	context->buffer = xdmalloc(sizeof(xdebug_socket_buf));
	context->buffer->buffer = NULL;
	context->buffer->buffer_size = 0;
#warning Add dtor!
	context->function_breakpoints = xdebug_hash_alloc(64, NULL);
	do {
		SSEND(context->socket, "?init\n");
		option = xdebug_socket_read_line(context->socket, context->buffer);
		if (!option) {
			return 0;
		}
		ret = xdebug_gdb_parse_option(context, option, XDEBUG_INIT | XDEBUG_BREAKPOINT | XDEBUG_RUN | XDEBUG_STATUS, "run", (char**) &error);
		xdebug_gdb_option_result(context, ret, error);
	} while (1 != ret);

	return 1;
}

int xdebug_gdb_deinit(xdebug_con *context)
{
	xdebug_hash_destroy(context->function_breakpoints);
}

int xdebug_gdb_error(xdebug_con *context, int type, char *message, const char *location, const uint line, xdebug_llist *stack)
{
	char *errortype;
	int   ret;
	char *option;
	char *error = NULL;
	xdebug_llist_element *le;

	errortype = error_type(type);

	SENDMSG(context->socket, xdebug_sprintf("\nProgram received signal %s: %s.\n", errortype, message));

	print_stackframe(context, 0, XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(stack)));

	xdfree(errortype);
	do {
		SSEND(context->socket, "?cmd\n");
		option = xdebug_socket_read_line(context->socket, context->buffer);
		if (!option) {
			return 0;
		}
		ret = xdebug_gdb_parse_option(context, option, XDEBUG_BREAKPOINT | XDEBUG_RUN | XDEBUG_RUNTIME | XDEBUG_STATUS, "cont", (char**) &error);
		xdebug_gdb_option_result(context, ret, error);
	} while (1 != ret);

	return 1;
}

int xdebug_gdb_breakpoint(xdebug_con *context, xdebug_llist *stack)
{
	struct function_stack_entry *i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(stack));
	int    ret;
	char  *option;
	char  *error = NULL;

	print_breakpoint(context, i);

	do {
		SSEND(context->socket, "?cmd\n");
		option = xdebug_socket_read_line(context->socket, context->buffer);
		if (!option) {
			return 0;
		}
		ret = xdebug_gdb_parse_option(context, option, XDEBUG_BREAKPOINT | XDEBUG_RUN| XDEBUG_RUNTIME | XDEBUG_STATUS, "cont", (char**) &error);
		xdebug_gdb_option_result(context, ret, error);
	} while (1 != ret);

	return 1;
}
