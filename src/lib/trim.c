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
#include <ctype.h>
#include "mm.h"

char *xdebug_trim(const char *str)
{
	char *trimmed = NULL, *begin = (char *) str, *end = NULL;

	/* trim leading space */
	while (isspace((unsigned char) *begin)) {
		++begin;
	}

	/* All spaces */
	if (*begin == '\0') {
		return xdstrdup("");
	}

	/* trim trailing space */
	end = begin + strlen(begin) - 1;
	while (end > begin && isspace((unsigned char) *end)) {
		--end;
	}
	end++;

	trimmed = xdmalloc(end - begin + 1);
	memcpy(trimmed, begin, (end - begin));
	trimmed[(end - begin)] = '\0';

	return trimmed;
}
