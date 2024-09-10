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
#include <stdlib.h>
#include <string.h>

#include "maps_private.h"
#include "../mm.h"
#include "../xdebug_strndup.h"

/** Private API, mainly used for testing */

size_t xdebug_path_maps_get_rule_count(xdebug_path_maps *maps)
{
	return maps->remote_to_local_map->size;
}

static size_t find_line_number_from_ranges(size_t remote_line, xdebug_path_map_range *ranges)
{
	xdebug_path_map_range *ptr = ranges;

	do {
		/* 1:1 match */
		if (ptr->remote_begin == ptr->remote_end && ptr->local_begin == ptr->local_end && remote_line == ptr->remote_begin) {
			return ptr->local_begin;
		}
		/* n:1 match */
		if (ptr->local_begin == ptr->local_end && (remote_line >= ptr->remote_begin) && (remote_line <= ptr->remote_end)) {
			return ptr->local_begin;
		}
		/* n:m match */
		if ((remote_line >= ptr->remote_begin) && (remote_line <= ptr->remote_end)) {
			return remote_line - ptr->remote_begin + ptr->local_begin;
		}
		ptr = ptr->next;
	} while (ptr != NULL);

	return -1;
}

/* Returns the remote type, and sets *local_path and *local_line if the type is not XDEBUG_PATH_MAP_TYPE_UNKNOWN */
int remote_to_local(xdebug_path_maps *maps, const char *remote_path, size_t remote_line, xdebug_str **local_path, size_t *local_line)
{
	xdebug_path_mapping *result;

	if (!xdebug_hash_find(maps->remote_to_local_map, remote_path, strlen(remote_path), (void**) &result)) {
		return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
	}

	*local_path = xdebug_str_copy(result->local_path);

	switch (result->type) {
		case XDEBUG_PATH_MAP_TYPE_DIRECTORY:
		case XDEBUG_PATH_MAP_TYPE_FILE:
			*local_line = remote_line;
			break;
		case XDEBUG_PATH_MAP_TYPE_LINES:
			*local_line = find_line_number_from_ranges(remote_line, result->head_range_ptr);
			break;
	}

	return result->type;
}
