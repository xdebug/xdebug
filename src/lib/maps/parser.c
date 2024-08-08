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

#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "maps_private.h"
#include "parser.h"

#define XDEBUG_NO_PHP_FEATURES

#include "../hash.h"
#include "../mm.h"
#include "../str.h"
#include "../trim.h"
#include "../xdebug_strndup.h"

typedef struct path_maps_parser_state {
	int   error_code;
	char *error_message;

	char *current_local_prefix;
	char *current_remote_prefix;

	FILE *current_file;

	xdebug_hash *file_rules;
} path_maps_parser_state;

static void path_maps_parser_state_ctor(struct path_maps_parser_state *state)
{
	state->error_code = PATH_MAPS_OK;
	state->error_message = NULL;

	state->current_file = NULL;
	state->current_local_prefix = NULL;
	state->current_remote_prefix = NULL;

	state->file_rules = xdebug_hash_alloc(256, xdebug_path_mapping_free);
}

static void path_maps_parser_state_dtor(struct path_maps_parser_state *state)
{
	state->error_code = PATH_MAPS_OK;
	if (state->error_message) {
		xdfree(state->error_message);
	}
	if (state->current_local_prefix) {
		xdfree(state->current_local_prefix);
		state->current_local_prefix = NULL;
	}
	if (state->current_remote_prefix) {
		xdfree(state->current_remote_prefix);
		state->current_remote_prefix = NULL;
	}
	if (state->current_file) {
		fclose(state->current_file);
		state->current_file = NULL;
	}
	if (state->file_rules) {
		xdebug_hash_destroy(state->file_rules);
	}
}

static void state_set_error(path_maps_parser_state *state, int error_code, const char *error_message)
{
	state->error_code = error_code;
	if (state->error_message) {
		xdfree(state->error_message);
	}
	state->error_message = xdstrdup(error_message);
}

static int state_get_error_code(path_maps_parser_state *state)
{
	return state->error_code;
}

static char *state_get_error_message(path_maps_parser_state *state)
{
	return xdstrdup(state->error_message);
}

static bool state_open_file(path_maps_parser_state *state, const char *filename)
{
	FILE *tmp = fopen(filename, "r");

	if (!tmp) {
		state_set_error(state, PATH_MAPS_CANT_OPEN_FILE, "Can't open file");
		return false;
	}

	state->current_file = tmp;
	return true;
}

static void state_set_remote_prefix(path_maps_parser_state *state, const char *prefix)
{
	state->current_remote_prefix = xdebug_trim(prefix);
}

static void state_set_local_prefix(path_maps_parser_state *state, const char *prefix)
{
	state->current_local_prefix = xdebug_trim(prefix);
}

static bool state_add_rule(path_maps_parser_state *state, const char *buffer, const char *equals)
{
	xdebug_path_mapping* tmp = (xdebug_path_mapping*) xdcalloc(1, sizeof(xdebug_path_mapping));
	xdebug_str remote_path = XDEBUG_STR_INITIALIZER;
	xdebug_str local_path  = XDEBUG_STR_INITIALIZER;

	if (state->current_remote_prefix) {
		xdebug_str_add(&remote_path, state->current_remote_prefix, false);
	}
	{
		char *remote_part = xdstrndup(buffer, equals - buffer - 1);
		char *trimmed = xdebug_trim(remote_part);

		xdebug_str_add(&remote_path, trimmed, false);

		xdfree(trimmed);
		xdfree(remote_part);
	}

	if (state->current_local_prefix) {
		xdebug_str_add(&local_path, state->current_local_prefix, false);
	}
	{
		char *trimmed = xdebug_trim(equals + 1);

		xdebug_str_add(&local_path, trimmed, false);

		xdfree(trimmed);
	}

	tmp->remote_path = remote_path.d;
	tmp->local_path = local_path.d;
	tmp->type = XDEBUG_PATH_MAP_TYPE_DIRECTORY;

	xdebug_hash_add(state->file_rules, remote_path.d, remote_path.l, tmp);
	return true;
}

static bool state_file_read_lines(path_maps_parser_state *state)
{
	char   buffer[2048];
	size_t current_line_no = 0;

	while (fgets(buffer, sizeof(buffer), state->current_file) != NULL) {
		size_t  buffer_len;
		char   *equals;

		++current_line_no;

		/* if last char is no \n, abort */
		buffer_len = strlen(buffer);

		/* no data at all */
		if (buffer_len == 0) {
			state_set_error(state, PATH_MAPS_EMPTY_LINE, "Line XXX is empty, that shouldn't be possible");
			return false;
		}

		/* All lines must end in \n */
		if (buffer[buffer_len - 1] != '\n') {
			state_set_error(state, PATH_MAPS_NO_NEWLINE, "Line XXX does not end in a new line");
			return false;
		}

		/* empty line, ignore */
		if (buffer_len == 1) {
			continue;
		}

		/* stanzas */
		if (strncmp(buffer, "remote_prefix:", strlen("remote_prefix:")) == 0) {
			state_set_remote_prefix(state, buffer + strlen("remote_prefix:"));
			continue;
		}
		if (strncmp(buffer, "local_prefix:", strlen("local_prefix:")) == 0) {
			state_set_local_prefix(state, buffer + strlen("local_prefix:"));
			continue;
		}

		/* comments */
		if (buffer[0] == '#') {
			continue;
		}

		/* assignments */
		equals = strchr(buffer, '=');
		if (!equals) {
			state_set_error(state, PATH_MAPS_GARBAGE, "Line XXX is not empty and has no stanza, assignment, or starts with '#'");
			return false;
		}

		state_add_rule(state, buffer, equals);
	}

	return true;
}

static void copy_rule(void *ret, xdebug_hash_element *e)
{
	xdebug_path_mapping *new_rule = (xdebug_path_mapping*) e->ptr;
	xdebug_path_mapping *tmp = (xdebug_path_mapping*) xdmalloc(sizeof(xdebug_path_mapping));

	tmp->local_path = xdstrdup(new_rule->local_path);
	tmp->remote_path = xdstrdup(new_rule->remote_path);
	tmp->type = new_rule->type;

	xdebug_hash_add((xdebug_hash*) ret, e->key.value.str.val, e->key.value.str.len, tmp);
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

	if (!state_open_file(&state, filename)) {
		goto error;
	}

	if (!state_file_read_lines(&state)) {
		goto error;
	}

	if (state.file_rules->size == 0) {
		state_set_error(&state, PATH_MAPS_NO_RULES, "The map file did not provide any mappings");
		goto error;
	}

	/* No errors, so copy all rules to actual map */
	xdebug_hash_apply(state.file_rules, (void*) maps->remote_to_local_map, copy_rule);

	path_maps_parser_state_dtor(&state);
	return true;

error:
	*error_code = state_get_error_code(&state);
	*error_message = state_get_error_message(&state);
	path_maps_parser_state_dtor(&state);
	return false;
}
