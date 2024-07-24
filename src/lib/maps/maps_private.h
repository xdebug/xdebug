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

#ifndef __XDEBUG_MAPS_MAPS_PRIVATE_H__
#define __XDEBUG_MAPS_MAPS_PRIVATE_H__

#define PATH_MAPS_OK             0x0000
#define PATH_MAPS_CANT_OPEN_FILE 0x0001

typedef struct xdebug_path_maps {
} xdebug_path_maps;

bool xdebug_path_maps_parse_file(xdebug_path_maps *maps, const char *filename, int *error_code, char **error_message);

#endif
