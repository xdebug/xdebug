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


/* define DEBUGGER_FAIL_SILENTLY */

#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#if MSVC5
# include <winsock.h>
# include <process.h>
# include <direct.h>
# include "win32/time.h"
#define PATH_MAX MAX_PATH
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
#endif

#include "xdebug_com.h"

/*
 * Converts a host name to an IP address.  */
static int lookup_hostname (const char *addr, struct in_addr *in)
{
	struct hostent *host_info;

	if (!inet_aton(addr, in)) {
		host_info = gethostbyname(addr);
		if (host_info == 0) {
			/* Error: unknown host */
			return -1;
		}
		*in = *((struct in_addr *) host_info->h_addr);
	}
	return 0;
}
/* }}} */

int xdebug_create_socket(const char *hostname, int dport)
{
	struct sockaddr_in address;
	int                err = -1;
	int                sockfd;

	memset(&address, 0, sizeof(address));
	lookup_hostname(hostname, &address.sin_addr);
	address.sin_family = AF_INET;
	address.sin_port = htons((unsigned short)dport);

	sockfd = socket(address.sin_family, SOCK_STREAM, 0);
	if (sockfd == SOCK_ERR) {
#ifndef DEBUGGER_FAIL_SILENTLY
#if WIN32|WINNT
		printf("create_debugger_socket(\"%s\", %d) socket: %d\n",
					hostname, dport, WSAGetLastError());
#else
		printf("create_debugger_socket(\"%s\", %d) socket: %s\n",
					hostname, dport, strerror(errno));
#endif
#endif
		return -1;
	}
	while ((err = connect(sockfd, (struct sockaddr *) &address,
						  sizeof(address))) == SOCK_ERR && errno == EAGAIN);

	if (err < 0) {
#ifndef DEBUGGER_FAIL_SILENTLY
#if WIN32|WINNT
		printf("create_debugger_socket(\"%s\", %d) connect: %d\n",
				   hostname, dport, WSAGetLastError());
#else
		printf("create_debugger_socket(\"%s\", %d) connect: %s\n",
				   hostname, dport, strerror(errno));
#endif
#endif
		SCLOSE(sockfd);
		return -1;
	}
	return sockfd;
}

void xdebug_close_socket(int socket)
{
	SCLOSE(socket);
}
