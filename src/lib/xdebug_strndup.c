/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2024 Derick Rethans                               |
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "mm.h"
#include "xdebug_strndup.h"

/**
 * Duplicate zend_strndup in core to avoid mismatches
 * in C-runtime libraries when xdebug and core are built
 * with different run-time libraries.
 */
char *xdebug_strndup(const char *s, int length)
{
	char *p;

	p = (char *) xdmalloc(length + 1);
	if (p == NULL) {
		return p;
	}
	if (length) {
		memcpy(p, s, length);
	}
	p[length] = 0;
	return p;
}
