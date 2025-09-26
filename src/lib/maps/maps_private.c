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
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "maps_private.h"
#include "../mm.h"
#include "../strrnchr.h"
#include "../usefulstuff.h"
#include "../vector.h"
#include "../xdebug_strndup.h"

static bool do_local_binary_search(xdebug_vector *line_ranges, int low, int high, size_t remote_line, xdebug_str **local_path, size_t *local_line)
{
	while (low <= high) {
		int                    mid = low + (high - low) / 2;
		xdebug_path_map_range *ptr = (xdebug_path_map_range*) xdebug_vector_element_get(line_ranges, mid);

		/* 1:1 match */
		if (ptr->remote_begin == ptr->remote_end && ptr->local_begin == ptr->local_end && remote_line == ptr->remote_begin) {
			*local_path = ptr->local_path;
			*local_line = ptr->local_begin;
			return true;
		}
		/* n:1 match */
		if (ptr->local_begin == ptr->local_end && (remote_line >= ptr->remote_begin) && (remote_line <= ptr->remote_end)) {
			*local_path = ptr->local_path;
			*local_line = ptr->local_begin;
			return true;
		}
		/* n:m match */
		if ((remote_line >= ptr->remote_begin) && (remote_line <= ptr->remote_end)) {
			*local_path = ptr->local_path;
			*local_line = remote_line - ptr->remote_begin + ptr->local_begin;
			return true;
		}

		/* Not found, so choose between first or second half */
		if (remote_line < ptr->remote_begin) {
			high = mid - 1;
		} else {
			low = mid + 1;
		}
	}

	return false;
}

static bool find_local_line_number_from_ranges(size_t remote_line, xdebug_vector *line_ranges, xdebug_str **local_path, size_t *local_line)
{
	return do_local_binary_search(line_ranges, 0, XDEBUG_VECTOR_COUNT(line_ranges) - 1, remote_line, local_path, local_line);
}

/* Returns the remote type, and sets *local_path and *local_line if the type is not XDEBUG_PATH_MAP_TYPE_UNKNOWN or XDEBUG_PATH_MAP_FLAGS_SKIP */
int remote_to_local(xdebug_path_maps *maps, const char *remote_path, size_t remote_line, xdebug_str **local_path, size_t *local_line)
{
	xdebug_path_mapping *result;
	xdebug_hash *map = maps->remote_to_local_map;
	char *url_path = xdebug_normalize_path_char(remote_path);

	if (!xdebug_hash_find(map, url_path, strlen(url_path), (void**) &result)) {
		/* We can't find an exact file match, so now try to see if we have a directory match, starting with the full
		 * path and then removing the trailing directory path until there are none left */
		char *end_slash;
		char *directory;

		end_slash = strrchr((char*) url_path, '/');
		if (!end_slash) {
			xdfree(url_path);
			return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
		}

		directory = xdstrndup(url_path, end_slash - url_path + 1);
		end_slash = strrchr((char*) directory, '/');

		do {
			size_t n = end_slash - directory + 1;

			if (xdebug_hash_find(map, directory, n, (void**) &result)) {
				if (result->type & XDEBUG_PATH_MAP_FLAGS_SKIP) {
					xdfree(directory);

					goto skipped_match;
				}
				if (result->type == XDEBUG_PATH_MAP_TYPE_DIRECTORY) {
					*local_line = remote_line;

					*local_path = xdebug_str_new();
					xdebug_str_add_fmt(*local_path, "%s%s", result->m.local_path->d, url_path + n);

					xdfree(directory);
					xdfree(url_path);
					return XDEBUG_PATH_MAP_TYPE_DIRECTORY;
				}
			}

			end_slash = strrnchr(directory, '/', n - 1);
		} while (end_slash);

		xdfree(directory);
		xdfree(url_path);
		return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
	}

	if (result->type & XDEBUG_PATH_MAP_FLAGS_SKIP) {
		goto skipped_match;
	}

	switch (result->type) {
		case XDEBUG_PATH_MAP_TYPE_DIRECTORY:
			/* This should not happen during normal execution, because if a remote path is a
			 * directory, then the OS didn't actually provide a file path to map. But, it can be
			 * useful for testing an user-initiated mapping. */

			/* 'break' intentially omitted */

		case XDEBUG_PATH_MAP_TYPE_FILE:
			*local_path = xdebug_str_copy(result->m.local_path);
			*local_line = remote_line;
			return result->type;

		case XDEBUG_PATH_MAP_TYPE_LINES: {
			xdebug_str *result_path;
			size_t      result_line;

			if (!result->m.line_ranges) {
				xdfree(url_path);
				return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
			}

			if (!find_local_line_number_from_ranges(remote_line, result->m.line_ranges, &result_path, &result_line)) {
				xdfree(url_path);
				return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
			}

			if (!result_path) {
				goto skipped_match;
			}

			*local_path = xdebug_str_copy(result_path);
			*local_line = result_line;

			xdfree(url_path);
			return result->type;
		}

		default:
			assert(false);
	}


skipped_match:
	*local_path = NULL;
	*local_line = -1;

	xdfree(url_path);
	return XDEBUG_PATH_MAP_FLAGS_SKIP;
}

