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

char *xdebug_handle_breakpoint(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_option(xdebug_con *context, xdebug_arg *args);
char *xdebug_handle_run(xdebug_con *context, xdebug_arg *args);

static xdebug_cmd commands_init[] = {
	{ "option", 2, "option [setting] [value]", xdebug_handle_option },
	{ "run",    0, "run", xdebug_handle_run },
	{ NULL,     0, NULL }
};

static xdebug_cmd commands_breakpoint[] = {
	{ "break",  1, "bre [functionname|filename:linenumber]", xdebug_handle_breakpoint },
	{ "bre",    1, "bre [functionname|filename:linenumber]", xdebug_handle_breakpoint },
	{ NULL,     0, NULL }
};


static char *find_hostname(void)
{
	char tmpname[33];
	int err;
	
	memset(tmpname, 0, sizeof(tmpname));
	err = gethostname(tmpname, sizeof(tmpname) - 1);
	if (err == -1) {
		return NULL;
	}
	return (char *) xdstrdup(tmpname);
}


static char *get_current_time(void)
{
	static char debug_timebuf[50];
	char microbuf[10];
#if HAVE_GETTIMEOFDAY
	struct timeval tv;
	struct timezone tz;
#endif
	const struct tm *tm;
	size_t len;
	time_t t;

	memset(debug_timebuf, 0, sizeof(debug_timebuf));
	t = time(NULL);
	tm = localtime((const time_t *) &t);
	len = strftime(debug_timebuf, (sizeof(debug_timebuf) - sizeof(microbuf) - 1), "%Y-%m-%d %H:%M", tm);

#if HAVE_GETTIMEOFDAY
	gettimeofday(&tv, &tz);
	snprintf(microbuf, sizeof(microbuf) - 1, ":%06d", tv.tv_usec);
	strcat(debug_timebuf, microbuf);
#endif
	return debug_timebuf;
}


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

static xdebug_cmd* lookup_cmd(char *line, int flag)
{
	xdebug_cmd *ptr;
	
	if (flag & XDEBUG_INIT) {
		ptr = scan_cmd(commands_init, line);
		if (ptr) {
			return (ptr);
		}
	}
	if (flag & XDEBUG_BREAKPOINT) {
		ptr = scan_cmd(commands_breakpoint, line);
		if (ptr) {
			return (ptr);
		}
	}
	return NULL;
}


char *xdebug_handle_breakpoint(xdebug_con *context, xdebug_arg *args)
{
	printf ("handle breakpoint!\n");

	if (strstr(args->args[0], "::")) { /* class::method */
		return xdstrdup("Class::method breakpoints are not yet supported.");
	} else if (strstr(args->args[0], ":")) { /* file:line */
		return xdstrdup("File:line breakpoints are not yet supported.");
	} else { /* function */
		printf ("function breakpoint on '%s'\n", args->args[0]);
		if (!xdebug_hash_add(context->function_breakpoints, args->args[0], strlen(args->args[0]), (void*) args->args[0])) {
			return xdstrdup("Breakpoint already set");
		}
	}
	return NULL;
}

char *xdebug_handle_option(xdebug_con *context, xdebug_arg *args)
{
	printf ("handle option!\n");
	return NULL;
}

char *xdebug_handle_run(xdebug_con *context, xdebug_arg *args)
{
	printf ("handle run!\n");
	return NULL;
}


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
		if (!(cmd = lookup_cmd(line, flags))) {
			return -1;
		}
	} else {
		char *tmp = (char*) xdmalloc(ptr - line + 1);
		memcpy(tmp, line, ptr - line);
		tmp[ptr - line] = '\0';
		if (cmd = lookup_cmd(tmp, flags)) {
			xdfree(tmp);
			xdebug_explode(" ", ptr + 1, args, -1); 
		} else {
			xdfree(tmp);
			return -1;
		}
	}
	/* Default in continue mode */
	retval = 0;
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


