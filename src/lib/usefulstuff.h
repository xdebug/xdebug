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

#ifndef __HAVE_USEFULSTUFF_H__
#define __HAVE_USEFULSTUFF_H__

typedef struct xdebug_arg {
	int    c;
	char **args;
} xdebug_arg;

#define xdebug_arg_init(arg) {    \
	arg->args = NULL;             \
	arg->c    = 0;                \
}

#define xdebug_arg_dtor(arg) {     \
	int adi;                       \
	for (adi = 0; adi < arg->c; adi++) { \
		xdfree(arg->args[adi]);    \
	}                              \
	if (arg->args) {               \
		xdfree(arg->args);         \
	}                              \
	xdfree(arg);                   \
}

xdebug_str* xdebug_join(const char *delim, xdebug_arg *args, int begin, int end);
void xdebug_explode(const char *delim, char *str, xdebug_arg *args, int limit);
char* xdebug_memnstr(char *haystack, const char *needle, int needle_len, char *end);
char* xdebug_strrstr(const char* haystack, const char* needle);
double xdebug_get_utime(void);
char* xdebug_get_time(void);
char *xdebug_path_to_url(const char *fileurl);
char *xdebug_path_from_url(const char *fileurl);
FILE *xdebug_fopen(char *fname, const char *mode, const char *extension, char **new_fname);
int xdebug_format_output_filename(char **filename, char *format, char *script_name);
int xdebug_format_file_link(char **filename, const char *error_filename, int error_lineno);
int xdebug_format_filename(char **formatted_name, const char *format, const char *default_format, const char *filename);

#endif
