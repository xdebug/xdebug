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
#include <fcntl.h>

ZEND_EXTERN_MODULE_GLOBALS(xdebug)
/*****************************************************************************
** Prototypes for debug command handlers
*/

char *xdebug_handle_backtrace(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_breakpoint(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_cont(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_eval(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_delete(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_finish(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_kill(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_list(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_next(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_option(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_print(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_pwd(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_quit(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_run(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_show(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_step(xdebug_con *context, xdebug_arg *args);

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
		"             Argument may be filename and linenumber, function name or '{main}'\n"
		"             for the first PHP line."
	},
	{ "bre",        1, "bre(ak) [functionname|filename:linenumber]", xdebug_handle_breakpoint, 0,
		"Set breakpoint at specified line or function.\n"
		"             Argument may be filename and linenumber, function name or '{main}'\n"
		"             for the first PHP line."
	},
	{ "delete",     1, "del(ete) [functionname|filename:linenumber]", xdebug_handle_delete, 1,
		"Removed breakpoint at specified line or function.\n"
		"             Argument may be filename and linenumber, function name or '{main}'\n"
		"             for the first PHP line."
	},
	{ "del",        1, "del(ete) [functionname|filename:linenumber]", xdebug_handle_delete, 0,
		"Removed breakpoint at specified line or function.\n"
		"             Argument may be filename and linenumber, function name or '{main}'\n"
		"             for the first PHP line."
	},
	{ NULL, 0, NULL, NULL, 0, NULL }
};

static xdebug_cmd commands_data[] = {
	{ "eval",       0, "eval",                                       xdebug_handle_eval,       1,
		"Evaluation PHP code"
	},
	{ "show",       0, "show",                                       xdebug_handle_show,       1,
		"Show a list of all variables"
	},
	{ "print",      1, "print",                                      xdebug_handle_print,      1,
		"Prints the contents of the variable"
	},
	{ NULL, 0, NULL, NULL, 0, NULL }
};

static xdebug_cmd commands_run[] = {
	{ "kill",       0, "kill",                                       xdebug_handle_kill,       1, "Kill the script" },
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

	{ "finish",     0, "finish",                                     xdebug_handle_finish,     1,
		"Continues executing until the current function returned to the\n"
		"             calling function."
	},

	{ "list",       0, "list [[file:]beginline] [endline]",          xdebug_handle_list,       1,
		"Lists specified line. With no arguments, lists ten more lines\n"
		"             after or before the previous listing. One argument specifies the\n"
		"             the line in a file to start, and then lines are listed around that\n"
		"             line. Two arguments specify starting and ending lines to list."
	},

	{ "next",       0, "next",                                       xdebug_handle_next,       1,
		"Continues executing until the next statement in the same stack\n"
		"             frame. Is basically the same as 'step' but does not go into\n"
		"             function calls if they occur."
	},

	{ "pwd",        0, "pwd",                                        xdebug_handle_pwd,        1,
		"Prints the current working directory."
	},

	{ "step",       0, "step",                                       xdebug_handle_step,       1,
		"Continues executing until the next statement."
	},

	{ NULL, 0, NULL, NULL, 0, NULL }
};

/*****************************************************************************
** Utility functions
*/

static xdebug_cmd* scan_cmd(xdebug_cmd *ptr, char *line)
{
	while (ptr->name) {
		if (strcmp (ptr->name, line) == 0) {
			return ptr;
		}
		ptr++;
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
	if ((ptr = lookup_cmd_in_group(line, commands_data,       flag, XDEBUG_DATA)) != NULL)       return ptr;
#if 0
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
	        ptr++;
	    }
	}
}

static void show_available_commands(xdebug_con *h, int flag)
{
	show_available_commands_in_group(h, flag, XDEBUG_INIT,       commands_init);
	show_available_commands_in_group(h, flag, XDEBUG_BREAKPOINT, commands_breakpoint);
	show_available_commands_in_group(h, flag, XDEBUG_RUN,        commands_run);
	show_available_commands_in_group(h, flag, XDEBUG_RUNTIME,    commands_runtime);
	show_available_commands_in_group(h, flag, XDEBUG_DATA,       commands_data);
#if 0
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
static void print_sourceline(xdebug_con *h, char *file, int begin, int end, int offset TSRMLS_DC)
{
	int    fd;
	fd_buf fd_buffer = { NULL, 0 };
	int    i = begin;
	char  *line = NULL;
	int    update = 0;

	if (i < 0) {
		begin = 0;
		i = 0;
	}

	/* Read until the "begin" line has been read */
	if ((fd = open(file, 0)) == -1) {
		SENDMSG(h->socket, xdebug_sprintf("The file '%s' could not be opened.\n", file));
	}
	
	while (i > 0) {
		if (line) {
			free(line);
			line = NULL;
		}
		line = fd_read_line(fd, &fd_buffer, FD_RL_FILE);
		i--;
	}
	/* Read until the "end" line has been read */
	do {
		if (line) {
			update = 1;
			SENDMSG(h->socket, xdebug_sprintf("%d\t%s\n", begin + i, line));
			free(line);
			line = NULL;
		}
		line = fd_read_line(fd, &fd_buffer, FD_RL_FILE);
		i++;
	} while (i < end + 1 - begin);

	/* Print last line */
	if (line) {
		update = 1;
		free(line);
		line = NULL;
	}

	/* Update "last" info */
	if (update) {
		if (XG(context).list.last_file && XG(context).list.last_file != file) {
			xdfree(XG(context).list.last_file);
		}
		if (XG(context).list.last_file != file) {
			XG(context).list.last_file = xdstrdup(file);
		}
		XG(context).list.last_line = end + 1 + offset;
	}
}

static void print_breakpoint(xdebug_con *h, function_stack_entry *i)
{
	int c = 0; /* Comma flag */
	int j = 0; /* Counter */
	char *tmp_fname;
	TSRMLS_FETCH();
/*
* Breakpoint 2, xdebug_execute (op_array=0x82caf50)
*     at /dat/dev/php/xdebug/xdebug.c:361
*/
	tmp_fname = show_fname(i TSRMLS_CC);
	SENDMSG(h->socket, xdebug_sprintf("Breakpoint, %s(", tmp_fname));
	xdfree(tmp_fname);

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

static void print_stackframe(xdebug_con *h, int nr, function_stack_entry *i)
{
	int c = 0; /* Comma flag */
	int j = 0; /* Counter */
	char *tmp_fname;
	TSRMLS_FETCH();
	
/*
* 0x4001af2e in xdebug_compile_file (file_handle=0xbffff960, type=2)
*     at /dat/dev/php/xdebug/xdebug.c:901
*         
*/
   	tmp_fname = show_fname(i TSRMLS_CC);
	if (nr) {
		SENDMSG(h->socket, xdebug_sprintf("#%-2d %s (", nr, tmp_fname));
	} else {
		SENDMSG(h->socket, xdebug_sprintf("%s (", tmp_fname));
	}
	xdfree(tmp_fname);

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
	xdebug_arg      *method = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));
	char            *tmp_name;
	xdebug_brk_info *extra_brk_info;

	xdebug_arg_init(method);

	if (strstr(args->args[0], "::")) { /* class::method */
		xdebug_explode("::", args->args[0], method, -1);
		if (method->c != 2) {
			xdebug_arg_dtor(method);
			return xdstrdup("Invalid format for class/method combination.");
		} else {
			if (!xdebug_hash_add(context->class_breakpoints, args->args[0], strlen(args->args[0]), (void*) 0)) {
				xdebug_arg_dtor(method);
				return xdstrdup("Breakpoint could not be set.");
			} else {
				SENDMSG(context->socket, xdebug_sprintf("Breakpoint on %s.\n", args->args[0]));
				xdebug_arg_dtor(method);
			}
		}
	} else if (strstr(args->args[0], "->")) { /* class->method */
		xdebug_explode("->", args->args[0], method, -1);
		if (method->c != 2) {
			xdebug_arg_dtor(method);
			return xdstrdup("Invalid format for class/method combination.");
		} else {
			if (!xdebug_hash_add(context->class_breakpoints, args->args[0], strlen(args->args[0]), (void*) 0)) {
				xdebug_arg_dtor(method);
				return xdstrdup("Breakpoint could not be set.");
			} else {
				SENDMSG(context->socket, xdebug_sprintf("Breakpoint on %s.\n", args->args[0]));
				xdebug_arg_dtor(method);
			}
		}
	} else if (strstr(args->args[0], ":")) { /* file:line */
		xdebug_explode(":", args->args[0], method, -1); /* 0 = filename, 1 = linenumer */
		if (method->c != 2) {
			xdebug_arg_dtor(method);
			return xdstrdup("Invalid format for file:line combination.");
		} else {
			/* Make search key */
			if (method->args[0][0] != '/') {
				tmp_name = xdebug_sprintf("/%s", method->args[0]);
			} else {
				tmp_name = xdebug_sprintf("%s", method->args[0]);
			}

			/* Set line number in extra structure */
			extra_brk_info = xdmalloc(sizeof(xdebug_brk_info));
			extra_brk_info->lineno = atoi(method->args[1]);
			extra_brk_info->file = tmp_name;
			extra_brk_info->file_len = strlen(tmp_name);

			/* Add breakpoint to the list */
			xdebug_llist_insert_next(context->line_breakpoints, XDEBUG_LLIST_TAIL(context->line_breakpoints), (void*) extra_brk_info);
			SENDMSG(context->socket, xdebug_sprintf("Breakpoint on %s.\n", method->args[0]));
			xdebug_arg_dtor(method);
		}
	} else { /* function */
		if (!xdebug_hash_add(context->function_breakpoints, args->args[0], strlen(args->args[0]), (void*) 0)) {
			xdebug_arg_dtor(method);
			return xdstrdup("Breakpoint could not be set.");
		} else {
			SENDMSG(context->socket, xdebug_sprintf("Breakpoint on %s.\n", args->args[0]));
			xdebug_arg_dtor(method);
		}
	}
	return NULL;
}

char *xdebug_handle_cont(xdebug_con *context, xdebug_arg *args)
{
	SSEND(context->socket, "Continuing.\n");
	return NULL;
}

char *xdebug_handle_delete(xdebug_con *context, xdebug_arg *args)
{
	xdebug_arg      *method = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));
	char            *tmp_name;
	xdebug_brk_info *extra_brk_info;
	xdebug_llist_element *le;
	xdebug_brk_info      *brk;
	TSRMLS_FETCH();

	xdebug_arg_init(method);

	if (strstr(args->args[0], "::")) { /* class::method */
		xdebug_explode("::", args->args[0], method, -1);
		if (method->c != 2) {
			xdebug_arg_dtor(method);
			return xdstrdup("Invalid format for class/method combination.");
		} else {
			if (!xdebug_hash_delete(context->class_breakpoints, args->args[0], strlen(args->args[0]))) {
				xdebug_arg_dtor(method);
				return xdstrdup("Breakpoint could not be removed.");
			} else {
				SENDMSG(context->socket, xdebug_sprintf("Breakpoint removed from %s.\n", args->args[0]));
				xdebug_arg_dtor(method);
			}
		}
	} else if (strstr(args->args[0], "->")) { /* class->method */
		xdebug_explode("->", args->args[0], method, -1);
		if (method->c != 2) {
			xdebug_arg_dtor(method);
			return xdstrdup("Invalid format for class/method combination.");
		} else {
			if (!xdebug_hash_delete(context->class_breakpoints, args->args[0], strlen(args->args[0]))) {
				xdebug_arg_dtor(method);
				return xdstrdup("Breakpoint could not be removed.");
			} else {
				SENDMSG(context->socket, xdebug_sprintf("Breakpoint removed from %s.\n", args->args[0]));
				xdebug_arg_dtor(method);
			}
		}
	} else if (strstr(args->args[0], ":")) { /* file:line */
		xdebug_explode(":", args->args[0], method, -1); /* 0 = filename, 1 = linenumer */
		if (method->c != 2) {
			xdebug_arg_dtor(method);
			return xdstrdup("Invalid format for file:line combination.");
		} else {
			/* Make search key */
			if (method->args[0][0] != '/') {
				tmp_name = xdebug_sprintf("/%s", method->args[0]);
			} else {
				tmp_name = xdebug_sprintf("%s", method->args[0]);
			}

			/* Set line number in extra structure */
			extra_brk_info = xdmalloc(sizeof(xdebug_brk_info));
			extra_brk_info->lineno = atoi(method->args[1]);
			extra_brk_info->file = tmp_name;
			extra_brk_info->file_len = strlen(tmp_name);

			/* Add breakpoint to the list */
			for (le = XDEBUG_LLIST_HEAD(XG(context).line_breakpoints); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
				brk = XDEBUG_LLIST_VALP(le);

				if (atoi(method->args[1]) == brk->lineno && strcmp(tmp_name, brk->file) == 0) {
					xdebug_llist_remove(context->line_breakpoints, le, NULL);
					SENDMSG(context->socket, xdebug_sprintf("Breakpoint removed from %s.\n", method->args[0]));
					xdebug_arg_dtor(method);
					return NULL;
				}
			}
			xdebug_arg_dtor(method);
		}
	} else { /* function */
		if (!xdebug_hash_delete(context->function_breakpoints, args->args[0], strlen(args->args[0]))) {
			xdebug_arg_dtor(method);
			return xdstrdup("Breakpoint could not be removed.");
		} else {
			SENDMSG(context->socket, xdebug_sprintf("Breakpoint removed from %s.\n", args->args[0]));
			xdebug_arg_dtor(method);
		}
	}
	return NULL;
}

char *xdebug_handle_eval(xdebug_con *context, xdebug_arg *args)
{
	int        i;
	xdebug_str buffer = {0, 0, NULL};
	zval       retval;
	char      *ret_value;
	int        old_error_reporting;
	TSRMLS_FETCH();

	/* Remember error reporting level */
	old_error_reporting = EG(error_reporting);
	EG(error_reporting) = 0;
	
	/* Concat all arguments back together */
	XDEBUG_STR_ADD(&buffer, args->args[0], 0);
	
	for (i = 1; i < args->c; i++) {
		XDEBUG_STR_ADD(&buffer, " ", 0);
		XDEBUG_STR_ADD(&buffer, args->args[i], 0);
	}
	
	SENDMSG(context->socket, xdebug_sprintf("Evaluating '%s'\n", buffer.d));
	if (zend_eval_string(buffer.d, &retval, "xdebug eval" TSRMLS_CC) == FAILURE) {
		XDEBUG_STR_FREE(&buffer);
		EG(error_reporting) = old_error_reporting;
		return xdstrdup("eval");
	} else {
		XDEBUG_STR_FREE(&buffer);
		EG(error_reporting) = old_error_reporting;
		ret_value = get_zval_value(&retval);
		SENDMSG(context->socket, xdebug_sprintf("returned: %s\n", ret_value));
		xdfree(ret_value);
		return NULL;
	}
}

char *xdebug_handle_finish(xdebug_con *context, xdebug_arg *args)
{
	xdebug_llist_element *le;
	function_stack_entry *fse;
	TSRMLS_FETCH();

	XG(context).do_next   = 0;
	XG(context).do_step   = 0;
	XG(context).do_finish = 1;

	if (XG(stack)) {
		le = XDEBUG_LLIST_TAIL(XG(stack));
		fse = XDEBUG_LLIST_VALP(le);
		XG(context).next_level = fse->level - 1;
	} else {
		XG(context).next_level = -1;
	}

	return NULL;
}

char *xdebug_handle_kill(xdebug_con *context, xdebug_arg *args)
{
	zend_bailout();
	return NULL;
}

char *xdebug_handle_list(xdebug_con *context, xdebug_arg *args)
{
	char *tmp_file  = NULL;
	int   tmp_begin = 0;
	int   tmp_end   = 0;
	xdebug_arg *parts = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));
	TSRMLS_FETCH();

	xdebug_arg_init(parts);

	switch (args->c) {
		case 0: 
			if (XG(context).list.last_file) {
				tmp_file  = XG(context).list.last_file;
				tmp_begin = XG(context).list.last_line;
			} else {
				tmp_file  = XG(context).program_name;
				tmp_begin = 1;
			}
			tmp_end = tmp_begin + 9;
			break;
		case 1: 
		case 2: 
			xdebug_explode(":", args->args[0], parts, -1);
			tmp_begin = parts->c == 1 ? atoi(parts->args[0]) : atoi(parts->args[1]);
			if (tmp_begin <= 0) {
				tmp_begin = 1;
			}

			if (parts->c == 1) { /* Only line is given */
				if (XG(context).list.last_file) {
					tmp_file = XG(context).list.last_file;
				} else {
					tmp_file = XG(context).program_name;
				}
			} else if (parts->c == 2) { /* File:line is given */
				tmp_file = parts->args[0];
			}

			if (args->c == 1) {
				tmp_end = tmp_begin + 9;
			} else {
				tmp_end = atoi(args->args[1]);
			}
			break;
		default:
			return xdstrdup("Too many arguments."); 
	}
	print_sourceline(context, tmp_file, tmp_begin, tmp_end, 0 TSRMLS_CC);

	xdebug_arg_dtor(parts);	
	return NULL;
}

char *xdebug_handle_next(xdebug_con *context, xdebug_arg *args)
{
	xdebug_llist_element *le;
	function_stack_entry *fse;
	TSRMLS_FETCH();

	XG(context).do_next   = 1;
	XG(context).do_step   = 0;
	XG(context).do_finish = 0;

	if (XG(stack)) {
		le = XDEBUG_LLIST_TAIL(XG(stack));
		fse = XDEBUG_LLIST_VALP(le);
		XG(context).next_level = fse->level;
	} else {
		XG(context).next_level = 0;
	}

	return NULL;
}

char *xdebug_handle_option(xdebug_con *context, xdebug_arg *args)
{
	return NULL;
}

char *xdebug_handle_print(xdebug_con *context, xdebug_arg *args)
{
	HashTable                   *st = NULL;
	zval                       **retval;
	TSRMLS_FETCH();

	st = EG(active_symbol_table);
	if (zend_hash_find(st, args->args[0], strlen(args->args[0]) + 1, (void **) &retval) == SUCCESS) {
		SENDMSG(context->socket, xdebug_sprintf ("$%s = %s\n", args->args[0], get_zval_value(*retval)));
		return NULL;
	}

	st = EG(active_op_array)->static_variables;
	if (st) {
		if (zend_hash_find(st, args->args[0], strlen(args->args[0]) + 1, (void **) &retval) == SUCCESS) {
			SENDMSG(context->socket, xdebug_sprintf ("$%s = %s\n", args->args[0], get_zval_value(*retval)));
			return NULL;
		}
	}
	
	st = &EG(symbol_table);
	if (zend_hash_find(st, args->args[0], strlen(args->args[0]) + 1, (void **) &retval) == SUCCESS) {
		SENDMSG(context->socket, xdebug_sprintf ("$%s = %s\n", args->args[0], get_zval_value(*retval)));
		return NULL;
	}

	return xdebug_sprintf("The symbol '%s' does not exist or is not yet initialized.", args->args[0]);
}

char *xdebug_handle_pwd(xdebug_con *context, xdebug_arg *args)
{
	char buffer[256 + 1];

	if (getcwd(buffer, 256)) {
		SENDMSG(context->socket, xdebug_sprintf("Working directory %s.\n", buffer));
	}
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


static void dump_used_var (void *context, xdebug_hash_element* he)
{
	char *name = (char*) he->ptr;
	xdebug_con *h = (xdebug_con*) context;

	SENDMSG(h->socket, xdebug_sprintf ("$%s\n", name));
}

char *xdebug_handle_show(xdebug_con *context, xdebug_arg *args)
{
	struct function_stack_entry *i;
	xdebug_hash                 *ht;
	TSRMLS_FETCH();

	if (XDEBUG_LLIST_TAIL(XG(stack))) {
		i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack)));
		ht = i->used_vars;

		xdebug_hash_apply(ht, (void *) context, dump_used_var);
	}
	
	return NULL;
}


char *xdebug_handle_step(xdebug_con *context, xdebug_arg *args)
{
	TSRMLS_FETCH();
	XG(context).do_step   = 1;
	XG(context).do_next   = 0;
	XG(context).do_finish = 0;
	
	return NULL;
}


/*****************************************************************************
** Parsing functions
*/

int xdebug_gdb_parse_option(xdebug_con *context, char* line, int flags, char *end_cmd, char **error)
{
	char *ptr;
	xdebug_cmd *cmd;
	int retval;
	char *ret_err = NULL;
	int i;
	
	xdebug_arg *args = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));
	xdebug_arg *endcmds = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));
	xdebug_arg_init(args);
	xdebug_arg_init(endcmds);

	xdebug_explode(",", end_cmd, endcmds, -1); 

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
		if ((cmd = lookup_cmd(tmp, flags))) {
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
	for (i = 0; i < endcmds->c; i++) {
		if (strcmp(cmd->name, endcmds->args[i]) == 0) {
			retval = 1;
			goto cleanup;
		}
	}
cleanup:
	xdebug_arg_dtor(args);
	xdebug_arg_dtor(endcmds);
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
	context->buffer = xdmalloc(sizeof(fd_buf));
	context->buffer->buffer = NULL;
	context->buffer->buffer_size = 0;

	context->function_breakpoints = xdebug_hash_alloc(64, NULL);
	context->class_breakpoints = xdebug_hash_alloc(64, NULL);
	context->line_breakpoints = xdebug_llist_alloc((xdebug_llist_dtor) xdebug_brk_dtor);
	do {
		SSEND(context->socket, "?init\n");
		option = fd_read_line(context->socket, context->buffer, FD_RL_SOCKET);
		if (!option) {
			return 0;
		}
		ret = xdebug_gdb_parse_option(context, option, XDEBUG_INIT | XDEBUG_DATA | XDEBUG_BREAKPOINT | XDEBUG_RUN | XDEBUG_STATUS, "run", (char**) &error);
		xdebug_gdb_option_result(context, ret, error);
		free(option);
	} while (1 != ret);

	return 1;
}

int xdebug_gdb_deinit(xdebug_con *context)
{
	xdebug_hash_destroy(context->function_breakpoints);
	xdebug_hash_destroy(context->class_breakpoints);
	xdebug_llist_destroy(context->line_breakpoints, NULL);
	xdfree(context->buffer);

	return 1;
}

int xdebug_gdb_error(xdebug_con *context, int type, char *message, const char *location, const uint line, xdebug_llist *stack)
{
	char *errortype;
	int   ret;
	char *option;
	char *error = NULL;
	int   runtime_allowed;

	errortype = error_type(type);

	SENDMSG(context->socket, xdebug_sprintf("\nProgram received signal %s: %s.\n", errortype, message));

	print_stackframe(context, 0, XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(stack)));

	xdfree(errortype);
	do {
		SSEND(context->socket, "?cmd\n");
		option = fd_read_line(context->socket, context->buffer, FD_RL_SOCKET);
		if (!option) {
			return 0;
		}
		runtime_allowed = (
			(type != E_ERROR) && 
			(type != E_CORE_ERROR) &&
			(type != E_COMPILE_ERROR) &&
			(type != E_USER_ERROR)
		) ? XDEBUG_BREAKPOINT | XDEBUG_RUNTIME : 0;
		ret = xdebug_gdb_parse_option(context, option, XDEBUG_DATA | XDEBUG_RUN | runtime_allowed | XDEBUG_STATUS, "cont", (char**) &error);
		xdebug_gdb_option_result(context, ret, error);
		free(option);
	} while (1 != ret);

	return 1;
}

int xdebug_gdb_breakpoint(xdebug_con *context, xdebug_llist *stack, char *file, int lineno, int type)
{
	struct function_stack_entry *i;
	int    ret;
	char  *option;
	char  *error = NULL;
	TSRMLS_FETCH();

	i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(stack));

	if (type == XDEBUG_BREAK) {
		print_breakpoint(context, i);
	}
	print_sourceline(context, file, lineno, lineno, -1 TSRMLS_CC);

	do {
		SSEND(context->socket, "?cmd\n");
		option = fd_read_line(context->socket, context->buffer, FD_RL_SOCKET);
		if (!option) {
			return 0;
		}
		ret = xdebug_gdb_parse_option(context, option, XDEBUG_BREAKPOINT | XDEBUG_DATA | XDEBUG_RUN | XDEBUG_RUNTIME | XDEBUG_STATUS, "cont,step,next,finish", (char**) &error);
		xdebug_gdb_option_result(context, ret, error);
		free(option);
	} while (1 != ret);

	return 1;
}
