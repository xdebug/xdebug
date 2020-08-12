/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2020 Derick Rethans                               |
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

#ifndef __XDEBUG_LIBRARY_PRIVATE_H__
#define __XDEBUG_LIBRARY_PRIVATE_H__

#define XG_LIB(v)      (XG(globals.library.v))
#define XINI_LIB(v)    (XG(settings.library.v))

void xdebug_open_log(void);
void xdebug_close_log(void);

void XDEBUG_ATTRIBUTE_FORMAT(printf, 3, 4) xdebug_log(int channel, int log_level, const char *fmt, ...);

#endif
