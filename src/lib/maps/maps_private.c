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
#include "../strrnchr.h"
#include "../vector.h"
#include "../xdebug_strndup.h"

static size_t do_local_binary_search(xdebug_vector *line_ranges, int low, int high, size_t remote_line)
{
	while (low <= high) {
		int                    mid = low + (high - low) / 2;
		xdebug_path_map_range *ptr = (xdebug_path_map_range*) xdebug_vector_element_get(line_ranges, mid);

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

		/* Not found, so choose between first or second half */
		if (remote_line < ptr->remote_begin) {
			high = mid - 1;
		} else {
			low = mid + 1;
		}
	}

	return -1;
}

static size_t find_local_line_number_from_ranges(size_t remote_line, xdebug_vector *line_ranges)
{
	return do_local_binary_search(line_ranges, 0, XDEBUG_VECTOR_COUNT(line_ranges) - 1, remote_line);
}

/* Returns the remote type, and sets *local_path and *local_line if the type is not XDEBUG_PATH_MAP_TYPE_UNKNOWN */
int remote_to_local(xdebug_path_maps *maps, const char *remote_path, size_t remote_line, xdebug_str **local_path, size_t *local_line)
{
	xdebug_path_mapping *result;
	xdebug_hash *map = maps->remote_to_local_map;

	if (!xdebug_hash_find(map, remote_path, strlen(remote_path), (void**) &result)) {
		/* We can't find an exact file match, so now try to see if we have a directory match, starting with the full
		 * path and then removing the trailing directory path until there are none left */
		char *end_slash;
		char *directory;

		end_slash = strrchr((char*) remote_path, '/');
		if (!end_slash) {
			return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
		}

		directory = xdstrndup(remote_path, end_slash - remote_path + 1);
		end_slash = strrchr((char*) directory, '/');

		do {
			size_t n = end_slash - directory + 1;

			if (xdebug_hash_find(map, directory, n, (void**) &result)) {
				if (result->type == XDEBUG_PATH_MAP_TYPE_DIRECTORY) {
					*local_line = remote_line;

					*local_path = xdebug_str_new();
					xdebug_str_add_fmt(*local_path, "%s%s", result->local_path->d, remote_path + n);

					xdfree(directory);
					return XDEBUG_PATH_MAP_TYPE_DIRECTORY;
				}
			}

			end_slash = strrnchr(directory, '/', n - 1);
		} while (end_slash);

		xdfree(directory);
		return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
	}

	*local_path = xdebug_str_copy(result->local_path);

	switch (result->type) {
		case XDEBUG_PATH_MAP_TYPE_DIRECTORY:
			/* FIXME: directory mapping */
		case XDEBUG_PATH_MAP_TYPE_FILE:
			*local_line = remote_line;
			break;
		case XDEBUG_PATH_MAP_TYPE_LINES:
			size_t tmp_local_line = find_local_line_number_from_ranges(remote_line, result->line_ranges);

			if (tmp_local_line == -1) {
				return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
			}

			*local_line = tmp_local_line;
			break;
	}

	return result->type;
}

static size_t do_remote_binary_search(xdebug_vector *line_ranges, int low, int high, size_t local_line)
{
	while (low <= high) {
		int                    mid = low + (high - low) / 2;
		xdebug_path_map_range *ptr = (xdebug_path_map_range*) xdebug_vector_element_get(line_ranges, mid);

		/* 1:1 match */
		if (ptr->local_begin == ptr->local_end && ptr->remote_begin == ptr->remote_end && local_line == ptr->local_begin) {
			return ptr->remote_begin;
		}
		/* n:1 match */
		if (ptr->remote_begin == ptr->remote_end && (local_line >= ptr->local_begin) && (local_line <= ptr->local_end)) {
			return ptr->remote_begin;
		}
		/* n:m match */
		if ((local_line >= ptr->local_begin) && (local_line <= ptr->local_end)) {
			return local_line - ptr->local_begin + ptr->remote_begin;
		}

		/* Not found, so choose between first or second half */
		if (local_line < ptr->local_begin) {
			high = mid - 1;
		} else {
			low = mid + 1;
		}
	}

	return -1;
}

static size_t find_remote_line_number_from_ranges(size_t local_line, xdebug_vector *line_ranges)
{
	return do_remote_binary_search(line_ranges, 0, XDEBUG_VECTOR_COUNT(line_ranges) - 1, local_line);
}

/* Returns the local type, and sets *remote_path and *remote_line if the type is not XDEBUG_PATH_MAP_TYPE_UNKNOWN */
int local_to_remote(xdebug_path_maps *maps, const char *local_path, size_t local_line, xdebug_str **remote_path, size_t *remote_line)
{
	xdebug_path_mapping *result;
	xdebug_hash *map = maps->local_to_remote_map;

	if (!xdebug_hash_find(map, local_path, strlen(local_path), (void**) &result)) {
		/* We can't find an exact file match, so now try to see if we have a directory match, starting with the full
		 * path and then removing the trailing directory path until there are none left */
		char *end_slash;
		char *directory;

		end_slash = strrchr((char*) local_path, '/');
		if (!end_slash) {
			return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
		}

		directory = xdstrndup(local_path, end_slash - local_path + 1);
		end_slash = strrchr((char*) directory, '/');

		do {
			size_t n = end_slash - directory + 1;

			if (xdebug_hash_find(map, directory, n, (void**) &result)) {
				if (result->type == XDEBUG_PATH_MAP_TYPE_DIRECTORY) {
					*remote_line = local_line;

					*remote_path = xdebug_str_new();
					xdebug_str_add_fmt(*remote_path, "%s%s", result->remote_path->d, local_path + n);

					xdfree(directory);
					return XDEBUG_PATH_MAP_TYPE_DIRECTORY;
				}
			}

			end_slash = strrnchr(directory, '/', n - 1);
		} while (end_slash);

		xdfree(directory);
		return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
	}

	*remote_path = xdebug_str_copy(result->remote_path);

	switch (result->type) {
		case XDEBUG_PATH_MAP_TYPE_DIRECTORY:
			/* FIXME: directory mapping */
		case XDEBUG_PATH_MAP_TYPE_FILE:
			*remote_line = local_line;
			break;
		case XDEBUG_PATH_MAP_TYPE_LINES:
			size_t tmp_remote_line = find_remote_line_number_from_ranges(local_line, result->line_ranges);

			if (tmp_remote_line == -1) {
				return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
			}

			*remote_line = tmp_remote_line;
			break;
	}

	return result->type;
}

xdebug_path_maps *xdebug_path_maps_ctor(void)
{
	xdebug_path_maps *tmp = (xdebug_path_maps*) xdcalloc(1, sizeof(xdebug_path_maps));
	tmp->remote_to_local_map = xdebug_hash_alloc(128, xdebug_path_mapping_dtor);
	tmp->local_to_remote_map = xdebug_hash_alloc(128, xdebug_path_mapping_dtor);

	return tmp;
}

void xdebug_path_maps_dtor(xdebug_path_maps *maps)
{
	xdebug_hash_destroy(maps->remote_to_local_map);
	xdebug_hash_destroy(maps->local_to_remote_map);
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
