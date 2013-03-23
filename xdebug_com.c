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


#define DEBUGGER_FAIL_SILENTLY

#include <string.h>
#include <sys/types.h>
#include <errno.h>
#ifndef PHP_WIN32
#include <unistd.h>
#endif

#ifdef PHP_WIN32
# include <process.h>
# include <direct.h>
# include "win32/time.h"
#define PATH_MAX MAX_PATH
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <netdb.h>
#endif

#include "xdebug_com.h"

#ifdef PHP_WIN32
int inet_aton(const char *cp, struct in_addr *inp)
{
	inp->s_addr = inet_addr(cp);
	if (inp->s_addr == INADDR_NONE) {
		return 0;
	}
	return 1;
}
#endif

/*
 * Converts a host name to an IP address.  */
static int lookup_hostname(const char *addr, struct in_addr *in)
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
#if WIN32|WINNT
	WORD               wVersionRequested;
	WSADATA            wsaData;
	char               optval = 1;

	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
#else
	long               optval = 1;
#endif
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
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
	return sockfd;
}

void xdebug_close_socket(int socket)
{
	SCLOSE(socket);
}
