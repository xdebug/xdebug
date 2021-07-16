/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2021 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
 */
#include "php_xdebug.h"
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#ifndef PHP_WIN32
# if HAVE_POLL_H
#  include <poll.h>
# elif HAVE_SYS_POLL_H
#  include <sys/poll.h>
# endif
# include <unistd.h>
# include <sys/socket.h>
# include <sys/un.h>
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

#include "com.h"

#include "debugger_private.h"
#include "handler_dbgp.h"
#include "lib/crc32.h"
#include "lib/log.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

#if !WIN32 && !WINNT
static int xdebug_create_socket_unix(const char *path)
{
	struct sockaddr_un sa;
	int sockfd;

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == SOCK_ERR) {
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "UNIX", "Creating socket for 'unix://%s', socket: %s.", path, strerror(errno));
		return SOCK_ERR;
	}

	sa.sun_family = AF_UNIX;
	strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
	if (connect(sockfd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "UNIX", "Creating socket for 'unix://%s', connect: %s.", path, strerror(errno));
		SCLOSE(sockfd);
		return (errno == EACCES) ? SOCK_ACCESS_ERR : SOCK_ERR;
	}

	/* Prevent the socket from being inherited by exec'd children */
	if (fcntl(sockfd, F_SETFD, FD_CLOEXEC) < 0) {
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "UNIX", "Creating socket for 'unix://%s', fcntl(FD_CLOEXEC): %s.", path, strerror(errno));
	}

	return sockfd;
}
#endif

#if !WIN32 && !WINNT

/* For OSX and FreeBSD */
# if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#  define SOL_TCP IPPROTO_TCP
# endif
# if !defined(TCP_KEEPIDLE) && defined(TCP_KEEPALIVE)
#  define TCP_KEEPIDLE TCP_KEEPALIVE
# endif

void set_keepalive_options(int fd)
{
	int optval = 1;
	int optlen = sizeof(optval);
	int ret;

	ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
	if (ret) {
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "KEEPALIVE", "Could not set SO_KEEPALIVE: %s.", strerror(errno));
		return;
	}

# if defined(TCP_KEEPIDLE)
	optval = 600;
	ret = setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &optval, optlen);
	if (ret) {
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "KEEPALIVE", "Could not set TCP_KEEPIDLE to %d: %s.", optval, strerror(errno));
		return;
	}
# endif

# if defined(TCP_KEEPCNT)
	optval = 20;
	ret = setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &optval, optlen);
	if (ret) {
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "KEEPALIVE", "Could not set TCP_KEEPCNT to %d: %s.", optval, strerror(errno));
		return;
	}
# endif

# if defined(TCP_KEEPINTVL)
	optval = 60;
	ret = setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &optval, optlen);
	if (ret) {
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "KEEPALIVE", "Could not set TCP_KEEPINTVL to %d: %s.", optval, strerror(errno));
		return;
	}
# endif
}
#endif  // !WIN32 && !WINNT

