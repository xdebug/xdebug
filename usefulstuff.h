/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 Derick Rethans                              |
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

#ifndef __HAVE_USEFULSTUFF_H__
#define __HAVE_USEFULSTUFF_H__

#define FD_RL_FILE    0
#define FD_RL_SOCKET  1

typedef struct _fd_buf fd_buf;

struct _fd_buf {
	char *buffer;
	int   buffer_size;
};

typedef struct xdebug_arg {
	int    c;
	char **args;
} xdebug_arg;

#define xdebug_arg_init(arg) {    \
	arg->args = NULL;             \
	arg->c    = 0;                \
}

#define xdebug_arg_dtor(arg) {     \
	int i;                         \
	for (i = 0; i < arg->c; i++) { \
		xdfree(arg->args[i]);      \
	}                              \
	if (arg->args) {               \
		xdfree(arg->args);         \
	}                              \
	xdfree(arg);                   \
}

char* fd_read_line(int socket, fd_buf *context, int type);
void xdebug_explode(char *delim, char *str, xdebug_arg *args, int limit);
char* xdebug_memnstr(char *haystack, char *needle, int needle_len, char *end);
char* xdebug_get_time(void);

#endif
