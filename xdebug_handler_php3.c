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
#include "TSRM.h"
#include "php_globals.h"
#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_com.h"
#include "xdebug_handler_php3.h"
#include "xdebug_llist.h"
#include "xdebug_mm.h"
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
	snprintf(microbuf, sizeof(microbuf) - 1, ":%06lu", (long) tv.tv_usec);
	strcat(debug_timebuf, microbuf);
#endif
	return debug_timebuf;
}

char *xdebug_php3_get_revision(void)
{
	return "$Revision: 1.22 $";
}

int xdebug_php3_init(xdebug_con *context, int mode)
{
	return 1;
}

int xdebug_php3_deinit(xdebug_con *context)
{
	return 1;
}

#define SENDMSG(socket, str) {  \
	char *message_buffer;       \
                                \
	message_buffer = str;       \
	SSEND(socket, message_buffer); \
	xdfree(message_buffer);     \
}

int xdebug_php3_error(xdebug_con *h, int type, char *exception_type, char *message, const char *location, const uint line, xdebug_llist *stack)
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
	if (exception_type) {
		errortype = exception_type;
	} else {
		errortype = xdebug_error_type(type);
	}

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
			function_stack_entry *i = XDEBUG_LLIST_VALP(le);
			char *tmp_name;
				
			tmp_name = xdebug_show_fname(i->function, 0, 0 TSRMLS_CC);
			SENDMSG(h->socket, xdebug_sprintf("%sfunction: %s\n", prefix, tmp_name));
			xdfree(tmp_name);

			SENDMSG(h->socket, xdebug_sprintf("%slocation: %s:%d\n", prefix, i->filename, i->lineno));
		}
	}

	/* stop */
	SENDMSG(h->socket, xdebug_sprintf("%sstop: %s\n", prefix, errortype));

	if (!exception_type) {
		xdfree(errortype);
	}
	xdfree(prefix);
	xdfree(hostname);

	return 1;
}