static int xdebug_create_socket(const char *hostname, int dport, int timeout)
{
	struct addrinfo            hints;
	struct addrinfo            *remote;
	struct addrinfo            *ptr;
	int                        status;
	int                        sockfd = 0;
	int                        sockerror;
	char                       sport[10];
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

	if (!strncmp(hostname, "unix://", strlen("unix://"))) {
#if WIN32|WINNT
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "UNIX-WIN", "Creating Unix domain socket ('%s') on Windows is not supported.", hostname);
		return SOCK_ERR;
#else
		return xdebug_create_socket_unix(hostname + strlen("unix://"));
#endif
	}

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
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK1", "Creating socket for '%s:%d', getaddrinfo: %d.", hostname, dport, WSAGetLastError());
#else
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK1", "Creating socket for '%s:%d', getaddrinfo: %s.", hostname, dport, strerror(errno));
#endif
		return SOCK_ERR;
	}

	/* Go through every returned IP address */
	for (ptr = remote; ptr != NULL; ptr = ptr->ai_next) {
		/* Try to create the socket. If the creation fails continue on with the
		 * next IP address in the list */
		if ((sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == SOCK_ERR) {
#if WIN32|WINNT
			xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK2", "Creating socket for '%s:%d', socket: %d.", hostname, dport, WSAGetLastError());
#else
			xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK2", "Creating socket for '%s:%d', socket: %s.", hostname, dport, strerror(errno));
#endif
			continue;
		}

		/* Put socket in non-blocking mode so we can use poll for timeouts */
#ifdef WIN32
		status = ioctlsocket(sockfd, FIONBIO, &yes);
		if (SOCKET_ERROR == status) {
			xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK2", "Creating socket for '%s:%d', FIONBIO: %d.", hostname, dport, WSAGetLastError());
		}
#else
		fcntl(sockfd, F_SETFL, O_NONBLOCK);
#endif

#if !WIN32 && !WINNT
		/* Prevent the socket from being inherited by exec'd children on *nix (not necessary on Win) */
		if (fcntl(sockfd, F_SETFD, FD_CLOEXEC) < 0) {
			xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK2", "Creating socket for '%s:%d', fcntl(FD_CLOEXEC): %s.", hostname, dport, strerror(errno));
		}
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
				xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK3", "Creating socket for '%s:%d', connect: %d.", hostname, dport, errno);
#else
			if (errno == EACCES) {
				xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK3", "Creating socket for '%s:%d', connect: %s.", hostname, dport, strerror(errno));
				SCLOSE(sockfd);
				sockfd = SOCK_ACCESS_ERR;

				continue;
			}
			if (errno != EINPROGRESS) {
				xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK3", "Creating socket for '%s:%d', connect: %s.", hostname, dport, strerror(errno));
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
					xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK4", "Creating socket for '%s:%d', WSAPoll error: %d (%d, %d).", hostname, dport, WSAGetLastError(), sockerror, errno);
#else
					xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK4", "Creating socket for '%s:%d', poll error: %s (%d).", hostname, dport, strerror(errno), sockerror);
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
					xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK4", "Creating socket for '%s:%d', WSAPoll success, but error: %d (%d).", hostname, dport, WSAGetLastError(), ufds[0].revents);
#else
					xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK4", "Creating socket for '%s:%d', poll success, but error: %s (%d).", hostname, dport, strerror(errno), ufds[0].revents);
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
					xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK4", "Creating socket for '%s:%d', WSAPoll: %d.", hostname, dport, WSAGetLastError());
#else
					xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK4", "Creating socket for '%s:%d', poll: %s.", hostname, dport, strerror(errno));
#endif
					sockerror = SOCK_ERR;
					break;
				}
			}

			if (sockerror > 0) {
				actually_connected = getpeername(sockfd, (struct sockaddr *)&sa, &size);
				if (actually_connected == -1) {
#if WIN32|WINNT
					xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK5", "Creating socket for '%s:%d', getpeername: %d.", hostname, dport, WSAGetLastError());
#else
					xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK5", "Creating socket for '%s:%d', getpeername: %s.", hostname, dport, strerror(errno));
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
			xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "SOCK6", "Creating socket for '%s:%d', FIONBIO: %d.", hostname, dport, WSAGetLastError());
		}
#else
		fcntl(sockfd, F_SETFL, 0);
#endif

		setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
#if !WIN32 && !WINNT
		set_keepalive_options(sockfd);
#endif

		/* Now we have a socket, update the last seen hostname and port */
		if (XG_DBG(context).connected_hostname) {
			xdfree(XG_DBG(context).connected_hostname);
		}
		XG_DBG(context).connected_hostname = xdstrdup(hostname);
		XG_DBG(context).connected_port = dport;
	}

	return sockfd;
}

void xdebug_close_socket(int socketfd)
{
	SCLOSE(socketfd);
}

