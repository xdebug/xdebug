/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2021 Derick Rethans                               |
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

#ifndef __HAVE_USEFULSTUFF_H__
#define __HAVE_USEFULSTUFF_H__

#include "php_xdebug.h"
#include "src/lib/compat.h"

#if HAVE_XDEBUG_ZLIB
# include <zlib.h>
#endif

#define XDEBUG_FILE_TYPE_NULL    0
#define XDEBUG_FILE_TYPE_NORMAL  1
#if HAVE_XDEBUG_ZLIB
# define XDEBUG_FILE_TYPE_GZ     2
#endif

typedef struct _xdebug_file {
	int type;
	union {
		FILE   *normal;
#if HAVE_XDEBUG_ZLIB
		gzFile  gz;
#endif
	} fp;
	char *name;
} xdebug_file;

xdebug_file *xdebug_file_ctor(void);
void xdebug_init_generic_file(xdebug_file *xf);
void xdebug_free_generic_file(xdebug_file *xf);
int xdebug_generic_fopen(xdebug_file *file, const char *filename, const char *extension, const char *mode);
int xdebug_generic_flush(xdebug_file *file);
int XDEBUG_ATTRIBUTE_FORMAT(printf, 2, 3) xdebug_generic_fprintf(xdebug_file *file, const char *fmt, ...);
size_t xdebug_generic_fwrite(const void *ptr, size_t size, size_t nmemb, xdebug_file *file);
int xdebug_generic_fclose(xdebug_file *file);

typedef struct xdebug_arg {
	int    c;
	char **args;
} xdebug_arg;

xdebug_arg *xdebug_arg_ctor(void);
void xdebug_arg_dtor(xdebug_arg *arg);

xdebug_str* xdebug_join(const char *delim, xdebug_arg *args, int begin, int end);
void xdebug_explode(const char *delim, const char *str, xdebug_arg *args, int limit);
const char* xdebug_memnstr(const char *haystack, const char *needle, int needle_len, const char *end);
char* xdebug_strrstr(const char* haystack, const char* needle);
char *xdebug_trim(const char *str);
char *xdebug_path_to_url(zend_string *fileurl);
char *xdebug_path_from_url(zend_string *fileurl);
FILE *xdebug_fopen(char *fname, const char *mode, const char *extension, char **new_fname);
int xdebug_format_output_filename(char **filename, char *format, char *script_name);
int xdebug_format_file_link(char **filename, const char *error_filename, int error_lineno);
int xdebug_format_filename(char **formatted_name, const char *default_format, zend_string *filename);

#endif
