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

#include "../hash.h"
#include "../str.h"

#define XDEBUG_PATH_MAP_TYPE_UNKNOWN   0x00
#define XDEBUG_PATH_MAP_TYPE_DIRECTORY 0x01
#define XDEBUG_PATH_MAP_TYPE_FILE      0x02
#define XDEBUG_PATH_MAP_TYPE_LINES     0x03

typedef struct xdebug_path_map_element {
	xdebug_str *path;
	int         begin;
	int         end;
} xdebug_path_map_element;

typedef struct xdebug_path_mapping {
	int         type;
	xdebug_path_map_element *remote;
	xdebug_path_map_element *local;
} xdebug_path_mapping;

typedef struct xdebug_path_maps {
	xdebug_hash *remote_to_local_map;
} xdebug_path_maps;

/* Functions for testing, and not exported into Xdebug */
size_t xdebug_path_maps_get_rule_count(xdebug_path_maps *maps);

xdebug_path_mapping *remote_to_local(xdebug_path_maps *maps, const char *remote);

xdebug_path_map_element* xdebug_path_map_element_ctor(void);
void xdebug_path_map_element_dtor(xdebug_path_map_element *element);

void xdebug_path_mapping_dtor(void *mapping);

#endif
