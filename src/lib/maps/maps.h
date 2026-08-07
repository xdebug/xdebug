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

#ifndef __XDEBUG_MAPS_MAPS_H__
#define __XDEBUG_MAPS_MAPS_H__

#define XDEBUG_PATH_MAP_RESULT_OK      0
#define XDEBUG_PATH_MAP_RESULT_SKIP    1
#define XDEBUG_PATH_MAP_RESULT_UNKNOWN 2

void xdebug_path_maps_scan(const char *script_source);
int xdebug_path_maps_local_to_remote(const char *local_path, size_t local_line, xdebug_str **remote_path, size_t *remote_line);
int xdebug_path_maps_remote_to_local(const char *remote_path, size_t remote_line, xdebug_str **local_path, size_t *local_line);

#endif