static bool do_remote_binary_search(xdebug_vector *line_ranges, int low, int high, size_t local_line, xdebug_str **remote_path, size_t *remote_line)
{
	while (low <= high) {
		int                    mid = low + (high - low) / 2;
		xdebug_path_map_range *ptr = (xdebug_path_map_range*) xdebug_vector_element_get(line_ranges, mid);

		/* 1:1 match */
		if (ptr->local_begin == ptr->local_end && ptr->remote_begin == ptr->remote_end && local_line == ptr->local_begin) {
			*remote_path = ptr->local_path;
			*remote_line = ptr->remote_begin;
			return true;
		}
		/* n:1 match */
		if (ptr->remote_begin == ptr->remote_end && (local_line >= ptr->local_begin) && (local_line <= ptr->local_end)) {
			*remote_path = ptr->local_path;
			*remote_line = ptr->remote_begin;
			return true;
		}
		/* n:m match */
		if ((local_line >= ptr->local_begin) && (local_line <= ptr->local_end)) {
			*remote_path = ptr->local_path;
			*remote_line = local_line - ptr->local_begin + ptr->remote_begin;
			return true;
		}

		/* Not found, so choose between first or second half */
		if (local_line < ptr->local_begin) {
			high = mid - 1;
		} else {
			low = mid + 1;
		}
	}

	return false;
}

static bool find_remote_line_number_from_ranges(size_t local_line, xdebug_vector *line_ranges, xdebug_str **remote_path, size_t *remote_line)
{
	return do_remote_binary_search(line_ranges, 0, XDEBUG_VECTOR_COUNT(line_ranges) - 1, local_line, remote_path, remote_line);
}

