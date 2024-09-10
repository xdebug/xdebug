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

xdebug_path_map_range* xdebug_path_map_range_ctor(int remote_begin, int remote_end, int local_begin, int local_end)
{
	xdebug_path_map_range *tmp = (xdebug_path_map_range*) xdcalloc(1, sizeof(xdebug_path_map_range));

	tmp->ref_count    = 1;
	tmp->remote_begin = remote_begin;
	tmp->remote_end   = remote_end;
	tmp->local_begin  = local_begin;
	tmp->local_end    = local_end;

	return tmp;
}

xdebug_path_map_range* xdebug_path_map_range_copy(xdebug_path_map_range *range)
{
	range->ref_count++;

	return range;
}

void xdebug_path_map_range_dtor(xdebug_path_map_range *range)
{
	range->ref_count--;

	if (range->ref_count) {
		return;
	}

	if (range->next) {
		xdebug_path_map_range_dtor(range->next);
	}
	xdfree(range);
}

xdebug_path_mapping *xdebug_path_mapping_ctor(void)
{
	xdebug_path_mapping *tmp = (xdebug_path_mapping*) xdcalloc(1, sizeof(xdebug_path_mapping));

	return tmp;
}

void xdebug_path_mapping_dtor(void *mapping)
{
	xdebug_path_mapping *tmp = (xdebug_path_mapping*) mapping;

	if (tmp->remote_path) {
		xdebug_str_free(tmp->remote_path);
	}
	if (tmp->local_path) {
		xdebug_str_free(tmp->local_path);
	}
	if (tmp->head_range_ptr) {
		xdebug_path_map_range_dtor(tmp->head_range_ptr);
	}
	xdfree(tmp);
}

