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
   | Authors:  Derick Rethans <derick@vl-srm.net>                         |
   +----------------------------------------------------------------------+
 */

#ifndef __HAVE_XDEBUG_COM_H__
#define __HAVE_XDEBUG_COM_H__

#ifdef PHP_WIN32
int inet_aton(const char *cp, struct in_addr *inp);
#endif

#if WIN32|WINNT
# define SOCK_ERR INVALID_SOCKET
# define SOCK_CONN_ERR SOCKET_ERROR
# define SOCK_RECV_ERR SOCKET_ERROR
#else
# define SOCK_ERR -1
# define SOCK_CONN_ERR -1
# define SOCK_RECV_ERR -1
#endif

#if WIN32|WINNT
#define SCLOSE(a) closesocket(a)
#define SSENDL(a,b,c) send(a,b,c,0)
#define SSEND(a,b) send(a,b,strlen(b),0)
#else
#define SCLOSE(a) close(a)
#define SSENDL(a,b,c) write(a,b,c)
#define SSEND(a,b) write(a,b,strlen(b))
#endif

#define SREAD(a,b,c) read(a,b,c)

int xdebug_create_socket(const char *hostname, int dport);
void xdebug_close_socket(int socket);

#endif
