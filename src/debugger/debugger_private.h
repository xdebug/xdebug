/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2019 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <derick@xdebug.org>                          |
   +----------------------------------------------------------------------+
 */

#ifndef __XDEBUG_DEBUGGER_PRIVATE_H__
#define __XDEBUG_DEBUGGER_PRIVATE_H__

#include "debugger.h"

#include "lib/private.h"

typedef struct _fd_buf fd_buf;

struct _fd_buf {
	char *buffer;
	int   buffer_size;
};

#define XG_DBG(v)      (XG(globals.debugger.v))
#define XINI_DBG(v)    (XG(settings.debugger.v))

#endif
