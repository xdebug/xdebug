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

#include "../hash.h"
#include "../mm.h"
#include "../str.h"
#include "../trim.h"
#include "../xdebug_strndup.h"

typedef struct path_maps_parser_state {
	int   error_code;
	int   error_line;
	char *error_message;

	char *current_local_prefix;
	char *current_remote_prefix;

	FILE *current_file;
	size_t current_line_no;

	xdebug_hash *file_rules;
} path_maps_parser_state;

static void path_maps_parser_state_ctor(struct path_maps_parser_state *state)
{
	state->error_code = PATH_MAPS_OK;
	state->error_message = NULL;

	state->current_file = NULL;
	state->current_line_no = 0;
	state->current_local_prefix = NULL;
	state->current_remote_prefix = NULL;

	state->file_rules = xdebug_hash_alloc(256, xdebug_path_mapping_dtor);
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
	state->error_line = state->current_line_no;
	if (state->error_message) {
		xdfree(state->error_message);
	}
	state->error_message = xdstrdup(error_message);
}

static int state_get_error_code(path_maps_parser_state *state)
{
	return state->error_code;
}

static int state_get_error_line(path_maps_parser_state *state)
{
	return state->error_line;
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

static bool is_valid_prefix(path_maps_parser_state *state, const char *prefix)
{
	if (prefix[0] == '\0') {
		state_set_error(state, PATH_MAPS_INVALID_PREFIX, "Prefix is empty");
		return false;
	}

	if (prefix[0] != '/') {
		char *message = xdebug_sprintf("Prefix is not an absolute path: '%s'", prefix);
		state_set_error(state, PATH_MAPS_INVALID_PREFIX, message);
		xdfree(message);
		return false;
	}

	return true;
}

static bool state_set_remote_prefix(path_maps_parser_state *state, const char *prefix)
{
	char *trimmed_prefix = xdebug_trim(prefix);

	if (!is_valid_prefix(state, trimmed_prefix)) {
		xdfree(trimmed_prefix);
		return false;
	}

	state->current_remote_prefix = trimmed_prefix;

	return true;
}

static bool state_set_local_prefix(path_maps_parser_state *state, const char *prefix)
{
	char *trimmed_prefix = xdebug_trim(prefix);

	if (!is_valid_prefix(state, trimmed_prefix)) {
		xdfree(trimmed_prefix);
		return false;
	}

	state->current_local_prefix = trimmed_prefix;

	return true;
}

const char *mapping_type_as_string[] = {
	"unknown",
	"directory",
	"file",
	"line-range"
};

enum map_element {
	REMOTE,
	LOCAL
};

const char *element_name_as_string[] = {
	"Remote",
	"Local"
};

static bool has_double_separator(path_maps_parser_state *state, const char *prefix, const char *path, map_element element)
{
	size_t prefix_len = prefix ? strlen(prefix) : 0;

	if (path[0] == '/' && prefix && prefix_len > 0 && prefix[prefix_len - 1] == '/') {
		char *message = xdebug_sprintf("%s prefix ends with separator ('%s') and mapping line begins with separator ('%s')",
			element_name_as_string[element], prefix, path);
		state_set_error(state, PATH_MAPS_DOUBLE_SEPARATOR, message);
		xdfree(message);
		return true;
	}

	return false;
}

/* returns true if there is no range, or a valid range; false if something is wrong */
static bool extract_line_range(path_maps_parser_state *state, const char *element, int *element_length, int *begin, int *end, map_element element_type)
{
	/* range: :(\d+)(-\d+)?
	   :4-20
	   :20
	 */
	const char *colon = NULL;
	const char *minus = NULL;

	/* find : */
	colon = strrchr(element, ':');

	if (!colon) {
		/* no range */
		return true;
	}

	/* if the colon is at the start, reject */
	if (colon == element) {
		char *message = xdebug_sprintf("%s element: Element only contains a range, but no path: '%s'",
			element_name_as_string[element_type],
			colon
		);

		state_set_error(state, PATH_MAPS_WRONG_RANGE, message);

		xdfree(message);
		return false;
	}

	/* if the colon follows a directory separator, reject */
	if (colon > element && (colon[-1] == '/')) {
		char *message = xdebug_sprintf("%s element: Ranges are not supported with directories: '%s'",
			element_name_as_string[element_type],
			element
		);

		state_set_error(state, PATH_MAPS_WRONG_RANGE, message);

		xdfree(message);
		return false;
	}

	/* find - */
	minus = strchr(colon, '-');
	if (!minus) {
		/* no end, whole section after colon should be a number */
		char *end_ptr;
		int lineno;

		lineno = strtol(colon + 1, &end_ptr, 10);
		if (*end_ptr != '\0') { /* something else followed the number */
			char *message = xdebug_sprintf("%s element: Non-number found as range: '%s'",
				element_name_as_string[element_type],
				colon
			);

			state_set_error(state, PATH_MAPS_WRONG_RANGE, message);

			xdfree(message);
			return false;
		}
		*element_length = colon - element;
		*begin = lineno;
		*end = lineno;

		return true;
	}

	return false;
}

static xdebug_path_map_element* prepare_remote_element(path_maps_parser_state *state, const char *buffer, const char *equals)
{
	xdebug_path_map_element *remote_element = xdebug_path_map_element_ctor();
	char       *remote_part = xdstrndup(buffer, equals - buffer - 1);
	char       *trimmed = xdebug_trim(remote_part);
	int         trimmed_length = strlen(trimmed), begin = 0, end = 0;

	if (has_double_separator(state, state->current_remote_prefix, trimmed, REMOTE)) {
		goto failure;
	}

	remote_element->type = XDEBUG_PATH_MAP_TYPE_FILE;

	if (!extract_line_range(state, trimmed, &trimmed_length, &begin, &end, REMOTE)) {
		goto failure;
	}

	if (begin > 0) {
		remote_element->type = XDEBUG_PATH_MAP_TYPE_LINES;
		remote_element->begin = begin;
		remote_element->end   = end;
	}

	if (trimmed[trimmed_length - 1] == '/') {
		remote_element->type = XDEBUG_PATH_MAP_TYPE_DIRECTORY;
	}

	/* Assemble */
	if (state->current_remote_prefix) {
		remote_element->path = xdebug_str_create_from_char(state->current_remote_prefix);
	} else {
		remote_element->path = xdebug_str_new();
	}

	xdebug_str_addl(remote_element->path, trimmed, trimmed_length, false);

	/* clean up */
	xdfree(trimmed);
	xdfree(remote_part);

	return remote_element;

failure:
	xdebug_path_map_element_dtor(remote_element);
	xdfree(trimmed);
	xdfree(remote_part);
	return NULL;
}

static xdebug_path_map_element* prepare_local_element(path_maps_parser_state *state, const char *equals)
{
	xdebug_path_map_element *local_element = xdebug_path_map_element_ctor();
	char       *trimmed = xdebug_trim(equals + 1);
	int         trimmed_length = strlen(trimmed), begin = 0, end = 0;

	if (has_double_separator(state, state->current_local_prefix, trimmed, LOCAL)) {
		goto failure;
	}

	local_element->type = XDEBUG_PATH_MAP_TYPE_FILE;

	if (!extract_line_range(state, trimmed, &trimmed_length, &begin, &end, LOCAL)) {
		goto failure;
	}

	if (begin > 0) {
		local_element->type = XDEBUG_PATH_MAP_TYPE_LINES;
		local_element->begin = begin;
		local_element->end   = end;
	}

	if (trimmed[trimmed_length - 1] == '/') {
		local_element->type = XDEBUG_PATH_MAP_TYPE_DIRECTORY;
	}

	if (state->current_local_prefix) {
		local_element->path = xdebug_str_create_from_char(state->current_local_prefix);
	} else {
		local_element->path = xdebug_str_new();
	}

	xdebug_str_addl(local_element->path, trimmed, trimmed_length, false);

	/* clean up */
	xdfree(trimmed);

	return local_element;

failure:
	xdebug_path_map_element_dtor(local_element);
	xdfree(trimmed);
	return NULL;
}

static bool state_add_rule(path_maps_parser_state *state, const char *buffer, const char *equals)
{
	xdebug_path_mapping* tmp = (xdebug_path_mapping*) xdcalloc(1, sizeof(xdebug_path_mapping));
	xdebug_path_map_element *remote_path = NULL;
	xdebug_path_map_element *local_path  = NULL;

	/* remote part */
	remote_path = prepare_remote_element(state, buffer, equals);
	if (!remote_path) {
		goto failure;
	}

	/* local part */
	local_path = prepare_local_element(state, equals);
	if (!local_path) {
		goto failure;
	}

	/* - if remote path is a directory type, then local needs to be one too */
	if (remote_path->type != local_path->type) {
		char *message = xdebug_sprintf("Remote mapping part ('%s') type (%s) must match local mapping part ('%s') type (%s)",
			XDEBUG_STR_VAL(remote_path->path), mapping_type_as_string[remote_path->type],
			XDEBUG_STR_VAL(local_path->path), mapping_type_as_string[local_path->type]);
		state_set_error(state, PATH_MAPS_MISMATCHED_TYPES, message);
		xdfree(message);

		goto failure;
	}

	/* assign */
	tmp->remote = remote_path;
	tmp->local  = local_path;

	xdebug_hash_add(state->file_rules, XDEBUG_STR_VAL(remote_path->path), XDEBUG_STR_LEN(remote_path->path), tmp);

	return true;

failure:
	if (remote_path) {
		xdebug_path_map_element_dtor(remote_path);
	}
	if (local_path) {
		xdebug_path_map_element_dtor(local_path);
	}
	xdebug_path_mapping_dtor(tmp);
	return false;
}

static bool state_file_read_lines(path_maps_parser_state *state)
{
	char   buffer[2048];
	state->current_line_no = 0;

	while (fgets(buffer, sizeof(buffer), state->current_file) != NULL) {
		size_t  buffer_len;
		char   *equals;

		++state->current_line_no;

		/* if last char is no \n, abort */
		buffer_len = strlen(buffer);

		/* no data at all, which shouldn't happend as fgets should have returned NULL */
		if (buffer_len == 0) {
			state_set_error(state, PATH_MAPS_EMPTY_LINE, "Empty line, which shouldn't be possible");
			return false;
		}

		/* All lines must end in \n */
		if (buffer[buffer_len - 1] != '\n') {
			state_set_error(state, PATH_MAPS_NO_NEWLINE, "Line does not end in a new line");
			return false;
		}

		/* empty line, ignore */
		if (buffer_len == 1) {
			continue;
		}

		/* stanzas */
		if (strncmp(buffer, "remote_prefix:", strlen("remote_prefix:")) == 0) {
			if (!state_set_remote_prefix(state, buffer + strlen("remote_prefix:"))) {
				return false;
			}
			continue;
		}
		if (strncmp(buffer, "local_prefix:", strlen("local_prefix:")) == 0) {
			if (!state_set_local_prefix(state, buffer + strlen("local_prefix:"))) {
				return false;
			}
			continue;
		}

		/* comments */
		if (buffer[0] == '#') {
			continue;
		}

		/* assignments */
		equals = strchr(buffer, '=');
		if (!equals) {
			state_set_error(state, PATH_MAPS_GARBAGE, "Line is not empty, and has no stanza, assignment, or starts with '#'");
			return false;
		}

		if (!state_add_rule(state, buffer, equals)) {
			return false;
		}
	}

	return true;
}

static xdebug_path_map_element* copy_element(xdebug_path_map_element *from)
{
	xdebug_path_map_element *tmp = xdebug_path_map_element_ctor();

	tmp->type  = from->type;
	tmp->path  = xdebug_str_copy(from->path);
	tmp->begin = from->begin;
	tmp->end   = from->end;

	return tmp;
}

static void copy_rule(void *ret, xdebug_hash_element *e)
{
	xdebug_path_mapping *new_rule = (xdebug_path_mapping*) e->ptr;
	xdebug_path_mapping *tmp = (xdebug_path_mapping*) xdmalloc(sizeof(xdebug_path_mapping));

	tmp->remote = copy_element(new_rule->remote);
	tmp->local =  copy_element(new_rule->local);

	xdebug_hash_add((xdebug_hash*) ret, e->key.value.str.val, e->key.value.str.len, tmp);
}

/* Parses a path mapping file.
 *
 * Returns true on success, with the 'maps' updated.
 * Returns false on error, with an error set in 'error_message' and no modifications to 'maps'.
 */
bool xdebug_path_maps_parse_file(xdebug_path_maps *maps, const char *filename, int *error_code, int *error_line, char **error_message)
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
	*error_line = state_get_error_line(&state);
	*error_message = state_get_error_message(&state);
	path_maps_parser_state_dtor(&state);
	return false;
}
