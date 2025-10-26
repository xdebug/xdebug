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

#ifndef __HAVE_NORMALIZE_PATH_H__
#define __HAVE_NORMALIZE_PATH_H__

#ifdef __cplusplus
extern "C" {
#endif

char *xdebug_normalize_path_char(const char *path);
#ifdef PHP_WIN32
void xdebug_normalize_path_xdebug_str_in_place(xdebug_str *path);
#else
# define xdebug_normalize_path_xdebug_str_in_place(path)
#endif

#ifdef __cplusplus
}
#endif

#endif
