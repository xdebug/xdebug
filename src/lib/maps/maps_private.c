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

xdebug_path_mapping *remote_to_local(xdebug_path_maps *maps, const char *remote)
{
	void *result;

	if (xdebug_hash_find(maps->remote_to_local_map, remote, strlen(remote), &result)) {
		return (xdebug_path_mapping*) result;
	}

	return NULL;
}
