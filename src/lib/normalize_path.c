/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2025 Derick Rethans                               |
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

#include <string.h>

#include "mm.h"
#include "str.h"

#ifdef PHP_WIN32
char *xdebug_normalize_path_char(const char *path)
{
	char *new_path = xdstrdup(path);
	char *ptr = new_path;

	do {
		if ((*ptr) == '\\') {
			*ptr = '/';
		}
		ptr++;
	} while (*ptr != '\0');

	return new_path;
}

void xdebug_normalize_path_xdebug_str_in_place(xdebug_str *path)
{
	int i;

	for (i = 0; i < XDEBUG_STR_LEN(path); i++) {
		if (XDEBUG_STR_VAL(path)[i] == '\\') {
			XDEBUG_STR_VAL(path)[i] = '/';
		}
	}
}
#else
char *xdebug_normalize_path_char(const char *path)
{
	return xdstrdup(path);
}
#endif

