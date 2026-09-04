/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2026 Derick Rethans                               |
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
#include <stdlib.h>

#include "arg.h"

xdebug_arg *xdebug_arg_ctor(void)
{
	xdebug_arg *tmp = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));

	tmp->args = NULL;
	tmp->c    = 0;

	return tmp;
}

void xdebug_arg_dtor(xdebug_arg *arg)
{
	int i;

	for (i = 0; i < arg->c; i++) {
		xdfree(arg->args[i]);
	}
	if (arg->args) {
		xdfree(arg->args);
	}
	xdfree(arg);
}

xdebug_str* xdebug_join(const char *delim, xdebug_arg *args, int begin, int end)
{
	int         i;
	xdebug_str *ret = xdebug_str_new();

	if (begin < 0) {
		begin = 0;
	}
	if (end > args->c - 1) {
		end = args->c - 1;
	}
	for (i = begin; i < end; i++) {
		xdebug_str_add(ret, args->args[i], 0);
		xdebug_str_add(ret, delim, 0);
	}
	xdebug_str_add(ret, args->args[end], 0);
	return ret;
}

void xdebug_explode(const char *delim, const char *str, xdebug_arg *args, int limit)
{
	const char *p1, *p2, *endp;

	endp = str + strlen(str);

	p1 = str;
	p2 = xdebug_memnstr(str, delim, strlen(delim), endp);

	if (p2 == NULL) {
		args->c++;
		args->args = (char**) xdrealloc(args->args, sizeof(char*) * args->c);
		args->args[args->c - 1] = (char*) xdmalloc(strlen(str) + 1);
		memcpy(args->args[args->c - 1], p1, strlen(str));
		args->args[args->c - 1][strlen(str)] = '\0';
	} else {
		do {
			args->c++;
			args->args = (char**) xdrealloc(args->args, sizeof(char*) * args->c);
			args->args[args->c - 1] = (char*) xdmalloc(p2 - p1 + 1);
			memcpy(args->args[args->c - 1], p1, p2 - p1);
			args->args[args->c - 1][p2 - p1] = '\0';
			p1 = p2 + strlen(delim);
		} while ((p2 = xdebug_memnstr(p1, delim, strlen(delim), endp)) != NULL && (limit == -1 || --limit > 1));

		if (p1 <= endp) {
			args->c++;
			args->args = (char**) xdrealloc(args->args, sizeof(char*) * args->c);
			args->args[args->c - 1] = (char*) xdmalloc(endp - p1 + 1);
			memcpy(args->args[args->c - 1], p1, endp - p1);
			args->args[args->c - 1][endp - p1] = '\0';
		}
	}
}

const char* xdebug_memnstr(const char *haystack, const char *needle, int needle_len, const char *end)
{
	const char *p = haystack;
	char first = *needle;

	/* let end point to the last character where needle may start */
	end -= needle_len;

	while (p <= end) {
		while (*p != first)
			if (++p > end)
				return NULL;
		if (memcmp(p, needle, needle_len) == 0)
			return p;
		p++;
	}
	return NULL;
}
