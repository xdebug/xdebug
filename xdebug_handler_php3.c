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
#include "xdebug_handler_php3.h"
#include "xdebug_var.h"

#ifdef PHP_WIN32
#include "win32/time.h"
#include <process.h>
#endif


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

int xdebug_php3_init(xdebug_con context, int mode)
{
}

int xdebug_php3_deinit(xdebug_con context)
{
}

#define SENDMSG(socket, str) {  \
	char *message_buffer;       \
                                \
	message_buffer = str;       \
	SSEND(socket, message_buffer); \
	xdfree(message_buffer);     \
}

int xdebug_php3_error(xdebug_con h, int type, char *message, const char *location, const uint line, xdebug_llist *stack)
{
	char *time_buffer;
	char *hostname;
	char *prefix;
	char *errortype;
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
	SENDMSG(h.socket, xdebug_sprintf("%sstart: %s\n", prefix, errortype));

	/* header */
	SENDMSG(h.socket, xdebug_sprintf("%smessage: %s\n", prefix, message));
	SENDMSG(h.socket, xdebug_sprintf("%slocation: %s:%d\n", prefix, location, line));
	SENDMSG(h.socket, xdebug_sprintf("%sframes: %d\n", prefix, stack->size));

	/* stack elements */
	if (stack) {
		for (le = XDEBUG_LLIST_HEAD(stack); le != NULL; le = XDEBUG_LLIST_NEXT(le))
		{
			struct function_stack_entry *i = XDEBUG_LLIST_VALP(le);
			char *tmp_name;
				
			tmp_name = show_fname (i TSRMLS_CC);
			SENDMSG(h.socket, xdebug_sprintf("%sfunction: %s\n", prefix, tmp_name));
			xdfree (tmp_name);

			SENDMSG(h.socket, xdebug_sprintf("%slocation: %s:%d\n", prefix, i->filename, i->lineno));
		}
	}

	/* stop */
	SENDMSG(h.socket, xdebug_sprintf("%sstop: %s\n", prefix, errortype));

	xdfree(errortype);
	xdfree(prefix);
	xdfree(hostname);
}
