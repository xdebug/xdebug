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

#include "php_xdebug.h"
#include "xdebug_socket.h"

char* xdebug_socket_read_line(int socket, xdebug_socket_buf *context)
{
	int size = 0, newl = 0, nbufsize = 0;
	char *tmp;
	char *tmp_buf = NULL;
	char *ptr;
	char buffer[128];

	if (!context->buffer) {
		context->buffer = xdcalloc(1,1);
		context->buffer_size = 0;
	}

	while ((ptr = memchr(context->buffer, '\n', context->buffer_size)) == NULL) {
		ptr = context->buffer + context->buffer_size;
		newl = read(socket, buffer, 8);
		if (newl > 0) {
			context->buffer = xdrealloc(context->buffer, context->buffer_size + newl + 1);
			memcpy(context->buffer + context->buffer_size, buffer, newl);
			context->buffer_size += newl;
			context->buffer[context->buffer_size] = '\0';
		} else {
			return NULL;
		}
	}

	ptr = memchr(context->buffer, '\n', context->buffer_size);
	size = ptr - context->buffer;
	/* Copy that line into tmp */
	tmp = xdmalloc(size + 1);
	tmp[size] = '\0';
	memcpy(tmp, context->buffer, size);
	/* Rewrite existing buffer */
	if ((nbufsize = context->buffer_size - size - 1)  > 0) {
		tmp_buf = xdmalloc(nbufsize + 1);
		memcpy(tmp_buf, ptr + 1, nbufsize);
		tmp_buf[nbufsize] = 0;
	}
	xdfree(context->buffer);
	context->buffer = tmp_buf;
	context->buffer_size = context->buffer_size - (size + 1);
	/* Return normal line */
	return tmp;
}