int xdebug_gdb_init(xdebug_con *context, int mode)
{
	char *option;
	int   ret;
	char *error = NULL;

	SSEND(context->socket, "hello\n");
	context->buffer = xdmalloc(sizeof(xdebug_socket_buf));
	context->buffer->buffer = NULL;
	context->buffer->buffer_size = 0;
	context->function_breakpoints = xdebug_hash_alloc(64, NULL);
	do {
		SSEND(context->socket, "?init\n");
		option = xdebug_socket_read_line(context->socket, context->buffer);
		if (!option) {
			return 0;
		}
		printf ("[%s]\n", option);
		ret = xdebug_gdb_parse_option(context, option, XDEBUG_INIT | XDEBUG_BREAKPOINT | XDEBUG_STATUS, "run", (char**) &error);
		if (error || ret == -1) {
			SSEND(context->socket, "+ERROR");
			if (error) {
				SSEND(context->socket, ": ");
				SSEND(context->socket, error);
			} else {
				SSEND(context->socket, "\n");
			}
		} else {
			SSEND(context->socket, "+OK\n");
		}
	} while (1 != ret);

	return 1;
}

int xdebug_gdb_deinit(xdebug_con *context)
{
	SSEND(context->socket, "bye\n");
	xdebug_hash_destroy(context->function_breakpoints);
}

#define SENDMSG(socket, str) {  \
	char *message_buffer;       \
                                \
	message_buffer = str;       \
	SSEND(socket, message_buffer); \
	xdfree(message_buffer);     \
}

int xdebug_gdb_error(xdebug_con *h, int type, char *message, const char *location, const uint line, xdebug_llist *stack)
{
	char *time_buffer;
	char *hostname;
	char *prefix;
	char *errortype;
	char *option;
	xdebug_llist_element *le;
	TSRMLS_FETCH();

	time_buffer = get_current_time();
	hostname    = find_hostname();
	if (!hostname) {
		hostname = estrdup("{unknown}");
	}
	prefix     = xdebug_sprintf("%s %s(%lu) ", time_buffer, hostname, getpid());
	errortype = error_type(type);

	/* start */
	SENDMSG(h->socket, xdebug_sprintf("%sstart: %s\n", prefix, errortype));

	/* header */
	SENDMSG(h->socket, xdebug_sprintf("%smessage: %s\n", prefix, message));
	SENDMSG(h->socket, xdebug_sprintf("%slocation: %s:%d\n", prefix, location, line));
	SENDMSG(h->socket, xdebug_sprintf("%sframes: %d\n", prefix, stack->size));

	/* stack elements */
	if (stack) {
		for (le = XDEBUG_LLIST_HEAD(stack); le != NULL; le = XDEBUG_LLIST_NEXT(le))
		{
			struct function_stack_entry *i = XDEBUG_LLIST_VALP(le);
			char *tmp_name;
				
			tmp_name = show_fname (i TSRMLS_CC);
			SENDMSG(h->socket, xdebug_sprintf("%sfunction: %s\n", prefix, tmp_name));
			xdfree (tmp_name);

			SENDMSG(h->socket, xdebug_sprintf("%slocation: %s:%d\n", prefix, i->filename, i->lineno));
		}
	}

	/* stop */
	SENDMSG(h->socket, xdebug_sprintf("%sstop: %s\n", prefix, errortype));

	xdfree(errortype);
	xdfree(prefix);
	xdfree(hostname);
	do {
		SSEND(h->socket, "?cmd\n");
		option = xdebug_socket_read_line(h->socket, h->buffer);
		printf ("[%s]\n", option);
		SSEND(h->socket, "+OK\n");
	} while (!option || strcmp(option, "cont") != 0);
}

int xdebug_gdb_breakpoint(xdebug_con *context, xdebug_llist *stack)
{
#warning print last stack element
#warning loop for commands until "cont"
#warning 	commands include:
#warning	- printing stack frames
#warning	- chancing parameters
#warning	- and more...
}
