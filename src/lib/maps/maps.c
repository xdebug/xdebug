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
#include "../log.h"
#include "../mm.h"
#include "../php-header.h"
#include "../usefulstuff.h"

static void scan_directory(const char *dir)
{
#ifndef WIN32
	struct stat dir_info;
#endif
	char *map_dir = xdebug_sprintf("%s%c.xdebug", dir, DEFAULT_SLASH);

#ifndef WIN32
	/* See if .xdebug directory exists */
	if (stat(map_dir, &dir_info) == -1) {
		xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_DEBUG, "NODIR", "The map directory '%s' does not exist", map_dir);
		return;
	}

	fprintf(stdout, "Map directory %s is a directory: %s\n", map_dir, S_ISDIR(dir_info.st_mode) ? "yes" : "no");
	if (!S_ISDIR(dir_info.st_mode)) {
		xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_WARN, "NOTADIR", "The map directory '%s' is not a directory", map_dir);
		return;
	}
#endif

	/* Read all files ending in .map */
	xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_INFO, "SCAN", "Scanning for map files in the map directory '%s'", map_dir);
}

void xdebug_path_maps_scan(const char *script_source)
{
	xdebug_arg *parts;
	char *slash;
	xdebug_str *current_dir = NULL, *parent_dir = NULL, *grand_dir = NULL;

	/* For CLI scripts, only scan the current directory for map files */
	if (strcmp(script_source, "Standard input code") == 0) {
		size_t length;
		char *current_directory = virtual_getcwd_ex(&length);

		scan_directory(current_directory);

		efree(current_directory);

		return;
	}

	/* Otherwise, find the grand parent, parent, and script directory from the path */
	slash = xdebug_sprintf("%c", DEFAULT_SLASH);
	parts = xdebug_arg_ctor();
	xdebug_explode(slash, script_source, parts, -1);

	current_dir = parts->c > 2 ? xdebug_join(slash, parts, 0, parts->c - 2) : NULL;
	parent_dir = parts->c > 3 ? xdebug_join(slash, parts, 0, parts->c - 3) : NULL;
	grand_dir = parts->c > 4 ? xdebug_join(slash, parts, 0, parts->c - 4) : NULL;

	if (grand_dir) {
		scan_directory(grand_dir->d);
		xdebug_str_free(grand_dir);
	}
	if (parent_dir) {
		scan_directory(parent_dir->d);
		xdebug_str_free(parent_dir);
	}
	if (current_dir) {
		scan_directory(current_dir->d);
		xdebug_str_free(current_dir);
	}

	xdfree(slash);
	xdebug_arg_dtor(parts);
}

/* Private Functions */
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
