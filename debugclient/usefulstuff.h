/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 The PHP Group                               |
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

#ifndef __HAVE_USEFULSTUFF_H__
#define __HAVE_USEFULSTUFF_H__

#define FD_RL_FILE    0
#define FD_RL_SOCKET  1

typedef struct _fd_buf fd_buf;

struct _fd_buf {
	char *buffer;
	int   buffer_size;
};

char* fd_read_line(int socket, fd_buf *context, int type);

#endif