/* Returns the local type, and sets *remote_path and *remote_line if the type is not XDEBUG_PATH_MAP_TYPE_UNKNOWN */
int local_to_remote(xdebug_path_maps *maps, const char *local_path, size_t local_line, xdebug_str **remote_path, size_t *remote_line)
{
	xdebug_path_mapping *result;
	xdebug_hash *map = maps->local_to_remote_map;
	char *url_path = xdebug_normalize_path_char(local_path);

	if (!xdebug_hash_find(map, url_path, strlen(url_path), (void**) &result)) {
		/* We can't find an exact file match, so now try to see if we have a directory match, starting with the full
		 * path and then removing the trailing directory path until there are none left */
		char *end_slash;
		char *directory;

		end_slash = strrchr((char*) url_path, '/');
		if (!end_slash) {
			xdfree(url_path);
			return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
		}

		directory = xdstrndup(url_path, end_slash - url_path + 1);
		end_slash = strrchr((char*) directory, '/');

		do {
			size_t n = end_slash - directory + 1;

			if (xdebug_hash_find(map, directory, n, (void**) &result)) {
				if (result->type == XDEBUG_PATH_MAP_TYPE_DIRECTORY) {
					*remote_line = local_line;

					*remote_path = xdebug_str_new();
					xdebug_str_add_fmt(*remote_path, "%s%s", result->remote_path->d, url_path + n);

					xdfree(directory);
					xdfree(url_path);
					return XDEBUG_PATH_MAP_TYPE_DIRECTORY;
				}
			}

			end_slash = strrnchr(directory, '/', n - 1);
		} while (end_slash);

		xdfree(directory);
		xdfree(url_path);
		return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
	}

	if (result->type & XDEBUG_PATH_MAP_FLAGS_SKIP) {
		assert(false);
	}

	switch (result->type) {
		case XDEBUG_PATH_MAP_TYPE_DIRECTORY:
			/* This should not happen during normal execution, because if a remote path is a
			 * directory, then the OS didn't actually provide a file path to map. But, it can be
			 * useful for testing an user-initiated mapping. */

			/* 'break' intentially omitted */

		case XDEBUG_PATH_MAP_TYPE_FILE:
			*remote_path = xdebug_str_copy(result->remote_path);
			*remote_line = local_line;
			break;

		case XDEBUG_PATH_MAP_TYPE_LINES:
			xdebug_str *result_path;
			size_t      result_line;

			if (!find_remote_line_number_from_ranges(local_line, result->m.line_ranges, &result_path, &result_line)) {
				return XDEBUG_PATH_MAP_TYPE_UNKNOWN;
			}

			*remote_path = xdebug_str_copy(result_path);
			*remote_line = result_line;
			break;
	}

	xdfree(url_path);
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

void xdebug_path_map_range_set(xdebug_path_map_range *range, int remote_begin, int remote_end, int local_flags, xdebug_str *local_path, int local_begin, int local_end)
{
	range->remote_begin = remote_begin;
	range->remote_end   = remote_end;
	range->local_flags  = local_flags;
	range->local_path   = local_path ? xdebug_str_copy(local_path) : NULL;
	range->local_begin  = local_begin;
	range->local_end    = local_end;
}

void xdebug_path_map_range_copy(xdebug_path_map_range *from, xdebug_path_map_range *to)
{
	to->remote_begin = from->remote_begin;
	to->remote_end   = from->remote_end;
	to->local_flags  = from->local_flags;
	to->local_path   = from->local_path ? xdebug_str_copy(from->local_path) : NULL;
	to->local_begin  = from->local_begin;
	to->local_end    = from->local_end;
}

void xdebug_path_map_range_dtor(xdebug_path_map_range *range)
{
	if (range->local_path) {
		xdebug_str_free(range->local_path);
	}
}

xdebug_path_mapping *xdebug_path_mapping_ctor(int type)
{
	xdebug_path_mapping *tmp = (xdebug_path_mapping*) xdcalloc(1, sizeof(xdebug_path_mapping));

	tmp->type = type;
	tmp->ref_count = 1;

	assert(!(type & XDEBUG_PATH_MAP_FLAGS_MASK));

	if ((type & XDEBUG_PATH_MAP_TYPE_MASK) == XDEBUG_PATH_MAP_TYPE_LINES) {
		tmp->m.line_ranges = xdebug_vector_alloc(sizeof(xdebug_path_map_range), (xdebug_vector_dtor) xdebug_path_map_range_dtor);
	} else {
		tmp->m.local_path = NULL;
	}

	return tmp;
}

void xdebug_path_mapping_dtor(void *mapping)
{
	xdebug_path_mapping *tmp = (xdebug_path_mapping*) mapping;

	tmp->ref_count--;

	if (tmp->ref_count > 0) {
		return;
	}

	if (tmp->remote_path) {
		xdebug_str_free(tmp->remote_path);
	}

	if (tmp->type == XDEBUG_PATH_MAP_TYPE_LINES) {
		if (tmp->m.line_ranges) {
			xdebug_vector_destroy(tmp->m.line_ranges);
		}
	} else if (tmp->m.local_path) {
		xdebug_str_free(tmp->m.local_path);
	}

	xdfree(tmp);
}

xdebug_path_mapping *xdebug_path_mapping_copy(xdebug_path_mapping *mapping)
{
	xdebug_path_mapping *tmp = (xdebug_path_mapping*) mapping;

	tmp->ref_count++;

	return tmp;
}
