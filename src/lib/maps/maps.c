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

void xdebug_path_map_range_set(xdebug_path_map_range *range, int remote_begin, int remote_end, int local_begin, int local_end)
{
	range->remote_begin = remote_begin;
	range->remote_end   = remote_end;
	range->local_begin  = local_begin;
	range->local_end    = local_end;
}

void xdebug_path_map_range_copy(xdebug_path_map_range *from, xdebug_path_map_range *to)
{
	to->remote_begin = from->remote_begin;
	to->remote_end   = from->remote_end;
	to->local_begin  = from->local_begin;
	to->local_end    = from->local_end;
}

void xdebug_path_map_range_dtor(xdebug_path_map_range *range)
{
	/* Do nothing */
}

xdebug_path_mapping *xdebug_path_mapping_ctor(void)
{
	xdebug_path_mapping *tmp = (xdebug_path_mapping*) xdcalloc(1, sizeof(xdebug_path_mapping));

	tmp->line_ranges = xdebug_vector_alloc(sizeof(xdebug_path_map_range), (xdebug_vector_dtor) xdebug_path_map_range_dtor);

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

	xdebug_vector_destroy(tmp->line_ranges);

	xdfree(tmp);
}

xdebug_path_mapping *xdebug_path_mapping_clone(xdebug_path_mapping *mapping)
{
	xdebug_path_mapping *tmp = (xdebug_path_mapping*) xdcalloc(1, sizeof(xdebug_path_mapping));

	tmp->type        = mapping->type;
	tmp->remote_path = xdebug_str_copy(mapping->remote_path);
	tmp->local_path  = xdebug_str_copy(mapping->local_path);
	tmp->line_ranges = xdebug_vector_clone(mapping->line_ranges);

	return tmp;
}
