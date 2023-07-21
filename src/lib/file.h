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

#ifndef __HAVE_LIB_FILE_H__
#define __HAVE_LIB_FILE_H__

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
	struct {
		FILE   *normal;
#if HAVE_XDEBUG_ZLIB
		gzFile  gz;
#endif
	} fp;
	char *name;
} xdebug_file;

xdebug_file *xdebug_file_ctor(void);
void xdebug_file_dtor(xdebug_file *xf);
void xdebug_file_init(xdebug_file *xf);
void xdebug_file_deinit(xdebug_file *xf);
int xdebug_file_open(xdebug_file *file, const char *filename, const char *extension, const char *mode);
int xdebug_file_flush(xdebug_file *file);
int XDEBUG_ATTRIBUTE_FORMAT(printf, 2, 3) xdebug_file_printf(xdebug_file *file, const char *fmt, ...);
size_t xdebug_file_write(const void *ptr, size_t size, size_t nmemb, xdebug_file *file);
int xdebug_file_close(xdebug_file *file);

#endif
