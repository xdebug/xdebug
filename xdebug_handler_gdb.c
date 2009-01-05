/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007 Derick Rethans      |
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

#include "php.h"
#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "TSRM.h"
#include "php_globals.h"
#include "php_xdebug.h"
#include "xdebug_private.h"
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

static char *xdebug_handle_backtrace(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_breakpoint(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_cont(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_eval(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_delete(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_finish(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_kill(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_list(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_next(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_option(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_print(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_pwd(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_quit(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_run(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_show(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_show_breakpoints(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_show_local(xdebug_con *context, xdebug_arg *args);
static char *xdebug_handle_step(xdebug_con *context, xdebug_arg *args);

/*****************************************************************************
** Dispatcher tables for supported debug commands
*/

static xdebug_gdb_cmd commands_init[] = {
	{ "option", 2, "option [setting] [value]", xdebug_handle_option, 1, "Set a debug session option" },
	{ "run",    0, "run",                      xdebug_handle_run,    1, "Start the script" },
	{ NULL, 0, NULL, NULL, 0, NULL }
};

static xdebug_gdb_cmd commands_breakpoint[] = {
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
	{ "show-breakpoints", 0, "show-breakpoints",                      xdebug_handle_show_breakpoints, 1,
		"Shows a list of all breakpoints and optional conditions."
	},
	{ NULL, 0, NULL, NULL, 0, NULL }
};

static xdebug_gdb_cmd commands_data[] = {
	{ "eval",       1, "eval [php code to execute]",                 xdebug_handle_eval,       1,
		"Evaluation PHP code"
	},
	{ "show",       0, "show",                                       xdebug_handle_show,       1,
		"Show a list of all variables"
	},
	{ "show-local", 0, "show-local",                                 xdebug_handle_show_local, 1,
		"Show a list of all variables including their contents"
	},
	{ "print",      1, "print",                                      xdebug_handle_print,      1,
		"Prints the contents of the variable"
	},
	{ NULL, 0, NULL, NULL, 0, NULL }
};

static xdebug_gdb_cmd commands_run[] = {
	{ "kill",       0, "kill",                                       xdebug_handle_kill,       1, "Kill the script" },
	{ "quit",       0, "quit",                                       xdebug_handle_quit,       1, "Close the debug session" },
	{ NULL, 0, NULL, NULL, 0, NULL }
};

static xdebug_gdb_cmd commands_runtime[] = {
	{ "backtrace",  0, "backtrace [full]",                           xdebug_handle_backtrace,  0,
		"Print backtrace of all stack frames.\n"
		"             Use of the 'full' qualifier also prints the values of the local\n"
		"             variables."
	},
	{ "bt",         0, "bt [full]",                                  xdebug_handle_backtrace,  1,
		"Print backtrace of all stack frames.\n"
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

static xdebug_gdb_cmd* scan_cmd(xdebug_gdb_cmd *ptr, char *line)
{
	while (ptr->name) {
		if (strcmp(ptr->name, line) == 0) {
			return ptr;
		}
		ptr++;
	}
	return NULL;
}


static char *make_message(xdebug_con *context, int error_code, char *message)
{
	xdebug_gdb_options *options = (xdebug_gdb_options*) context->options;
	char               *tmp;
	char               *ret;
	char               *type;
	int                 len;

	if ((error_code & XDEBUG_E) == XDEBUG_E) {
		type = "error";
	} else {
		type = "data";
	}
	
	switch (options->response_format) {
		case XDEBUG_RESPONSE_XML:
			/* we ignore binary safety here */
			tmp = xdebug_xmlize(message, strlen(message), &len);
			ret = xdebug_sprintf("<xdebug><%s><code>%d</code><message>%s</message></%s></xdebug>", type, error_code, tmp, type);
			efree(tmp);
			return ret;
			break;

		case XDEBUG_RESPONSE_NORMAL:
		default:
			return xdebug_sprintf("%d %s", error_code, message);
			break;
	}
}

static void send_message(xdebug_con *context, int error_code, char *message)
{
	char *tmp;

	tmp = make_message(context, error_code, message);
	SENDMSG(context->socket, xdebug_sprintf("%s\n", tmp));
	xdfree(tmp);
}


/*****************************************************************************
** Helpers for looking up commands
*/

static inline xdebug_gdb_cmd* lookup_cmd_in_group(char *line, xdebug_gdb_cmd *group, int flag, int test_flag)
{
	xdebug_gdb_cmd *ptr;

	if (flag & test_flag) {
		ptr = scan_cmd(group, line);
		if (ptr) {
			return (ptr);
		}
	}
	return NULL;
}

static xdebug_gdb_cmd* lookup_cmd(char *line, int flag)
{
	xdebug_gdb_cmd *ptr;
	
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

static inline void show_available_commands_in_group(xdebug_con *h, int fmt, int flag, int test_flag, xdebug_gdb_cmd *ptr)
{
	char *tmp;
	int   len;

	if (flag & test_flag ) {
    	while (ptr->name) {
			if (ptr->show && ptr->help) {
				switch (fmt) {
					case XDEBUG_RESPONSE_XML:
						/* we ignore binary safety here */
						tmp = xdebug_xmlize(ptr->help, strlen(ptr->help), &len);
						SENDMSG(h->socket, xdebug_sprintf("<command><name>%s</name><desc>%s</desc></command>", ptr->name, tmp));
						efree(tmp);
						break;
					default:
						SENDMSG(h->socket, xdebug_sprintf("%-12s %s\n", ptr->name, ptr->help));
						break;
				}
			}
	        ptr++;
	    }
	}
}

static void show_available_commands(xdebug_con *h, int flag)
{
	xdebug_gdb_options* o = (xdebug_gdb_options*) h->options;

	if (o->response_format == XDEBUG_RESPONSE_XML) {
		SENDMSG(h->socket, xdebug_sprintf("<xdebug><help>"));
	}
	show_available_commands_in_group(h, o->response_format, flag, XDEBUG_INIT,       commands_init);
	show_available_commands_in_group(h, o->response_format, flag, XDEBUG_BREAKPOINT, commands_breakpoint);
	show_available_commands_in_group(h, o->response_format, flag, XDEBUG_RUN,        commands_run);
	show_available_commands_in_group(h, o->response_format, flag, XDEBUG_RUNTIME,    commands_runtime);
	show_available_commands_in_group(h, o->response_format, flag, XDEBUG_DATA,       commands_data);
#if 0
	show_available_commands_in_group(h, o->response_format, flag, XDEBUG_STATUS,     commands_status);
#endif
	if (o->response_format == XDEBUG_RESPONSE_XML) {
		SENDMSG(h->socket, xdebug_sprintf("</help></xdebug>\n"));
	}
}

static void show_command_info(xdebug_con *h, xdebug_gdb_cmd* cmd)
{
	xdebug_gdb_options *o = (xdebug_gdb_options*) h->options;
	char               *t1, *t2;
	int                 len;

	if (cmd) {
		if (o->response_format == XDEBUG_RESPONSE_XML) {
			/* we ignore binary safety here */
			t1 = xdebug_xmlize(cmd->description, strlen(cmd->description), &len);
			t2 = xdebug_xmlize(cmd->help, strlen(cmd->help), &len);
			SENDMSG(h->socket, xdebug_sprintf("<xdebug><help><command><syntax>%s</syntax><desc>%s</desc></help</xdebug>\n", t1, t2));
			efree(t1);
			efree(t2);
		} else {
			SENDMSG(h->socket, xdebug_sprintf("Syntax: %s\n%12s %s\n", cmd->description, " ", cmd->help));
		}
	} else {
		send_message(h, XDEBUG_E_NO_INFO, "No information available for this command.");
	}
}

/*****************************************************************************
** Data printing functions
*/
static char *return_printable_symbol(xdebug_con *context, char *name, zval *val)
{
	xdebug_gdb_options* options = (xdebug_gdb_options*) context->options;
	char *str_rep, *ret;
	TSRMLS_FETCH();

	switch (options->response_format) {
	   	case XDEBUG_RESPONSE_NORMAL:
			if (val) {
				str_rep = xdebug_get_zval_value(val, 0, xdebug_var_get_nolimit_options(TSRMLS_C));
			} else {
				str_rep = xdstrdup("*uninitialized*");
			}
			if (name) {
				ret = xdebug_sprintf("$%s = %s\n", name, str_rep);
			} else {
				ret = xdebug_sprintf("%s\n", str_rep);
			}
			xdfree(str_rep);
			return ret;
			break;

		case XDEBUG_RESPONSE_XML:
		default:
			return xdebug_get_zval_value_xml(name, val);
	}
}

static void dump_used_var(void *context, xdebug_hash_element* he)
{
	char               *name = (char*) he->ptr;
	xdebug_con         *h = (xdebug_con*) context;
	xdebug_gdb_options *options = (xdebug_gdb_options*) h->options;

	if (!options->dump_superglobals) {
		if ((strcmp(name, "GLOBALS") == 0) ||
			(strcmp(name, "_GET") == 0) ||
			(strcmp(name, "_POST") == 0) ||
			(strcmp(name, "_COOKIE") == 0) ||
			(strcmp(name, "_REQUEST") == 0) ||
			(strcmp(name, "_SERVER") == 0) ||
			(strcmp(name, "_ENV") == 0) ||
			(strcmp(name, "_SESSION") == 0))
		{
			return;
		}
	}
	if (options->response_format == XDEBUG_RESPONSE_XML) {
		SENDMSG(h->socket, xdebug_sprintf("<var name='%s'/>", name));
	} else {
		SENDMSG(h->socket, xdebug_sprintf("$%s\n", name));
	}
}


static void dump_used_var_with_contents(void *context, xdebug_hash_element* he)
{
	char               *name = (char*) he->ptr;
	xdebug_con         *h = (xdebug_con*) context;
	xdebug_gdb_options *options = (xdebug_gdb_options*) h->options;
	char               *contents;
	zval               *zval_var;

	if (!options->dump_superglobals) {
		if ((strcmp(name, "GLOBALS") == 0) ||
			(strcmp(name, "_GET") == 0) ||
			(strcmp(name, "_POST") == 0) ||
			(strcmp(name, "_COOKIE") == 0) ||
			(strcmp(name, "_REQUEST") == 0) ||
			(strcmp(name, "_SERVER") == 0) ||
			(strcmp(name, "_ENV") == 0) ||
			(strcmp(name, "_SESSION") == 0))
		{
			return;
		}
	}
	zval_var = xdebug_get_php_symbol(name, strlen(name) + 1);
	contents = return_printable_symbol(context, name, zval_var);
	if (contents) {
		if (options->response_format == XDEBUG_RESPONSE_XML) {
			SENDMSG(h->socket, contents);
		} else {
			SENDMSG(h->socket, xdebug_sprintf("%s", contents));
			xdfree(contents);
		}
	} else {
		if (options->response_format == XDEBUG_RESPONSE_XML) {
			SENDMSG(h->socket, xdebug_sprintf("<var name='%s'/>", name));
		} else {
			SENDMSG(h->socket, xdebug_sprintf("$%s = *uninitialized*\n", name));
		}
	}
}

static void print_sourceline(xdebug_con *h, const char *file, int begin, int end, int offset, int response_format TSRMLS_DC)
{
	int    fd;
	fd_buf fd_buffer = { NULL, 0 };
	int    i = begin;
	char  *line = NULL;
	int    update = 0, len;
	char  *tmp;

	if (i < 0) {
		begin = 0;
		i = 0;
	}

	/* Read until the "begin" line has been read */
	if ((fd = open(file, 0)) == -1) {
		SENDMSG(h->socket, xdebug_sprintf("The file '%s' could not be opened.\n", file));
		return;
	}
	
	while (i > 0) {
		if (line) {
			free(line);
			line = NULL;
		}
		line = xdebug_fd_read_line(fd, &fd_buffer, FD_RL_FILE);
		i--;
	}
	/* Read until the "end" line has been read */
	do {
		if (line) {
			update = 1;
			if (response_format == XDEBUG_RESPONSE_XML) {
				/* we ignore binary safety here */
				tmp = xdebug_xmlize(line, strlen(line), &len);
				SENDMSG(h->socket, xdebug_sprintf("<line file='%s' no='%d'>%s</line>", file, begin + i, tmp));
				efree(tmp);
			} else {
				SENDMSG(h->socket, xdebug_sprintf("%d\t%s\n", begin + i, line));
			}
			free(line);
			line = NULL;
		}
		line = xdebug_fd_read_line(fd, &fd_buffer, FD_RL_FILE);
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
	close(fd);
}

static void print_breakpoint(xdebug_con *h, function_stack_entry *i, int response_format)
{
	int   c = 0; /* Comma flag */
	int   j = 0; /* Counter */
	char *tmp_fname;
	char *tmp;
	char *tmp_value;
	int   xml = (response_format == XDEBUG_RESPONSE_XML);
	int   len;
	TSRMLS_FETCH();
/*
* Breakpoint 2, xdebug_execute (op_array=0x82caf50)
*     at /dat/dev/php/xdebug/xdebug.c:361
*/
	tmp_fname = xdebug_show_fname(i->function, 0, 0 TSRMLS_CC);
	if (xml) {
		SENDMSG(h->socket, xdebug_sprintf("<breakpoint><function><name>%s</name><params>", tmp_fname));
	} else {
		SENDMSG(h->socket, xdebug_sprintf("Breakpoint, %s(", tmp_fname));
	}
	xdfree(tmp_fname);

	/* Printing vars */
	for (j = 0; j < i->varc; j++) {
		if (c) {
			SSEND(h->socket, ", ");
		} else {
			c = 1;
		}

		if (i->var[j].name) {
		   SENDMSG(h->socket, xdebug_sprintf("$%s = ", i->var[j].name));
		}
		tmp_value = xdebug_get_zval_value(i->var[j].addr, 0, xdebug_var_get_nolimit_options(TSRMLS_C));
		/* we ignore binary safety here */
		tmp = xdebug_xmlize(tmp_value, strlen(tmp_value), &len);
		SSENDL(h->socket, tmp, len);
		xdfree(tmp_value);
		efree(tmp);
	}

	if (xml) {
		SENDMSG(h->socket, xdebug_sprintf("</params></function><file>%s</file><line>%d</line></breakpoint>", i->filename, i->lineno));
	} else {
		SENDMSG(h->socket, xdebug_sprintf(")\n\tat %s:%d\n", i->filename, i->lineno));
	}
}

static void print_stackframe(xdebug_con *h, int nr, function_stack_entry *i, int response_format, int flags)
{
	int c = 0; /* Comma flag */
	int j = 0; /* Counter */
	char *tmp_fname;
	char *tmp;
	char *tmp_value;
	int   len;
	xdebug_hash *tmp_hash;
	TSRMLS_FETCH();
	
/*
* 0x4001af2e in xdebug_compile_file (file_handle=0xbffff960, type=2)
*     at /dat/dev/php/xdebug/xdebug.c:901
*         
*/
   	tmp_fname = xdebug_show_fname(i->function, 0, 0 TSRMLS_CC);
	if (response_format == XDEBUG_RESPONSE_XML) {
		if (nr) {
			SENDMSG(h->socket, xdebug_sprintf("<stackframe><level>%d</level><function><name>%s</name><params>", nr, tmp_fname));
		} else {
			SENDMSG(h->socket, xdebug_sprintf("<stackframe><function><name>%s</name><params>", tmp_fname));
		}
	} else {
		if (nr) {
			SENDMSG(h->socket, xdebug_sprintf("#%-2d %s (", nr, tmp_fname));
		} else {
			SENDMSG(h->socket, xdebug_sprintf("%s (", tmp_fname));
		}
	}
	xdfree(tmp_fname);

	/* Printing vars */
	for (j = 0; j < i->varc; j++) {
		if (c) {
			SSEND(h->socket, ", ");
		} else {
			c = 1;
		}

		if (i->var[j].name) {
		   SENDMSG(h->socket, xdebug_sprintf("$%s = ", i->var[j].name));
		}
		tmp_value = xdebug_get_zval_value(i->var[j].addr, 0, xdebug_var_get_nolimit_options(TSRMLS_C));
		/* we ignore binary safety here */
		tmp = xdebug_xmlize(tmp_value, strlen(tmp_value), &len);
		SSENDL(h->socket, tmp, len);
		xdfree(tmp_value);
		efree(tmp);
	}

	XG(active_symbol_table) = i->symbol_table;
	if (response_format == XDEBUG_RESPONSE_XML) {
		SENDMSG(h->socket, xdebug_sprintf("</params></function><file>%s</file><line>%d</line>", i->filename, i->lineno));
		if (flags & XDEBUG_FRAME_FULL && i->used_vars) {
			SSEND(h->socket, "<locals>");
			tmp_hash = xdebug_used_var_hash_from_llist(i->used_vars);
			xdebug_hash_apply(tmp_hash, (void *) h, dump_used_var_with_contents);
			xdebug_hash_destroy(tmp_hash);
			SSEND(h->socket, "</locals>");
		}
		SSEND(h->socket, "</stackframe>");
	} else {
		SENDMSG(h->socket, xdebug_sprintf(")\n    at %s:%d\n", i->filename, i->lineno));
		if (flags & XDEBUG_FRAME_FULL && i->used_vars) {
			tmp_hash = xdebug_used_var_hash_from_llist(i->used_vars);
			xdebug_hash_apply(tmp_hash, (void *) h, dump_used_var_with_contents);
			xdebug_hash_destroy(tmp_hash);
			SSEND(h->socket, "\n");
		}
	}
	XG(active_symbol_table) = NULL;
}

/*****************************************************************************
** Client command handlers
*/

char *xdebug_handle_backtrace(xdebug_con *context, xdebug_arg *args)
{
	xdebug_llist_element *le;
	int                   counter = 1;
	int                   full = XDEBUG_FRAME_NORMAL;
	xdebug_gdb_options   *options = (xdebug_gdb_options*) context->options;
	int                   xml = (options->response_format == XDEBUG_RESPONSE_XML);
	TSRMLS_FETCH();

	if (args->c == 1 && strcmp(args->args[0], "full") == 0) {
		full = XDEBUG_FRAME_FULL;
	}

	SSEND(context->socket, xml ? "<xdebug><backtrace>" : "");
	for (le = XDEBUG_LLIST_TAIL(XG(stack)); le != NULL; le = XDEBUG_LLIST_PREV(le)) {
		print_stackframe(context, counter++, XDEBUG_LLIST_VALP(le), options->response_format, full);
	}
	SSEND(context->socket, xml ? "</backtrace></xdebug>\n" : "\n");

	return NULL;
}

char *xdebug_handle_breakpoint(xdebug_con *context, xdebug_arg *args)
{
	xdebug_arg      *method = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));
	char            *tmp_name;
	xdebug_brk_info *extra_brk_info;
	xdebug_str       eval = {0, 0, NULL};
	int              i = 0;

	xdebug_arg_init(method);

	extra_brk_info = xdmalloc(sizeof(xdebug_brk_info));
	extra_brk_info->type = NULL;
	extra_brk_info->file = NULL;
	extra_brk_info->condition = NULL;
	extra_brk_info->classname = NULL;
	extra_brk_info->functionname = NULL;
	extra_brk_info->disabled = 0;
	extra_brk_info->temporary = 0;
	extra_brk_info->hit_count = 0;
	extra_brk_info->hit_value = 0;
	extra_brk_info->hit_condition = XDEBUG_HIT_DISABLED;
	extra_brk_info->function_break_type = XDEBUG_BRK_FUNC_CALL;

	if (strstr(args->args[0], "::")) { /* class::method */
		xdebug_explode("::", args->args[0], method, -1);
		extra_brk_info->classname = xdstrdup(method->args[0]);
		extra_brk_info->functionname = xdstrdup(method->args[1]);

		if (method->c != 2) {
			xdebug_arg_dtor(method);
			xdebug_brk_info_dtor(extra_brk_info);
			return make_message(context, XDEBUG_E_INVALID_FORMAT, "Invalid format for class/method combination.");
		} else {
			if (!xdebug_hash_add(context->class_breakpoints, args->args[0], strlen(args->args[0]), (void*) extra_brk_info)) {
				xdebug_arg_dtor(method);
				xdebug_brk_info_dtor(extra_brk_info);
				return make_message(context, XDEBUG_E_BREAKPOINT_NOT_SET, "Breakpoint could not be set.");
			} else {
				send_message(context, XDEBUG_D_BREAKPOINT_SET, "Breakpoint set on class/method combination.");
				xdebug_arg_dtor(method);
			}
		}
	} else if (strstr(args->args[0], "->")) { /* class->method */
		xdebug_explode("->", args->args[0], method, -1);
		extra_brk_info->classname = xdstrdup(method->args[0]);
		extra_brk_info->functionname = xdstrdup(method->args[1]);

		if (method->c != 2) {
			xdebug_arg_dtor(method);
			xdebug_brk_info_dtor(extra_brk_info);
			return make_message(context, XDEBUG_E_INVALID_FORMAT, "Invalid format for class/method combination.");
		} else {
			if (!xdebug_hash_add(context->class_breakpoints, args->args[0], strlen(args->args[0]), (void*) extra_brk_info)) {
				xdebug_arg_dtor(method);
				xdebug_brk_info_dtor(extra_brk_info);
				return make_message(context, XDEBUG_E_BREAKPOINT_NOT_SET, "Breakpoint could not be set.");
			} else {
				send_message(context, XDEBUG_D_BREAKPOINT_SET, "Breakpoint set on class/method combination.");
				xdebug_arg_dtor(method);
			}
		}
	} else if (strstr(args->args[0], ":")) { /* file:line */
		xdebug_explode(":", args->args[0], method, -1); /* 0 = filename, 1 = linenumer */
		if (method->c != 2) {
			xdebug_arg_dtor(method);
			return make_message(context, XDEBUG_E_INVALID_FORMAT, "Invalid format for file:line combination.");
		} else {
			/* Make search key */
			if (method->args[0][0] != '/') {
#if WIN32|WINNT
				if (strlen(method->args[0]) > 3
					&& method->args[0][1] != '|')
				{
					tmp_name = xdebug_sprintf("/%s", method->args[0]);
				} else {
					tmp_name = xdebug_sprintf("%s", method->args[0]);
				}
#else
				tmp_name = xdebug_sprintf("/%s", method->args[0]);
#endif
			} else {
				tmp_name = xdebug_sprintf("%s", method->args[0]);
			}

			/* Set line number in extra structure */
			extra_brk_info->lineno = atoi(method->args[1]);
			extra_brk_info->file = tmp_name;
			extra_brk_info->file_len = strlen(tmp_name);

			/* If there are more second parameters to the "break" command, then we
			 * concat them and use it as a conditional statement */
			if (args->c > 1) {
				xdebug_str_add(&eval, args->args[1], 0);

				for (i = 2; i < args->c; i++) {
					xdebug_str_add(&eval, " ", 0);
					xdebug_str_add(&eval, args->args[i], 0);
				}
				extra_brk_info->condition = xdstrdup(eval.d);
				xdebug_str_free(&eval);
			}

			/* Add breakpoint to the list */
			xdebug_llist_insert_next(context->line_breakpoints, XDEBUG_LLIST_TAIL(context->line_breakpoints), (void*) extra_brk_info);
			send_message(context, XDEBUG_D_BREAKPOINT_SET, "Breakpoint set on file:line combination.");
			xdebug_arg_dtor(method);
		}
	} else { /* function */
		extra_brk_info->functionname = xdstrdup(args->args[0]);

		if (!xdebug_hash_add(context->function_breakpoints, args->args[0], strlen(args->args[0]), (void*) extra_brk_info)) {
			xdebug_arg_dtor(method);
			xdebug_brk_info_dtor(extra_brk_info);
			return make_message(context, XDEBUG_E_BREAKPOINT_NOT_SET, "Breakpoint could not be set.");
		} else {
			send_message(context, XDEBUG_D_BREAKPOINT_SET, "Breakpoint set on function.");
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
	xdebug_llist_element *le;
	xdebug_brk_info      *brk;
	TSRMLS_FETCH();

	xdebug_arg_init(method);

	if (strstr(args->args[0], "::")) { /* class::method */
		xdebug_explode("::", args->args[0], method, -1);
		if (method->c != 2) {
			xdebug_arg_dtor(method);
			return make_message(context, XDEBUG_E_INVALID_FORMAT, "Invalid format for class/method combination.");
		} else {
			if (!xdebug_hash_delete(context->class_breakpoints, args->args[0], strlen(args->args[0]))) {
				xdebug_arg_dtor(method);
				return make_message(context, XDEBUG_E_BREAKPOINT_NOT_REMOVED, "Breakpoint could not be removed.");
			} else {
				send_message(context, XDEBUG_D_BREAKPOINT_REMOVED, "Breakpoint removed.");
				xdebug_arg_dtor(method);
			}
		}
	} else if (strstr(args->args[0], "->")) { /* class->method */
		xdebug_explode("->", args->args[0], method, -1);
		if (method->c != 2) {
			xdebug_arg_dtor(method);
			return make_message(context, XDEBUG_E_INVALID_FORMAT, "Invalid format for class/method combination.");
		} else {
			if (!xdebug_hash_delete(context->class_breakpoints, args->args[0], strlen(args->args[0]))) {
				xdebug_arg_dtor(method);
				return make_message(context, XDEBUG_E_BREAKPOINT_NOT_REMOVED, "Breakpoint could not be removed.");
			} else {
				send_message(context, XDEBUG_D_BREAKPOINT_REMOVED, "Breakpoint removed.");
				xdebug_arg_dtor(method);
			}
		}
	} else if (strstr(args->args[0], ":")) { /* file:line */
		xdebug_explode(":", args->args[0], method, -1); /* 0 = filename, 1 = linenumer */
		if (method->c != 2) {
			xdebug_arg_dtor(method);
			return make_message(context, XDEBUG_E_INVALID_FORMAT, "Invalid format for file:line combination.");
		} else {
			/* Make search key */
			if (method->args[0][0] != '/') {
#if WIN32|WINNT
				if (strlen(method->args[0]) > 3
					&& method->args[0][1] != '|')
				{
					tmp_name = xdebug_sprintf("/%s", method->args[0]);
				} else {
					tmp_name = xdebug_sprintf("%s", method->args[0]);
				}
#else
				tmp_name = xdebug_sprintf("/%s", method->args[0]);
#endif
			} else {
				tmp_name = xdebug_sprintf("%s", method->args[0]);
			}

			/* Add breakpoint to the list */
			for (le = XDEBUG_LLIST_HEAD(XG(context).line_breakpoints); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
				brk = XDEBUG_LLIST_VALP(le);

				if (atoi(method->args[1]) == brk->lineno && strcmp(tmp_name, brk->file) == 0) {
					xdebug_llist_remove(context->line_breakpoints, le, NULL);
					send_message(context, XDEBUG_D_BREAKPOINT_REMOVED, "Breakpoint removed.");
					xdebug_arg_dtor(method);
					xdfree(tmp_name);
					return NULL;
				}
			}
			xdfree(tmp_name);
			xdebug_arg_dtor(method);
		}
	} else { /* function */
		if (!xdebug_hash_delete(context->function_breakpoints, args->args[0], strlen(args->args[0]))) {
			xdebug_arg_dtor(method);
			return make_message(context, XDEBUG_E_BREAKPOINT_NOT_REMOVED, "Breakpoint could not be removed.");
		} else {
			send_message(context, XDEBUG_D_BREAKPOINT_REMOVED, "Breakpoint removed.");
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
	int        eval_status;

	TSRMLS_FETCH();

	/* Remember error reporting level and debug session state */
	old_error_reporting     = EG(error_reporting);
	EG(error_reporting)     = 0;
	XG(breakpoints_allowed) = 0;
	XG(remote_enabled)      = 0;
	XG(remote_enable)       = 0;

	/* Concat all arguments back together */
	xdebug_str_add(&buffer, args->args[0], 0);

	for (i = 1; i < args->c; i++) {
		xdebug_str_add(&buffer, " ", 0);
		xdebug_str_add(&buffer, args->args[i], 0);
	}

	/* Run eval statement */
	eval_status = zend_eval_string(buffer.d, &retval, "xdebug eval" TSRMLS_CC);
	xdebug_str_free(&buffer);

	/* Restore error reporting level and debug session state */
	EG(error_reporting)     = old_error_reporting;
	XG(breakpoints_allowed) = 1;
	XG(remote_enabled)      = 1;
	XG(remote_enable)       = 1;

	/* Return error message */
	if (eval_status == FAILURE) {
		return make_message(context, XDEBUG_E_EVAL, "Error evaluating code");
	} 

	/* Return data */
	ret_value = return_printable_symbol(context, NULL, &retval);
	SENDMSG(context->socket, xdebug_sprintf("%s\n", ret_value));
	zval_dtor(&retval);
	xdfree(ret_value);
	return NULL;
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
	char               *tmp_file  = NULL;
	int                 tmp_begin = 0;
	int                 tmp_end   = 0;
	xdebug_arg         *parts = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));
	xdebug_gdb_options *options = (xdebug_gdb_options*) context->options;
	int                 xml = (options->response_format == XDEBUG_RESPONSE_XML);
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
			return make_message(context, XDEBUG_E_TOO_MANY_ARGUMENTS, "Too many arguments.");
			break;
	}
	SSEND(context->socket, xml ? "<xdebug><list>" : "");
	print_sourceline(context, tmp_file, tmp_begin, tmp_end, 0, options->response_format TSRMLS_CC);
	SSEND(context->socket, xml ? "</list></xdebug>\n" : "\n");

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
	xdebug_gdb_options *options = (xdebug_gdb_options*)context->options;

	if (strcmp(args->args[0], "response_format") == 0) {
		options->response_format = atoi(args->args[1]);
	} else if (strcmp(args->args[0], "dump_superglobals") == 0) {
		options->dump_superglobals = atoi(args->args[1]);
	} else {
		return make_message(context, XDEBUG_E_UNKNOWN_OPTION, "Unknown option.");
	}

	return NULL;
}

char *xdebug_handle_print(xdebug_con *context, xdebug_arg *args)
{
	xdebug_gdb_options  *options = (xdebug_gdb_options*)context->options;
	int                  xml = (options->response_format == XDEBUG_RESPONSE_XML);
	char                *var_data;
	zval                *zvar;
	TSRMLS_FETCH();

	XG_INIT_SYMBOL_TABLE
	XG(active_symbol_table) = EG(active_symbol_table);
	zvar = xdebug_get_php_symbol(args->args[0], strlen(args->args[0]) + 1);
	if (zvar) {
		var_data = return_printable_symbol(context, args->args[0], zvar);
		XG(active_symbol_table) = NULL;

		if (var_data) {
			SSEND(context->socket, xml ? "<xdebug><print>" : "");
			SENDMSG(context->socket, var_data);
			SSEND(context->socket, xml ? "</print></xdebug>\n" : "\n");
			return NULL;
		}
	}

	return make_message(context, XDEBUG_E_SYMBOL_NOT_FOUND, "This symbol does not exist or is not yet initialized.");
}

char *xdebug_handle_pwd(xdebug_con *context, xdebug_arg *args)
{
	char                buffer[256 + 1];
	xdebug_gdb_options *options = (xdebug_gdb_options*) context->options;

	if (getcwd(buffer, 256)) {
		if (options->response_format == XDEBUG_RESPONSE_XML) {
			SENDMSG(context->socket, xdebug_sprintf("<xdebug><pwd><directory>%s</directory></pwd></xdebug>\n", buffer));
		} else {
			SENDMSG(context->socket, xdebug_sprintf("Working directory %s.\n", buffer));
		}
	}
	return NULL;
}

char *xdebug_handle_quit(xdebug_con *context, xdebug_arg *args)
{
	return NULL;
}

char *xdebug_handle_run(xdebug_con *context, xdebug_arg *args)
{
	xdebug_gdb_options          *options = (xdebug_gdb_options*) context->options;

	if (options->response_format == XDEBUG_RESPONSE_XML) {
		SENDMSG(context->socket, xdebug_sprintf("<xdebug><run><program>%s</program></run></xdebug>\n", context->program_name));
	} else {
		SENDMSG(context->socket, xdebug_sprintf("Starting program: %s\n", context->program_name));
	}
	return NULL;
}


static void dump_class_breakpoint(void *context, xdebug_hash_element* he)
{
	xdebug_con         *h = (xdebug_con*) context;
	xdebug_gdb_options *options = (xdebug_gdb_options*) h->options;
	xdebug_brk_info    *brk_info = (xdebug_brk_info*) he->ptr;

	if (options->response_format == XDEBUG_RESPONSE_XML) {
		SENDMSG(
			h->socket,
			xdebug_sprintf("<breakpoint type='class'><class>%s</class><method>%s</method></breakpoint>",
			brk_info->classname, brk_info->functionname));
	} else {
		SENDMSG(
			h->socket,
			xdebug_sprintf("Method breakpoint: %s::%s\n",
			brk_info->classname, brk_info->functionname));
	}
}

static void dump_function_breakpoint(void *context, xdebug_hash_element* he)
{
	xdebug_con         *h = (xdebug_con*) context;
	xdebug_gdb_options *options = (xdebug_gdb_options*) h->options;
	xdebug_brk_info    *brk_info = (xdebug_brk_info*) he->ptr;

	if (options->response_format == XDEBUG_RESPONSE_XML) {
		SENDMSG(h->socket, xdebug_sprintf("<breakpoint type='function'><function>%s</function></breakpoint>", brk_info->functionname));
	} else {
		SENDMSG(h->socket, xdebug_sprintf("Function breakpoint: %s\n", brk_info->functionname));
	}
}

static void dump_line_breakpoint(xdebug_con *h, xdebug_gdb_options *options, xdebug_brk_info* brk_info)
{
	char *condition = NULL;
	int   len;

	if (options->response_format == XDEBUG_RESPONSE_XML) {
		if (condition) {
			/* we ignore binary safety here */
			condition = xdebug_xmlize(brk_info->condition, strlen(brk_info->condition), &len);
			SENDMSG(h->socket,
				xdebug_sprintf("<breakpoint type='line' condition='%s'><file>%s</file><line>%d</line></breakpoint>",
				condition,
				brk_info->file,
				brk_info->lineno));
			xdfree(condition);
		} else {
			SENDMSG(h->socket, xdebug_sprintf("<breakpoint type='line'><file>%s</file><line>%d</line></breakpoint>",
				brk_info->file,
				brk_info->lineno));
		}
	} else {
		if (condition) {
			SENDMSG(h->socket,
				xdebug_sprintf("Location breakpoint: %s:%d (condition= %s)\n",
				brk_info->file,
				brk_info->lineno,
				brk_info->condition));
		} else {
			SENDMSG(h->socket,
				xdebug_sprintf("Location breakpoint: %s:%d\n",
				brk_info->file,
				brk_info->lineno));
		}
	}
}


char *xdebug_handle_show_breakpoints(xdebug_con *context, xdebug_arg *args)
{
	xdebug_gdb_options   *options = (xdebug_gdb_options*) context->options;
	xdebug_llist_element *le;

	if (options->response_format == XDEBUG_RESPONSE_XML) {
		SSEND(context->socket, "<breakpoints>");
	}

	xdebug_hash_apply(context->function_breakpoints, (void *) context, dump_function_breakpoint);
	xdebug_hash_apply(context->class_breakpoints, (void *) context, dump_class_breakpoint);

	for (le = XDEBUG_LLIST_TAIL(context->line_breakpoints); le != NULL; le = XDEBUG_LLIST_PREV(le)) {
		dump_line_breakpoint(context, options, XDEBUG_LLIST_VALP(le));
	}

	if (options->response_format == XDEBUG_RESPONSE_XML) {
		SSEND(context->socket, "</breakpoints>\n");
	}
	return NULL;
}


static char* show_local_vars(xdebug_con *context, xdebug_arg *args, void (*func)(void *, xdebug_hash_element*))
{
	function_stack_entry *i;
	xdebug_hash          *tmp_hash;
	xdebug_gdb_options   *options = (xdebug_gdb_options*) context->options;
	TSRMLS_FETCH();

	
	if (XDEBUG_LLIST_TAIL(XG(stack))) {
		i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack)));

		/* Only show vars when they are scanned */
		if (i->used_vars) {
			if (options->response_format == XDEBUG_RESPONSE_XML) {
				SSEND(context->socket, "<xdebug><show>");
			}

			tmp_hash = xdebug_used_var_hash_from_llist(i->used_vars);
			xdebug_hash_apply(tmp_hash, (void *) context, func);
			xdebug_hash_destroy(tmp_hash);

			if (options->response_format == XDEBUG_RESPONSE_XML) {
				SSEND(context->socket, "</show></xdebug>\n");
			}
		} else {
			return make_message(context, XDEBUG_E_NOT_USER_DEFINED, "You can not show variables in functions not defined in your script.");
		}
	}
	
	return NULL;
}

char *xdebug_handle_show(xdebug_con *context, xdebug_arg *args)
{
	return show_local_vars(context, args, dump_used_var);
}


char *xdebug_handle_show_local(xdebug_con *context, xdebug_arg *args)
{
	char *tmp;
	TSRMLS_FETCH();
	
	XG_INIT_SYMBOL_TABLE
	XG(active_symbol_table) = EG(active_symbol_table);
	tmp = show_local_vars(context, args, dump_used_var_with_contents);
	XG(active_symbol_table) = NULL;
	return tmp;
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

static int xdebug_gdb_parse_option(xdebug_con *context, char* line, int flags, char *end_cmd, char **error)
{
	char *ptr;
	xdebug_gdb_cmd *cmd;
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
			*error = make_message(context, XDEBUG_E_UNDEFINED_COMMAND, "Undefined command, try \"help\".");
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
				*error = make_message(context, XDEBUG_E_UNDEFINED_COMMAND, "Undefined command, try \"help\".");
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
			*error = make_message(context, XDEBUG_E_UNDEFINED_COMMAND, "Undefined command, try \"help\".");
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

char *xdebug_gdb_get_revision(void)
{
	return "$Revision: 1.89 $";
}

int xdebug_gdb_init(xdebug_con *context, int mode)
{
	char *option;
	int   ret;
	char *error = NULL;
	xdebug_gdb_options *options;
	TSRMLS_FETCH();

	SENDMSG(context->socket, xdebug_sprintf("This is Xdebug version %s.\n", XDEBUG_VERSION));
	SSEND(context->socket, "Copyright 2002, 2003, 2004, 2005, 2006, 2007 by Derick Rethans,\n");
	context->buffer = xdmalloc(sizeof(fd_buf));
	context->buffer->buffer = NULL;
	context->buffer->buffer_size = 0;

	context->options = xdmalloc(sizeof(xdebug_gdb_options));
	options = (xdebug_gdb_options*) context->options;
	options->response_format = XDEBUG_RESPONSE_NORMAL;
	options->dump_superglobals = 1;

	/* Initialize auto globals in Zend Engine 2 */
	zend_is_auto_global("_ENV",     sizeof("_ENV")-1     TSRMLS_CC);
	zend_is_auto_global("_GET",     sizeof("_GET")-1     TSRMLS_CC);
	zend_is_auto_global("_POST",    sizeof("_POST")-1    TSRMLS_CC);
	zend_is_auto_global("_COOKIE",  sizeof("_COOKIE")-1  TSRMLS_CC);
	zend_is_auto_global("_REQUEST", sizeof("_REQUEST")-1 TSRMLS_CC);
	zend_is_auto_global("_FILES",   sizeof("_FILES")-1   TSRMLS_CC);
	zend_is_auto_global("_SERVER",  sizeof("_SERVER")-1  TSRMLS_CC);

	context->function_breakpoints = xdebug_hash_alloc(64, (xdebug_hash_dtor) xdebug_hash_brk_dtor);
	context->exception_breakpoints = xdebug_hash_alloc(64, (xdebug_hash_dtor) xdebug_hash_brk_dtor);
	context->class_breakpoints = xdebug_hash_alloc(64, (xdebug_hash_dtor) xdebug_hash_brk_dtor);
	context->line_breakpoints = xdebug_llist_alloc((xdebug_llist_dtor) xdebug_llist_brk_dtor);
	do {
		SENDMSG(context->socket, xdebug_sprintf("?init %s\n", context->program_name));
		option = xdebug_fd_read_line(context->socket, context->buffer, FD_RL_SOCKET);
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
	xdfree(context->options);
	xdebug_hash_destroy(context->function_breakpoints);
	xdebug_hash_destroy(context->exception_breakpoints);
	xdebug_hash_destroy(context->class_breakpoints);
	xdebug_llist_destroy(context->line_breakpoints, NULL);
	xdfree(context->buffer);

	return 1;
}

int xdebug_gdb_error(xdebug_con *context, int type, char *exception_type, char *message, const char *file, const uint lineno, xdebug_llist *stack)
{
	char               *errortype;
	int                 ret;
	char               *option;
	char               *error = NULL;
	int                 runtime_allowed;
	xdebug_gdb_options *options = (xdebug_gdb_options*) context->options;

	if (exception_type) {
		errortype = exception_type;
	} else {
		errortype = xdebug_error_type(type);
	}

	runtime_allowed = (
		(type != E_ERROR) && 
		(type != E_CORE_ERROR) &&
		(type != E_COMPILE_ERROR) &&
		(type != E_USER_ERROR)
	) ? XDEBUG_BREAKPOINT | XDEBUG_RUNTIME : 0;

	if (options->response_format == XDEBUG_RESPONSE_XML) {
		SENDMSG(context->socket, xdebug_sprintf("<xdebug><signal><code>%d</code><type>%s</type><message>%s</message><file>%s</file><line>%lu</line><stack>", type, errortype, message, file, lineno));
		print_stackframe(context, 0, XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(stack)), options->response_format, XDEBUG_FRAME_NORMAL);
		SENDMSG(context->socket, xdebug_sprintf("</stack></signal></xdebug>\n"));
	} else {
		SENDMSG(context->socket, xdebug_sprintf("\nProgram received signal %s: %s.\n", errortype, message));
		print_stackframe(context, 0, XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(stack)), options->response_format, XDEBUG_FRAME_NORMAL);
	}

	if (!exception_type) {
		xdfree(errortype);
	}

	do {
		SSEND(context->socket, "?cmd\n");
		option = xdebug_fd_read_line(context->socket, context->buffer, FD_RL_SOCKET);
		if (!option) {
			return 0;
		}
		ret = xdebug_gdb_parse_option(context, option, XDEBUG_DATA | XDEBUG_RUN | runtime_allowed | XDEBUG_STATUS, "cont,continue", (char**) &error);
		xdebug_gdb_option_result(context, ret, error);
		free(option);
	} while (1 != ret);

	return 1;
}

int xdebug_gdb_breakpoint(xdebug_con *context, xdebug_llist *stack, const char *file, long lineno, int type, char *exception_type, char *message)
{
	function_stack_entry *i;
	int    ret;
	char  *option;
	char  *error = NULL;
	xdebug_gdb_options *options = (xdebug_gdb_options*) context->options;
	int                 xml = (options->response_format == XDEBUG_RESPONSE_XML);
	TSRMLS_FETCH();

	i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(stack));

	SSEND(context->socket, xml ? "<xdebug><break>" : "");
	if (type == XDEBUG_BREAK) {
		print_breakpoint(context, i, options->response_format);
	}
	print_sourceline(context, file, lineno, lineno, -1, options->response_format TSRMLS_CC);
	SSEND(context->socket, xml ? "</break></xdebug>\n" : "\n");

	do {
		SSEND(context->socket, "?cmd\n");
		option = xdebug_fd_read_line(context->socket, context->buffer, FD_RL_SOCKET);
		if (!option) {
			return 0;
		}
		ret = xdebug_gdb_parse_option(context, option, XDEBUG_BREAKPOINT | XDEBUG_DATA | XDEBUG_RUN | XDEBUG_RUNTIME | XDEBUG_STATUS, "cont,continue,step,next,finish", (char**) &error);
		xdebug_gdb_option_result(context, ret, error);
		free(option);
	} while (1 != ret);

	return 1;
}
