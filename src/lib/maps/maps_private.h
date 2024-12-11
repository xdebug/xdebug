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
#include "../vector.h"

#define XDEBUG_PATH_MAP_TYPE_UNKNOWN   0x00
#define XDEBUG_PATH_MAP_TYPE_DIRECTORY 0x01
#define XDEBUG_PATH_MAP_TYPE_FILE      0x02
#define XDEBUG_PATH_MAP_TYPE_LINES     0x03

typedef struct xdebug_path_map_range xdebug_path_map_range;

struct xdebug_path_map_range {
	int remote_begin;
	int remote_end;
	int local_begin;
	int local_end;
};

typedef struct xdebug_path_mapping {
	int                      type;
	xdebug_str              *remote_path;
	xdebug_str              *local_path;
	xdebug_vector           *line_ranges;
} xdebug_path_mapping;

typedef struct xdebug_path_maps {
	xdebug_hash *remote_to_local_map;
	xdebug_hash *local_to_remote_map;
} xdebug_path_maps;

xdebug_path_maps *xdebug_path_maps_ctor(void);
void xdebug_path_maps_dtor(xdebug_path_maps *maps);

int remote_to_local(xdebug_path_maps *maps, const char *remote_path, size_t remote_line, xdebug_str **local_path, size_t *local_line);
int local_to_remote(xdebug_path_maps *maps, const char *local_path, size_t local_line, xdebug_str **remote_path, size_t *remote_line);

void xdebug_path_map_range_set(xdebug_path_map_range *from, int remote_begin, int remote_end, int local_begin, int local_end);
void xdebug_path_map_range_copy(xdebug_path_map_range *from, xdebug_path_map_range *to);
void xdebug_path_map_range_dtor(xdebug_path_map_range *range);

xdebug_path_mapping *xdebug_path_mapping_ctor(void);
void xdebug_path_mapping_dtor(void *mapping);
xdebug_path_mapping *xdebug_path_mapping_clone(xdebug_path_mapping *mapping);

#endif
