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

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "usefulstuff.h"

#define VERSION "0.5.0"

int main(int argc, char *argv[])
{
	int port = 7869;
	int ssocket = socket (AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_in;
	int				client_in_len;
	int				fd;
	struct sockaddr_in client_in;
	struct timeval	 timeout;
	struct in_addr	*iaddr;
	char *buffer;
	ssize_t l;
	char *cmd;
	fd_buf cxt = { NULL, 0 };
	fd_buf std_in = { NULL, 0 };

	if (ssocket < 0) {
		printf ("setup_socket: couldn't create socket\n");
	}

	memset (&server_in, 0, sizeof(struct sockaddr));
	server_in.sin_family	  = AF_INET;
	server_in.sin_addr.s_addr = htonl(INADDR_ANY);
	server_in.sin_port		= htons((int) port);

	while (bind (ssocket, (struct sockaddr *) &server_in, sizeof(struct sockaddr_in)) < 0) {
		printf ("setup_socket: couldn't bind AF_INET socket?\n");
		sleep(5);
	}
	printf ("Xdebug GDB emulation client (%s)\n", VERSION);
	printf ("Copyright 2002 by Derick Rethans, JDI Media Solutions.\n");

	while (1) {
		if (listen (ssocket, 5) == -1) {
			printf ("setup_socket: listen call failed\n");
		}
		printf ("\nWaiting for debug server to connect.\n");
		fd = accept (ssocket, (struct sockaddr *) &client_in, &client_in_len);
		iaddr = &client_in.sin_addr;
		printf ("Connect\n");
		while ((buffer = fd_read_line (fd, &cxt)) > 0) {

			if (buffer[0] == '?') {
				printf ("(%s) ", &buffer[1]);
				fflush(stdout);
				if (cmd = fd_read_line (0, &std_in)) {
					if (send (fd, cmd, strlen(cmd), MSG_NOSIGNAL) == -1) {
						break;
					}
					if (send (fd, "\n", 1, MSG_NOSIGNAL) == -1) {
						break;
					}
					if (strcmp (cmd, "quit") == 0) {
						break;
					}
				}
			} else if (buffer[0] == '-') {
				printf ("%s\n", &buffer[8]);
			} else if (buffer[0] != '+') {
				printf ("%s\n", buffer);
			}
		}
		printf ("Disconnect\n\n");
		close(fd);
	}
}