/* Starting the debugger */
static void xdebug_init_normal_debugger(xdebug_str *connection_attempts)
{
	zval *remote_addr = NULL;
	char *cp = NULL;
	int   cp_found = 0;
	const char *header = NULL;

	if (!XINI_DBG(discover_client_host)) {
		xdebug_str_add_fmt(connection_attempts, "%s:%ld (through xdebug.client_host/xdebug.client_port)", XINI_DBG(client_host), XINI_DBG(client_port));
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_INFO, "Connecting to configured address/port: %s:%ld.", XINI_DBG(client_host), (long int) XINI_DBG(client_port));

		XG_DBG(context).socket = xdebug_create_socket(XINI_DBG(client_host), XINI_DBG(client_port), XINI_DBG(connect_timeout_ms));
		return;
	}

	xdebug_log(XLOG_CHAN_DEBUG, XLOG_INFO, "Checking remote connect back address.");
	if (XINI_DBG(client_discovery_header) && XINI_DBG(client_discovery_header)[0]) {
		header = XINI_DBG(client_discovery_header);
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_INFO, "Checking user configured header '%s'.", XINI_DBG(client_discovery_header));

		remote_addr = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), XINI_DBG(client_discovery_header), HASH_KEY_STRLEN(XINI_DBG(client_discovery_header)));
	}
	if (!remote_addr) {
		header = "HTTP_X_FORWARDED_FOR";
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_INFO, "Checking header 'HTTP_X_FORWARDED_FOR'.");

		remote_addr = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_X_FORWARDED_FOR", HASH_KEY_SIZEOF("HTTP_X_FORWARDED_FOR"));
	}
	if (!remote_addr) {
		header = "REMOTE_ADDR";
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_INFO, "Checking header 'REMOTE_ADDR'.");

		remote_addr = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), "REMOTE_ADDR", HASH_KEY_SIZEOF("REMOTE_ADDR"));
	}

	if (remote_addr && strstr(Z_STRVAL_P(remote_addr), "://")) {
		header = NULL;
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "INVADDR", "Invalid remote address provided containing URI spec '%s'.", Z_STRVAL_P(remote_addr));

		remote_addr = NULL;
	}

	if (!remote_addr) {
		xdebug_str_add_fmt(connection_attempts, "%s:%ld (fallback through xdebug.client_host/xdebug.client_port)", XINI_DBG(client_host), XINI_DBG(client_port));
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "HDR", "Could not discover client host through HTTP headers, connecting to configured address/port: %s:%ld. :-|", XINI_DBG(client_host), (long int) XINI_DBG(client_port));

		XG_DBG(context).socket = xdebug_create_socket(XINI_DBG(client_host), XINI_DBG(client_port), XINI_DBG(connect_timeout_ms));
		return;
	}

	/* Use first IP according to RFC 7239 */
	cp = strchr(Z_STRVAL_P(remote_addr), ',');
	if (cp) {
		*cp = '\0';
		cp_found = 1;
	}

	xdebug_str_add_fmt(connection_attempts, "%s:%ld (from %s HTTP header)", Z_STRVAL_P(remote_addr), XINI_DBG(client_port), header);
	xdebug_log(XLOG_CHAN_DEBUG, XLOG_INFO, "Client host discovered through HTTP header, connecting to %s:%ld.", Z_STRVAL_P(remote_addr), (long int) XINI_DBG(client_port));

	XG_DBG(context).socket = xdebug_create_socket(Z_STRVAL_P(remote_addr), XINI_DBG(client_port), XINI_DBG(connect_timeout_ms));

	if (XG_DBG(context).socket < 0) {
		xdebug_str_add_fmt(connection_attempts, ", %s:%ld (fallback through xdebug.client_host/xdebug.client_port)", XINI_DBG(client_host), XINI_DBG(client_port));
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_WARN, "CON", "Could not connect to client host discovered through HTTP headers, connecting to configured address/port: %s:%ld. :-|", XINI_DBG(client_host), (long int) XINI_DBG(client_port));

		XG_DBG(context).socket = xdebug_create_socket(XINI_DBG(client_host), XINI_DBG(client_port), XINI_DBG(connect_timeout_ms));
	}

	/* Replace the ',', in case we had changed the original header due
	 * to multiple values */
	if (cp_found) {
		*cp = ',';
	}
}

static void xdebug_init_cloud_debugger(const char *cloud_id)
{
	unsigned long  crc = xdebug_crc32(cloud_id, strlen(cloud_id));
	char          *host;

	host = xdebug_sprintf("%c.cloud.xdebug.com", (crc & 0x0f) + 'a');

	xdebug_log(XLOG_CHAN_DEBUG, XLOG_INFO, "Connecting to configured address/port: %s:%ld.", host, 9020L);
	XG_DBG(context).socket = xdebug_create_socket(host, 9020, XINI_DBG(connect_timeout_ms));

	xdfree(host);
}

