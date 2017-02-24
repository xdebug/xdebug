/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2016 Derick Rethans                               |
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
   |           Thomas Vanhaniemi <thomas.vanhaniemi@arcada.fi>            |
   +----------------------------------------------------------------------+
 */
#include "php_xdebug.h"
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#ifndef PHP_WIN32
# include <sys/poll.h>
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/tcp.h>
# if HAVE_NETINET_IN_H
#  include <netinet/in.h>
# endif
# include <netdb.h>
#else
# include <process.h>
# include <direct.h>
# include "win32/time.h"
# undef UNICODE
# include <winsock2.h>
# include <ws2tcpip.h>
# include <mstcpip.h>
# pragma comment (lib, "Ws2_32.lib")
# define PATH_MAX MAX_PATH
# define poll WSAPoll
#endif

#include "xdebug_private.h"
#include "xdebug_com.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

int xdebug_create_socket(const char *hostname, int dport TSRMLS_DC)
{
	struct addrinfo            hints;
	struct addrinfo            *remote;
	struct addrinfo            *ptr;
	int                        status;
	int                        sockfd;
	int                        sockerror;
	char                       sport[10];
	int                        timeout = 200;
	int                        actually_connected;
	struct sockaddr_in6        sa;
	socklen_t                  size = sizeof(sa);
#if WIN32|WINNT
	WSAPOLLFD                  ufds[1] = {0};
	WORD                       wVersionRequested;
	WSADATA                    wsaData;
	char                       optval = 1;
	u_long                     yes = 1;
	u_long                     no = 0;

	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
#else
	struct pollfd              ufds[1];
	long                       optval = 1;
#endif

	/* Make a string of the port number that can be used with getaddrinfo */
	sprintf(sport, "%d", dport);

	/* Create hints for getaddrinfo saying that we want IPv4 and IPv6 TCP stream sockets */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	/* Call getaddrinfo and return SOCK_ERR if the call fails for some reason */
	if ((status = getaddrinfo(hostname, sport, &hints, &remote)) != 0) {
#if WIN32|WINNT
		XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', getaddrinfo: %d.\n", hostname, dport, WSAGetLastError());
#else
		XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', getaddrinfo: %s.\n", hostname, dport, strerror(errno));
#endif
		return SOCK_ERR;
	}

	/* Go through every returned IP address */
	for (ptr = remote; ptr != NULL; ptr = ptr->ai_next) {
		/* Try to create the socket. If the creation fails continue on with the
		 * next IP address in the list */
		if ((sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == SOCK_ERR) {
#if WIN32|WINNT
			XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', socket: %d.\n", hostname, dport, WSAGetLastError());
#else
			XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', socket: %s.\n", hostname, dport, strerror(errno));
#endif
			continue;
		}

		/* Put socket in non-blocking mode so we can use poll for timeouts */
#ifdef WIN32
		status = ioctlsocket(sockfd, FIONBIO, &yes);
		if (SOCKET_ERROR == status) {
			XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', FIONBIO: %d.\n", hostname, dport, WSAGetLastError());
		}
#else
		fcntl(sockfd, F_SETFL, O_NONBLOCK);
#endif

		/* Try to connect to the newly created socket */
		/* Worth noting is that the port is set in the getaddrinfo call before */
		status = connect(sockfd, ptr->ai_addr, ptr->ai_addrlen);

		/* Determine if we got a connection. If no connection could be made
		 * we close the socket and continue with the next IP address in the list */
		if (status < 0) {
#ifdef WIN32
			errno = WSAGetLastError();
			if (errno != WSAEINPROGRESS && errno != WSAEWOULDBLOCK) {
				XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', connect: %d.\n", hostname, dport, errno);
#else
			if (errno == EACCES) {
				XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', connect: %s.\n", hostname, dport, strerror(errno));
				SCLOSE(sockfd);
				sockfd = SOCK_ACCESS_ERR;

				continue;
			}
			if (errno != EINPROGRESS) {
				XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', connect: %s.\n", hostname, dport, strerror(errno));
#endif
				SCLOSE(sockfd);
				sockfd = SOCK_ERR;

				continue;
			}

			ufds[0].fd = sockfd;
#if WIN32|WINNT
			ufds[0].events = POLLIN | POLLOUT;
#else
			ufds[0].events = POLLIN | POLLOUT | POLLPRI;
#endif
			while (1) {
				sockerror = poll(ufds, 1, timeout);

#if WIN32|WINNT
				errno = WSAGetLastError();
				if (errno == WSAEINPROGRESS || errno == WSAEWOULDBLOCK) {
					/* XXX introduce retry count? */
					continue;
				}
#endif
				/* If an error occured when doing the poll */
				if (sockerror == SOCK_ERR) {
#if WIN32|WINNT
					XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', WSAPoll error: %d (%d, %d).\n", hostname, dport, WSAGetLastError(), sockerror, errno);
#else
					XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', poll error: %s (%d).\n", hostname, dport, strerror(errno), sockerror);
#endif
					sockerror = SOCK_ERR;
					break;
				}

				/* A timeout occured when polling the socket */
				if (sockerror == 0) {
					sockerror = SOCK_TIMEOUT_ERR;
					break;
				}

				/* If the poll was successful but an error occured */
				if (ufds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) {
#if WIN32|WINNT
					XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', WSAPoll success, but error: %d (%d).\n", hostname, dport, WSAGetLastError(), ufds[0].revents);
#else
					XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', poll success, but error: %s (%d).\n", hostname, dport, strerror(errno), ufds[0].revents);
#endif
					sockerror = SOCK_ERR;
					break;
				}

				/* If the poll was successful break out */
				if (ufds[0].revents & (POLLIN | POLLOUT)) {
					sockerror = sockfd;
					break;
				} else {
					/* We should never get here, but added as a failsafe to break out from any loops */
#if WIN32|WINNT
					XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', WSAPoll: %d.\n", hostname, dport, WSAGetLastError());
#else
					XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', poll: %s.\n", hostname, dport, strerror(errno));
#endif
					sockerror = SOCK_ERR;
					break;
				}
			}

			if (sockerror > 0) {
				actually_connected = getpeername(sockfd, (struct sockaddr *)&sa, &size);
				if (actually_connected == -1) {
#if WIN32|WINNT
					XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', getpeername: %d.\n", hostname, dport, WSAGetLastError());
#else
					XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', getpeername: %s.\n", hostname, dport, strerror(errno));
#endif
					sockerror = SOCK_ERR;
				}
			}

			/* If there where some errors close the socket and continue with the next IP address */
			if (sockerror < 0) {
				SCLOSE(sockfd);
				sockfd = sockerror;

				continue;
			}
		}

		break;
	}

	/* Free the result returned by getaddrinfo */
	freeaddrinfo(remote);

	/* If we got a socket, set the option "No delay" to true (1) */
	if (sockfd > 0) {
#ifdef WIN32
		status = ioctlsocket(sockfd, FIONBIO, &no);
		if (SOCKET_ERROR == status) {
			XDEBUG_LOG_PRINT(XG(remote_log_file), "W: Creating socket for '%s:%d', FIONBIO: %d.\n", hostname, dport, WSAGetLastError());
		}
#else
		fcntl(sockfd, F_SETFL, 0);
#endif

		setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
	}

	return sockfd;
}

void xdebug_close_socket(int socketfd)
{
	SCLOSE(socketfd);
}
