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

#include "maps_private.h"
#include "../mm.h"


xdebug_path_maps *xdebug_path_maps_ctor(void)
{
	xdebug_path_maps *tmp = (xdebug_path_maps*) xdcalloc(1, sizeof(xdebug_path_maps));
	tmp->remote_to_local_map = xdebug_hash_alloc(128, xdebug_path_mapping_dtor);

	return tmp;
}

void xdebug_path_maps_dtor(xdebug_path_maps *maps)
{
	xdebug_hash_destroy(maps->remote_to_local_map);
	xdfree(maps);
}

xdebug_path_map_element* xdebug_path_map_element_ctor(void)
{
	xdebug_path_map_element *tmp = (xdebug_path_map_element*) xdcalloc(1, sizeof(xdebug_path_map_element));

	return tmp;
}

void xdebug_path_map_element_dtor(xdebug_path_map_element *element)
{
	if (element->path) {
		xdebug_str_free(element->path);
	}
	xdfree(element);
}

void xdebug_path_mapping_dtor(void *mapping)
{
	xdebug_path_mapping *tmp = (xdebug_path_mapping*) mapping;

	if (tmp->remote) {
		xdebug_path_map_element_dtor(tmp->remote);
	}
	if (tmp->local) {
		xdebug_path_map_element_dtor(tmp->local);
	}
	xdfree(tmp);
}