/**
 * dXXXXXXa-cXXa-4XX7-9XX3-fXXXXXXXXXX0
 */
static int ide_key_is_cloud_id()
{
	const char *k = XG_DBG(ide_key);

	if (strlen(k) != 36) {
		return 0;
	}

	if (k[8] != '-' || k[13] != '-' || k[18] != '-' || k[23] != '-') {
		return 0;
	}

	return 1;
}

static void xdebug_init_debugger()
{
	xdebug_str *connection_attempts = xdebug_str_new();

	/* Get handler from mode */
	XG_DBG(context).handler = &xdebug_handler_dbgp;

	if (strcmp(XINI_DBG(cloud_id), "") != 0) {
		xdebug_init_cloud_debugger(XINI_DBG(cloud_id));
		XG_DBG(context).host_type = XDEBUG_CLOUD;
	} else if (XG_DBG(ide_key) && ide_key_is_cloud_id()) {
		xdebug_init_cloud_debugger(XG_DBG(ide_key));
		XG_DBG(context).host_type = XDEBUG_CLOUD_FROM_TRIGGER_VALUE;
	} else {
		xdebug_init_normal_debugger(connection_attempts);
		XG_DBG(context).host_type = XDEBUG_NORMAL;
	}

	/* Check whether we're connected, or why not */
	if (XG_DBG(context).socket >= 0) {
		xdebug_log(XLOG_CHAN_DEBUG, XLOG_INFO, "Connected to debugging client: %s. :-)", connection_attempts->d);
		xdebug_mark_debug_connection_pending();

		if (!XG_DBG(context).handler->remote_init(&(XG_DBG(context)), XDEBUG_REQ)) {
			/* The request could not be started, ignore it then */
			xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_ERR, "SES-INIT", "The debug session could not be started. Tried: %s. :-(", connection_attempts->d);
		} else {
			/* All is well, turn off script time outs */
			zend_unset_timeout();
			EG(timeout_seconds) = 0;
			zend_set_timeout(EG(timeout_seconds), 0);
		}
	} else if (XG_DBG(context).socket == -1) {
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_ERR, "NOCON", "Could not connect to debugging client. Tried: %s :-(", connection_attempts->d);
	} else if (XG_DBG(context).socket == -2) {
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_ERR, "TIMEOUT", "Time-out connecting to debugging client, waited: " ZEND_LONG_FMT " ms. Tried: %s :-(", XINI_DBG(connect_timeout_ms), connection_attempts->d);
	} else if (XG_DBG(context).socket == -3) {
		xdebug_log_ex(XLOG_CHAN_DEBUG, XLOG_ERR, "NOPERM", "No permission connecting to debugging client (%s). This could be SELinux related. :-(", connection_attempts->d);
	}

	xdebug_str_free(connection_attempts);
}

void xdebug_abort_debugger()
{
	if (XG_DBG(remote_connection_enabled)) {
		xdebug_mark_debug_connection_not_active();
	}
}

void xdebug_restart_debugger()
{
	xdebug_abort_debugger();
	xdebug_init_debugger();
}

/* Remote connection activation and house keeping */
int xdebug_is_debug_connection_active()
{
	return XG_DBG(remote_connection_enabled);
}

void xdebug_mark_debug_connection_active()
{
	XG_DBG(remote_connection_enabled) = 1;
	XG_DBG(remote_connection_pid) = xdebug_get_pid();
}

void xdebug_mark_debug_connection_pending()
{
	XG_DBG(remote_connection_enabled) = 0;
	XG_DBG(remote_connection_pid) = 0;
}

void xdebug_mark_debug_connection_not_active()
{
	if (XG_DBG(remote_connection_enabled)) {
		xdebug_close_socket(XG_DBG(context).socket);
	}

	XG_DBG(remote_connection_enabled) = 0;
	XG_DBG(remote_connection_pid) = 0;
}

void xdebug_debug_init_if_requested_on_error()
{
	RETURN_IF_MODE_IS_NOT(XDEBUG_MODE_STEP_DEBUG);

	if (!xdebug_lib_start_upon_error()) {
		return;
	}

	if (!xdebug_is_debug_connection_active()) {
		xdebug_init_debugger();
	}
}

