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


#include "xdebug_com.h"
#include "php_xdebug.h"
#include "xdebug_handlers.h"
#include "xdebug_handler_php3.h"
#include "xdebug_handler_gdb.h"

xdebug_remote_handler_info handlers[] = {
	{ "php3", xdebug_handler_php3 },
	{ "gdb",  xdebug_handler_gdb },
	{ 0, 0 }
};

xdebug_remote_handler* xdebug_handler_get(char* mode)
{
	xdebug_remote_handler_info *ptr = handlers;

	while (ptr->name) {
		if (strcmp (mode, ptr->name) == 0) {
			return &ptr->handler;
		}
		ptr++;
	}
	return NULL;
}
