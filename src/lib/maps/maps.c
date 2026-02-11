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
#include <stddef.h>
#include <stdlib.h>

#ifndef XDEBUG_NO_PHP_FEATURES
# include "php_xdebug.h"
#endif

#if PHP_VERSION_ID >= 80500
# include "php_glob.h"
# define xdebug_glob php_glob
# define xdebug_glob_t php_glob_t
# define xdebug_globfree php_globfree
#else
# ifndef PHP_WIN32
#  include <glob.h>
# else
#  include "php.h"
#  include "win32/glob.h"
# endif
# define xdebug_glob glob
# define xdebug_glob_t glob_t
# define xdebug_globfree globfree
#endif

#ifndef PHP_GLOB_NOMATCH
# define PHP_GLOB_NOMATCH GLOB_NOMATCH
#endif

#include "maps_private.h"
#include "parser.h"
#include "../lib_private.h"
#include "../log.h"
#include "../mm.h"
#include "../php-header.h"
#include "../usefulstuff.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

static bool scan_directory_exists(const char *dir)
{
#ifndef WIN32
	struct stat dir_info;
#endif
	char *map_dir = xdebug_sprintf("%s%c.xdebug", dir, DEFAULT_SLASH);

#ifndef WIN32
	/* See if .xdebug directory exists */
	if (stat(map_dir, &dir_info) == -1) {
		xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_DEBUG, "NODIR", "The map directory '%s' does not exist", map_dir);
		return false;
	}

	if (!S_ISDIR(dir_info.st_mode)) {
		xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_WARN, "NOTADIR", "The map directory '%s' is not a directory", map_dir);
		return false;
	}
#endif

	return true;
}

static void scan_directory(const char *dir)
{
	char *scan_dir = xdebug_sprintf("%s%c.xdebug%c*.map", dir, DEFAULT_SLASH, DEFAULT_SLASH);
	xdebug_glob_t globbuf;
	int glob_result = 0;
	size_t i;

	/* Read all files ending in .map */
	xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_INFO, "SCAN", "Scanning for map files with pattern '%s'", scan_dir);

	glob_result = xdebug_glob(scan_dir, 0, NULL, &globbuf);

	switch (glob_result) {
		case 0: /* No error */
			break;

		case PHP_GLOB_NOMATCH:
			xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_DEBUG, "NOMATCH", "No map files found with pattern '%s'", scan_dir);
			xdfree(scan_dir);

			return;

		default:
			xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_WARN, "ERR-SCAN", "Scanning for map files with pattern '%s' failed", scan_dir);
			xdfree(scan_dir);

			return;
	}

	for (i = 0; i < globbuf.gl_pathc; i++) {
		int error_code;
		int error_line;
		char *error_message;

		xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_INFO, "SCAN-READ", "Reading mapping file '%s'", globbuf.gl_pathv[i]);

		if (!xdebug_path_maps_parse_file(XG_LIB(path_mapping_information), dir, globbuf.gl_pathv[i], &error_code, &error_line, &error_message)) {
			xdebug_log_ex(
				XLOG_CHAN_PATHMAP, XLOG_WARN,
				"ERR-FILE", "Parse error in path mapping file '%s' on line %d: %s",
				globbuf.gl_pathv[i], error_line, error_message
			);
		}
	}

	xdebug_globfree(&globbuf);
	xdfree(scan_dir);
}

static void dump_rule_info(void *ret, xdebug_hash_element *e)
{
	xdebug_path_mapping *rule = (xdebug_path_mapping*) e->ptr;

	if ((rule->type & XDEBUG_PATH_MAP_FLAGS_MASK) == XDEBUG_PATH_MAP_FLAGS_SKIP) {
		switch (rule->type & XDEBUG_PATH_MAP_TYPE_MASK) {
			case XDEBUG_PATH_MAP_TYPE_DIRECTORY:
				xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_DEBUG, "RULE", "Skip directory '%s'", XDEBUG_STR_VAL(rule->remote_path));
				break;
			case XDEBUG_PATH_MAP_TYPE_FILE:
				xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_DEBUG, "RULE", "Skip file '%s'", XDEBUG_STR_VAL(rule->remote_path));
				break;
		}
	} else {
		switch (rule->type & XDEBUG_PATH_MAP_TYPE_MASK) {
			case XDEBUG_PATH_MAP_TYPE_DIRECTORY:
				xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_DEBUG, "RULE", "Map directory '%s' to '%s'", XDEBUG_STR_VAL(rule->remote_path), XDEBUG_STR_VAL(rule->m.local_path));
				break;
			case XDEBUG_PATH_MAP_TYPE_FILE:
				xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_DEBUG, "RULE", "Map file '%s' to '%s'", XDEBUG_STR_VAL(rule->remote_path), XDEBUG_STR_VAL(rule->m.local_path));
				break;
		}
	}
}

static void dump_mapping_rules(xdebug_path_maps *map_info)
{
	xdebug_hash_apply(map_info->remote_to_local_map, NULL, dump_rule_info);
	xdebug_hash_apply(map_info->local_to_remote_map, NULL, dump_rule_info);
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

		if (scan_directory_exists(current_directory)) {
			scan_directory(current_directory);
		}

		efree(current_directory);

		return;
	}

	/* Otherwise, find the grand parent, parent, and script directory from the path */
	slash = xdebug_sprintf("%c", DEFAULT_SLASH);
	parts = xdebug_arg_ctor();
	xdebug_explode(slash, script_source, parts, -1);

	current_dir = parts->c >= 2 ? xdebug_join(slash, parts, 0, parts->c - 2) : NULL;
	parent_dir = parts->c >= 3 ? xdebug_join(slash, parts, 0, parts->c - 3) : NULL;
	grand_dir = parts->c >= 4 ? xdebug_join(slash, parts, 0, parts->c - 4) : NULL;

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

	xdebug_log_ex(XLOG_CHAN_PATHMAP, XLOG_DEBUG, "RULES", "Found %zd path mapping rules", XG_LIB(path_mapping_information)->remote_to_local_map->size);
	dump_mapping_rules(XG_LIB(path_mapping_information));
}

int xdebug_path_maps_local_to_remote(const char *local_path, size_t local_line, xdebug_str **remote_path, size_t *remote_line)
{
	return local_to_remote(
		XG_LIB(path_mapping_information),
		local_path, local_line,
		remote_path, remote_line
	);
}

int xdebug_path_maps_remote_to_local(const char *remote_path, size_t remote_line, xdebug_str **local_path, size_t *local_line)
{
	return remote_to_local(
		XG_LIB(path_mapping_information),
		remote_path, remote_line,
		local_path, local_line
	);
}
