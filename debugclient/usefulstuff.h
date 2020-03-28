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

#ifndef __HAVE_USEFULSTUFF_H__
#define __HAVE_USEFULSTUFF_H__

#define FD_RL_FILE    0
#define FD_RL_SOCKET  1

typedef struct _fd_buf fd_buf;

struct _fd_buf {
	char *buffer;
	int   buffer_size;
};

#define fd_read_line(s,c,t) fd_read_line_delim(s, c, t, '\n', NULL)
char* fd_read_line_delim(int socket, fd_buf *context, int type, unsigned char delim, int *length);

#endif
