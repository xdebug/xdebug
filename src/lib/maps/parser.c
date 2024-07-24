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

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "maps_private.h"

typedef struct path_maps_parser_state {
	char *current_local_prefix;
	char *current_remote_prefix;

	FILE *current_file;
} path_maps_parser_state;


static void path_maps_parser_state_ctor(struct path_maps_parser_state *state)
{
	state->current_file = NULL;
	state->current_local_prefix = NULL;
	state->current_remote_prefix = NULL;
}

static void path_maps_parser_state_dtor(struct path_maps_parser_state *state)
{
	if (state->current_file) {
		fclose(state->current_file);
		state->current_file = NULL;
	}
	if (state->current_local_prefix) {
		free(state->current_local_prefix);
		state->current_local_prefix = NULL;
	}
	if (state->current_remote_prefix) {
		free(state->current_remote_prefix);
		state->current_remote_prefix = NULL;
	}
}

static FILE *path_maps_parser_open_file(const char *filename, int *error_code, char **error_message)
{
	FILE *tmp;

	tmp = fopen(filename, "r");

	if (!tmp) {
		*error_code = PATH_MAPS_CANT_OPEN_FILE;
		*error_message = strdup("Can't open file");
		return NULL;
	}

	return tmp;
}


/* Parses a path mapping file.
 *
 * Returns true on success, with the 'maps' updated.
 * Returns false on error, with an error set in 'error_message' and no modifications to 'maps'.
 */
bool xdebug_path_maps_parse_file(xdebug_path_maps *maps, const char *filename, int *error_code, char **error_message)
{
	struct path_maps_parser_state state;

	path_maps_parser_state_ctor(&state);

	state.current_file = path_maps_parser_open_file(filename, error_code, error_message);
	if (!state.current_file) {
		goto error;
	}

	path_maps_parser_state_dtor(&state);
	return true;

error:
	path_maps_parser_state_dtor(&state);
	return false;
}
