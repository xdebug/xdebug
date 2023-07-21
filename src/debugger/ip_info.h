/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2022 Derick Rethans                               |
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

#ifdef __linux__

#include "php_xdebug.h"

char *xdebug_get_ip_for_interface(const char *iface);

#if HAVE_LINUX_RTNETLINK_H
# define XDEBUG_GATEWAY_SUPPORT 1
char *xdebug_get_gateway_ip(void);
#else
# define XDEBUG_GATEWAY_SUPPORT 0
#endif

#if HAVE_RES_NINIT && HAVE_RES_NCLOSE
# define XDEBUG_NAMESERVER_SUPPORT 1
char *xdebug_get_private_nameserver(void);
#else
# define XDEBUG_NAMESERVER_SUPPORT 0
#endif

#endif  /* __linux__ */