void xdebug_debug_init_if_requested_on_xdebug_break()
{
	RETURN_IF_MODE_IS_NOT(XDEBUG_MODE_STEP_DEBUG);

	if (xdebug_is_debug_connection_active()) {
		return;
	}

	if (xdebug_lib_start_if_mode_is_trigger(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_init_debugger();
	}
}

static void xdebug_update_ide_key(char *new_key)
{
	if (XG_DBG(ide_key)) {
		xdfree(XG_DBG(ide_key));
	}
	XG_DBG(ide_key) = xdstrdup(new_key);
}

static int xdebug_handle_start_session()
{
	int   activate_session = 0;
	zval *dummy;
	char *dummy_env;

	/* Set session cookie if requested */
	if (
		((
			(dummy = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_ENV]), "XDEBUG_SESSION_START", sizeof("XDEBUG_SESSION_START") - 1)) != NULL
		) || (
			(dummy = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_GET]), "XDEBUG_SESSION_START", sizeof("XDEBUG_SESSION_START") - 1)) != NULL
		) || (
			(dummy = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_POST]), "XDEBUG_SESSION_START", sizeof("XDEBUG_SESSION_START") - 1)) != NULL
		))
		&& !SG(headers_sent)
	) {
		convert_to_string_ex(dummy);
		xdebug_update_ide_key(Z_STRVAL_P(dummy));

		xdebug_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION") - 1, Z_STRVAL_P(dummy), Z_STRLEN_P(dummy), 0, "/", 1, NULL, 0, 0, 1, 0);
		activate_session = 1;
	} else if (
		(dummy_env = getenv("XDEBUG_SESSION_START")) != NULL
	) {
		xdebug_update_ide_key(dummy_env);

		if (!SG(headers_sent)) {
			xdebug_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION") - 1, XG_DBG(ide_key), strlen(XG_DBG(ide_key)), 0, "/", 1, NULL, 0, 0, 1, 0);
		}

		activate_session = 1;
	} else if (
		(dummy = zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_COOKIE]), "XDEBUG_SESSION", sizeof("XDEBUG_SESSION") - 1)) != NULL
	) {
		convert_to_string_ex(dummy);
		xdebug_update_ide_key(Z_STRVAL_P(dummy));

		activate_session = 1;
	} else if (getenv("XDEBUG_CONFIG")) {
		if (XG_DBG(ide_key) && *XG_DBG(ide_key) && !SG(headers_sent)) {
			xdebug_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION") - 1, XG_DBG(ide_key), strlen(XG_DBG(ide_key)), 0, "/", 1, NULL, 0, 0, 1, 0);
			activate_session = 1;
		}
	}

	return activate_session;
}

static void xdebug_handle_stop_session()
{
	/* Remove session cookie if requested */
	if (
		((
			zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_GET]), "XDEBUG_SESSION_STOP", sizeof("XDEBUG_SESSION_STOP") - 1) != NULL
		) || (
			zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_POST]), "XDEBUG_SESSION_STOP", sizeof("XDEBUG_SESSION_STOP") - 1) != NULL
		))
		&& !SG(headers_sent)
	) {
		xdebug_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION") - 1, (char*) "", 0, 0, "/", 1, NULL, 0, 0, 1, 0);
	}
}

void xdebug_debug_init_if_requested_at_startup(void)
{
	char *found_trigger_value = NULL;

	if (XG_DBG(detached)) {
		return;
	}

	if (xdebug_is_debug_connection_active()) {
		return;
	}

	if (
		xdebug_lib_start_with_request(XDEBUG_MODE_STEP_DEBUG) ||
		(!xdebug_lib_never_start_with_request() && xdebug_handle_start_session()) ||
		xdebug_lib_start_with_trigger(XDEBUG_MODE_STEP_DEBUG, &found_trigger_value)
	) {
		if (found_trigger_value) {
			xdebug_update_ide_key(found_trigger_value);
		}
		xdebug_init_debugger();
	}

	if (found_trigger_value) {
		xdfree(found_trigger_value);
	}

	xdebug_handle_stop_session();
}
