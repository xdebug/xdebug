/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
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

#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#include <io.h>
#endif
#include "usefulstuff.h"

#define READ_BUFFER_SIZE 128

char* fd_read_line_delim(int socket, fd_buf *context, int type, unsigned char delim, int *length)
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

	while ((ptr = memchr(context->buffer, delim, context->buffer_size)) == NULL) {
		ptr = context->buffer + context->buffer_size;
		if (type == FD_RL_FILE) {
			newl = read(socket, buffer, READ_BUFFER_SIZE);
		} else {
			newl = recv(socket, buffer, READ_BUFFER_SIZE, 0);
		}
		if (newl > 0) {
			context->buffer = realloc(context->buffer, context->buffer_size + newl + 1);
			memcpy(context->buffer + context->buffer_size, buffer, newl);
			context->buffer_size += newl;
			context->buffer[context->buffer_size] = '\0';
		} else {
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

