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

	bool  copy_error;

	char *current_local_prefix;
	char *current_remote_prefix;

	char *current_working_directory;
	FILE *current_file;
	size_t current_line_no;

	xdebug_hash *file_rules;
} path_maps_parser_state;

static void path_maps_parser_state_ctor(struct path_maps_parser_state *state)
{
	state->error_code = PATH_MAPS_OK;
	state->error_message = NULL;

	state->copy_error = false;

	state->current_file = NULL;
	state->current_line_no = 0;
	state->current_local_prefix = NULL;
	state->current_remote_prefix = NULL;
	state->current_working_directory = NULL;

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
	if (state->current_working_directory) {
		xdfree(state->current_working_directory);
		state->current_working_directory = NULL;
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

	/* Relative prefix */
	if (prefix[0] == '.') {
		/* Valid, starting with ./ */
		if (prefix[1] != '/') {
			char *message = xdebug_sprintf("Prefix is not a valid relative path: '%s'", prefix);
			state_set_error(state, PATH_MAPS_INVALID_PREFIX, message);
			xdfree(message);
			return false;
		}

		return true;
	}

	if (prefix[0] != '/') {
		char *message = xdebug_sprintf("Prefix is not an absolute or relative path: '%s'", prefix);
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
	"line-range",
	"skip"
};

enum map_element {
	REMOTE,
	LOCAL
};

const char *element_name_as_string[] = {
	"Remote",
	"Local"
};

static bool has_no_separator(path_maps_parser_state *state, const char *prefix, const char *path, enum map_element element)
{
	size_t prefix_len = prefix ? strlen(prefix) : 0;

	if (path[0] != '/' && prefix && prefix_len > 0 && prefix[prefix_len - 1] != '/') {
		char *message = xdebug_sprintf("%s prefix ('%s') does not end with a separator, and mapping line does not begin with a separator ('%s')",
			element_name_as_string[element], prefix, path);
		state_set_error(state, PATH_MAPS_NO_SEPARATOR, message);
		xdfree(message);
		return true;
	}

	return false;
}

static bool has_double_separator(path_maps_parser_state *state, const char *prefix, const char *path, enum map_element element)
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
static bool extract_line_range(path_maps_parser_state *state, const char *element, int *element_length, int *begin, int *end, enum map_element element_type)
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

		if (lineno < 1) {
			char *message = xdebug_sprintf("%s element: Line number much be larger than 0: '%s'",
				element_name_as_string[element_type],
				element
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

	/* check whether the minus doesn't immediately follow the colon */
	if (minus == colon + 1) {
		char *message = xdebug_sprintf("%s element: The starting line number must be provided: '%s'",
			element_name_as_string[element_type],
			element
		);

		state_set_error(state, PATH_MAPS_WRONG_RANGE, message);

		xdfree(message);
		return false;
	}

	/* we should have the begin line number between colon+1 and minus-1, and the end line number beteen minus+1...\0 */
	{
		char *end_ptr;
		int begin_lineno, end_lineno;

		begin_lineno = strtol(colon + 1, &end_ptr, 10);

		if (*end_ptr != '-') { /* something else followed the number, and not the expected - */
			char *message = xdebug_sprintf("%s element: Non-number found as begin range: '%s'",
				element_name_as_string[element_type],
				colon
			);

			state_set_error(state, PATH_MAPS_WRONG_RANGE, message);

			xdfree(message);
			return false;
		}

		end_lineno = strtol(minus + 1, &end_ptr, 10);
		if (*end_ptr != '\0') { /* something else followed the number */
			char *message = xdebug_sprintf("%s element: Non-number found as end range: '%s'",
				element_name_as_string[element_type],
				colon
			);

			state_set_error(state, PATH_MAPS_WRONG_RANGE, message);

			xdfree(message);
			return false;
		}

		if (end_lineno < begin_lineno) {
			char *message = xdebug_sprintf("%s element: End of range (%d) is before start of range (%d): '%s'",
				element_name_as_string[element_type],
				begin_lineno, end_lineno,
				colon
			);

			state_set_error(state, PATH_MAPS_WRONG_RANGE, message);

			xdfree(message);
			return false;
		}

		*element_length = colon - element;
		*begin = begin_lineno;
		*end = end_lineno;

		return true;
	}

	state_set_error(state, PATH_MAPS_WRONG_RANGE, "Unknown error in extracting line range");
	return false;
}

static xdebug_str* prepare_remote_element(path_maps_parser_state *state, const char *buffer, const char *equals, int *type, int *remote_begin, int *remote_end)
{
	xdebug_str *remote_path;
	char       *remote_part = xdstrndup(buffer, equals - buffer);
	char       *trimmed = xdebug_trim(remote_part);
	int         trimmed_length = strlen(trimmed), begin = 0, end = 0;

	if (trimmed_length < 1) {
		state_set_error(state, PATH_MAPS_GARBAGE, "Remote part is empty");
		goto failure;
	}

	if (has_no_separator(state, state->current_remote_prefix, trimmed, REMOTE)) {
		goto failure;
	}
	if (has_double_separator(state, state->current_remote_prefix, trimmed, REMOTE)) {
		goto failure;
	}

	*type = XDEBUG_PATH_MAP_TYPE_FILE;

	if (!extract_line_range(state, trimmed, &trimmed_length, &begin, &end, REMOTE)) {
		goto failure;
	}

	if (begin > 0) {
		*type = XDEBUG_PATH_MAP_TYPE_LINES;
		*remote_begin = begin;
		*remote_end   = end;
	}

	if (trimmed[trimmed_length - 1] == '/') {
		*type = XDEBUG_PATH_MAP_TYPE_DIRECTORY;
	}

	/* Assemble */
	if (state->current_remote_prefix) {
		if (state->current_remote_prefix[0] == '.') {
			remote_path = xdebug_str_create_from_char(state->current_working_directory);
			xdebug_str_add(remote_path, state->current_remote_prefix + 1, false);
		} else {
			remote_path = xdebug_str_create_from_char(state->current_remote_prefix);
		}
	} else {
		remote_path = xdebug_str_new();
	}

	if (trimmed[0] == '.' && trimmed[1] == '/') {
		xdebug_str_add(remote_path, state->current_working_directory, false);
		xdebug_str_addl(remote_path, trimmed + 1, trimmed_length - 1, false);
	} else {
		xdebug_str_addl(remote_path, trimmed, trimmed_length, false);
	}

	/* clean up */
	xdfree(trimmed);
	xdfree(remote_part);

	return remote_path;

failure:
	xdfree(trimmed);
	xdfree(remote_part);
	return NULL;
}

static xdebug_str* prepare_local_element(path_maps_parser_state *state, const char *equals, int *type, int *local_begin, int *local_end)
{
	xdebug_str *local_path;
	char       *trimmed = xdebug_trim(equals + 1);
	int         trimmed_length = strlen(trimmed), begin = 0, end = 0;

	if (trimmed_length < 1) {
		state_set_error(state, PATH_MAPS_GARBAGE, "Local part is empty");
		goto failure;
	}

	if (strcmp("SKIP", trimmed) == 0) {
		*type = XDEBUG_PATH_MAP_FLAGS_SKIP;
		*local_begin = -1;
		*local_end = -1;
		local_path = NULL;

		goto cleanup;
	}

	if (has_no_separator(state, state->current_local_prefix, trimmed, LOCAL)) {
		goto failure;
	}
	if (has_double_separator(state, state->current_local_prefix, trimmed, LOCAL)) {
		goto failure;
	}

	*type = XDEBUG_PATH_MAP_TYPE_FILE;

	if (!extract_line_range(state, trimmed, &trimmed_length, &begin, &end, LOCAL)) {
		goto failure;
	}

	if (begin > 0) {
		*type = XDEBUG_PATH_MAP_TYPE_LINES;
		*local_begin = begin;
		*local_end   = end;
	}

	if (trimmed[trimmed_length - 1] == '/') {
		*type = XDEBUG_PATH_MAP_TYPE_DIRECTORY;
	}

	/* Assemble */
	if (state->current_local_prefix) {
		if (state->current_local_prefix[0] == '.') {
			local_path = xdebug_str_create_from_char(state->current_working_directory);
			xdebug_str_add(local_path, state->current_local_prefix + 1, false);
		} else {
			local_path = xdebug_str_create_from_char(state->current_local_prefix);
		}
	} else {
		local_path = xdebug_str_new();
	}

	if (trimmed[0] == '.' && trimmed[1] == '/') {
		xdebug_str_add(local_path, state->current_working_directory, false);
		xdebug_str_addl(local_path, trimmed + 1, trimmed_length - 1, false);
	} else {
		xdebug_str_addl(local_path, trimmed, trimmed_length, false);
	}

cleanup:
	/* clean up */
	xdfree(trimmed);

	return local_path;

failure:
	*type = XDEBUG_PATH_MAP_TYPE_UNKNOWN;
	xdfree(trimmed);
	return NULL;
}

static bool state_add_rule(path_maps_parser_state *state, const char *buffer, const char *equals)
{
	xdebug_path_mapping *existing_path_mapping = NULL;
	xdebug_str *remote_path = NULL;
	xdebug_str *local_path  = NULL;
	int remote_type = XDEBUG_PATH_MAP_TYPE_UNKNOWN;
	int local_type  = XDEBUG_PATH_MAP_TYPE_UNKNOWN;
	int remote_begin = 0;
	int remote_end = 0;
	int local_begin = 0;
	int local_end = 0;

	/* remote part */
	remote_path = prepare_remote_element(state, buffer, equals, &remote_type, &remote_begin, &remote_end);
	if (!remote_path) {
		goto failure;
	}

	/* local part */
	local_path = prepare_local_element(state, equals, &local_type, &local_begin, &local_end);
	if (local_type == XDEBUG_PATH_MAP_TYPE_UNKNOWN) {
		goto failure;
	}

	if (! (local_type & XDEBUG_PATH_MAP_FLAGS_SKIP)) {
		/* - the types of the remote and local types need to be the same */
		if (remote_type != local_type) {
			char *message = xdebug_sprintf("Remote mapping part ('%s') type (%s) must match local mapping part ('%s') type (%s)",
				XDEBUG_STR_VAL(remote_path), mapping_type_as_string[remote_type],
				XDEBUG_STR_VAL(local_path), mapping_type_as_string[local_type]);
			state_set_error(state, PATH_MAPS_MISMATCHED_TYPES, message);
			xdfree(message);

			goto failure;
		}

		/* - if local range is multiple lines, then remote range needs to be to, and the same difference */
		if (
			(remote_type == XDEBUG_PATH_MAP_TYPE_LINES && local_begin != local_end) &&
			((remote_end - remote_begin) != (local_end - local_begin))
		) {
			char *message = xdebug_sprintf("The remote range span (%d-%d) needs to have the same difference (%d) as the local range span (%d-%d) difference (%d)",
				remote_begin, remote_end,
				remote_end - remote_begin,
				local_begin, local_end,
				local_end - local_begin);
			state_set_error(state, PATH_MAPS_WRONG_RANGE, message);
			xdfree(message);

			goto failure;
		}
	}

	/* assign */
	if (xdebug_hash_find(state->file_rules, XDEBUG_STR_VAL(remote_path), XDEBUG_STR_LEN(remote_path), (void**) &existing_path_mapping)) {
		xdebug_path_map_range *tail_range, *new_range;

		/* If there is an existing path mapping for this `remote_path`, but it is not TYPE_LINES, then we shouldn't see the same
		 * DIRECTORY/FILE again, so this is an error */
		if (existing_path_mapping->type != XDEBUG_PATH_MAP_TYPE_LINES) {
			char *message = xdebug_sprintf("An entry for '%s' is already present, but it is not a line range entry. You can't mix them",
				remote_path);
			state_set_error(state, PATH_MAPS_MIXING_PATH_AND_LINES, message);
			xdfree(message);

			goto failure;
		}

		/* Check to make sure the local file paths are the same, and that the line numbers is incrementing */
		tail_range = (xdebug_path_map_range*) XDEBUG_VECTOR_TAIL(existing_path_mapping->m.line_ranges);

		if (
			!(local_type & XDEBUG_PATH_MAP_FLAGS_SKIP) &&
			!(tail_range->local_flags & XDEBUG_PATH_MAP_FLAGS_SKIP) &&
			!xdebug_str_is_equal(tail_range->local_path, local_path)
		) {
			char *message = xdebug_sprintf("The local path (%s) must match earlier local paths (%s) for the same remote path (%s)",
				XDEBUG_STR_VAL(local_path), XDEBUG_STR_VAL(tail_range->local_path), XDEBUG_STR_VAL(remote_path));
			state_set_error(state, PATH_MAPS_WRONG_RANGE, message);
			xdfree(message);

			goto failure;
		}

		if (!(local_type & XDEBUG_PATH_MAP_FLAGS_SKIP) && remote_begin <= tail_range->remote_end) {
			char *message = xdebug_sprintf("The remote range begin line (%d) must be higher than the previous range end line (%d)",
				remote_begin, tail_range->remote_end);
			state_set_error(state, PATH_MAPS_WRONG_RANGE, message);
			xdfree(message);

			goto failure;
		}

		if (!(local_type & XDEBUG_PATH_MAP_FLAGS_SKIP) && local_begin <= tail_range->local_end) {
			char *message = xdebug_sprintf("The local range begin line (%d) must be higher than the previous range end line (%d)",
				local_begin, tail_range->local_end);
			state_set_error(state, PATH_MAPS_WRONG_RANGE, message);
			xdfree(message);

			goto failure;
		}

		new_range = (xdebug_path_map_range*) xdebug_vector_push(existing_path_mapping->m.line_ranges);
		xdebug_path_map_range_set(new_range, remote_begin, remote_end, local_type & XDEBUG_PATH_MAP_FLAGS_MASK, local_path, local_begin, local_end);

		xdebug_str_free(remote_path);
		if (local_path) {
			xdebug_str_free(local_path);
		}
	} else {
		xdebug_path_mapping* tmp = xdebug_path_mapping_ctor(remote_type | (local_type & XDEBUG_PATH_MAP_FLAGS_MASK));
		xdebug_path_map_range *new_range;

		tmp->type = remote_type;
		tmp->remote_path = remote_path;

		if (remote_type == XDEBUG_PATH_MAP_TYPE_LINES) {
			new_range = (xdebug_path_map_range*) xdebug_vector_push(tmp->m.line_ranges);
			xdebug_path_map_range_set(new_range, remote_begin, remote_end, local_type & XDEBUG_PATH_MAP_FLAGS_MASK, local_path, local_begin, local_end);
		} else if (local_path) {
			tmp->m.local_path = xdebug_str_copy(local_path);
		}

		xdebug_hash_add(state->file_rules, XDEBUG_STR_VAL(remote_path), XDEBUG_STR_LEN(remote_path), tmp);

		if (local_path) {
			xdebug_str_free(local_path);
		}
	}

	return true;

failure:
	if (remote_path) {
		xdebug_str_free(remote_path);
	}
	if (local_path) {
		xdebug_str_free(local_path);
	}
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
		if (equals == buffer) {
			state_set_error(state, PATH_MAPS_GARBAGE, "Line starts with a '='");
			return false;
		}

		if (!state_add_rule(state, buffer, equals)) {
			return false;
		}
	}

	return true;
}

static void copy_rule(void *ret, xdebug_hash_element *e, void *state_v)
{
	struct path_maps_parser_state *state = (struct path_maps_parser_state*) state_v;
	xdebug_path_mapping *new_rule = (xdebug_path_mapping*) e->ptr;
	xdebug_path_mapping *existing_path_mapping = NULL;

	if (state->copy_error) {
		return;
	}

	if (xdebug_hash_find((xdebug_hash*) ret, XDEBUG_STR_VAL(new_rule->remote_path), XDEBUG_STR_LEN(new_rule->remote_path), (void**) &existing_path_mapping)) {
		xdebug_str *message = xdebug_str_create_from_const_char("Duplicate rules in multiple files for '");
		xdebug_str_addl(message, XDEBUG_STR_VAL(new_rule->remote_path), XDEBUG_STR_LEN(new_rule->remote_path), 0);
		xdebug_str_add_literal(message, "'");

		state_set_error(state, PATH_MAPS_DUPLICATE_RULES, message->d);
		state->copy_error = true;

		xdebug_str_free(message);
	} else {
		xdebug_path_mapping *tmp = xdebug_path_mapping_copy(new_rule);

		xdebug_hash_add((xdebug_hash*) ret, XDEBUG_STR_VAL(new_rule->remote_path), XDEBUG_STR_LEN(new_rule->remote_path), tmp);
	}
}

static void copy_rule_reverse(void *ret, xdebug_hash_element *e, void *state_v)
{
	struct path_maps_parser_state *state = (struct path_maps_parser_state*) state_v;
	xdebug_path_mapping *new_rule = (xdebug_path_mapping*) e->ptr;
	xdebug_path_mapping *existing_path_mapping = NULL;

	if (state->copy_error) {
		return;
	}

	if (new_rule->type & XDEBUG_PATH_MAP_FLAGS_SKIP) {
		return;
	}

	switch (new_rule->type & XDEBUG_PATH_MAP_TYPE_MASK) {
		case XDEBUG_PATH_MAP_TYPE_FILE:
		case XDEBUG_PATH_MAP_TYPE_DIRECTORY:
			if (xdebug_hash_find((xdebug_hash*) ret, XDEBUG_STR_VAL(new_rule->m.local_path), XDEBUG_STR_LEN(new_rule->m.local_path), (void**) &existing_path_mapping)) {
				xdebug_str *message = xdebug_str_create_from_const_char("Duplicate rules in multiple files for '");
				xdebug_str_addl(message, XDEBUG_STR_VAL(new_rule->m.local_path), XDEBUG_STR_LEN(new_rule->m.local_path), 0);
				xdebug_str_add_literal(message, "'");

				state_set_error(state, PATH_MAPS_DUPLICATE_RULES, message->d);
				state->copy_error = true;

				xdebug_str_free(message);
			} else {
				xdebug_path_mapping *tmp = xdebug_path_mapping_copy(new_rule);

				xdebug_hash_add((xdebug_hash*) ret, XDEBUG_STR_VAL(new_rule->m.local_path), XDEBUG_STR_LEN(new_rule->m.local_path), tmp);
			}
			break;

		case XDEBUG_PATH_MAP_TYPE_LINES: {
			size_t i;
			xdebug_path_map_range *range_cursor;
			xdebug_path_mapping *reverse_map;
			xdebug_str *last_path;

			if (!new_rule->m.line_ranges || XDEBUG_VECTOR_COUNT(new_rule->m.line_ranges) == 0) {
				return;
			}

			/* Alloc new vector */
			reverse_map = xdebug_path_mapping_ctor(XDEBUG_PATH_MAP_TYPE_LINES);

			/* Loop over each range and build up vector */
			range_cursor = (xdebug_path_map_range*) XDEBUG_VECTOR_HEAD(new_rule->m.line_ranges);

			for (i = 0; i < XDEBUG_VECTOR_COUNT(new_rule->m.line_ranges); i++, range_cursor++) {
				xdebug_path_map_range *reverse_range;

				if (range_cursor->local_flags & XDEBUG_PATH_MAP_FLAGS_SKIP) {
					continue;
				}

				reverse_range = (xdebug_path_map_range*) xdebug_vector_push(reverse_map->m.line_ranges);
				xdebug_path_map_range_set(reverse_range, range_cursor->remote_begin, range_cursor->remote_end, range_cursor->local_flags & XDEBUG_PATH_MAP_FLAGS_MASK, xdebug_str_copy(new_rule->remote_path), range_cursor->local_begin, range_cursor->local_end);
				last_path = range_cursor->local_path;
			}

			if (XDEBUG_VECTOR_COUNT(reverse_map->m.line_ranges) > 0) {
				xdebug_hash_add((xdebug_hash*) ret, XDEBUG_STR_VAL(last_path), XDEBUG_STR_LEN(last_path), reverse_map);
			}

			break;
		}
	}
}

/* Parses a path mapping file.
 *
 * Returns true on success, with the 'maps' updated.
 * Returns false on error, with an error set in 'error_message' and no modifications to 'maps'.
 */
bool xdebug_path_maps_parse_file(xdebug_path_maps *maps, const char *cwd, const char *filename, int *error_code, int *error_line, char **error_message)
{
	struct path_maps_parser_state state;

	path_maps_parser_state_ctor(&state);

	if (cwd) {
		state.current_working_directory = strdup(cwd);
	} else {
		int *p = NULL;
		*p = 0;
	}

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

	/* No errors, so copy all rules to remote-to-local map */
	state.copy_error = false;
	xdebug_hash_apply_with_argument(state.file_rules, (void*) maps->remote_to_local_map, copy_rule, (void*) &state);
	if (state.copy_error) {
		goto error;
	}

	/* No errors, so copy all rules to local-to-remote map */
	state.copy_error = false;
	xdebug_hash_apply_with_argument(state.file_rules, (void*) maps->local_to_remote_map, copy_rule_reverse, (void*) &state);
	if (state.copy_error) {
		goto error;
	}

	path_maps_parser_state_dtor(&state);
	return true;

error:
	*error_code = state_get_error_code(&state);
	*error_line = state_get_error_line(&state);
	*error_message = state_get_error_message(&state);
	path_maps_parser_state_dtor(&state);
	return false;
}
