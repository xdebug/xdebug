/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2023 Derick Rethans                               |
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

#include "usefulstuff.h"
#include "cmd_parser.h"

/* {{{ Constants for state machine */
#define STATE_NORMAL                   0
#define STATE_QUOTED                   1
#define STATE_OPT_FOLLOWS              2
#define STATE_SEP_FOLLOWS              3
#define STATE_VALUE_FOLLOWS_FIRST_CHAR 4
#define STATE_VALUE_FOLLOWS            5
#define STATE_SKIP_CHAR                6
#define STATE_ESCAPED_CHAR_FOLLOWS     7
/* }}} */

void xdebug_cmd_arg_dtor(xdebug_dbgp_arg *arg)
{
	int i;

	for (i = 0; i < 27; i++) {
		if (arg->value[i]) {
			xdebug_str_free(arg->value[i]);
		}
	}
	xdfree(arg);
}

int xdebug_cmd_parse(const char *line, char **cmd, xdebug_dbgp_arg **ret_args)
{
	xdebug_dbgp_arg *args = NULL;
	char *ptr;
	int   state;
	int   charescaped = 0;
	char  opt = ' ', *value_begin = NULL;

	args = xdmalloc(sizeof (xdebug_dbgp_arg));
	memset(args->value, 0, sizeof(args->value));
	*cmd = NULL;

	/* Find the end of the command, this is always on the first space */
	ptr = strchr(line, ' ');
	if (!ptr) {
		/* No space found. If the line is not empty, return the line
		 * and assume it only consists of the command name. If the line
		 * is 0 chars long, we return a failure. */
		if (strlen(line)) {
			*cmd = strdup(line);
			*ret_args = args;
			return XDEBUG_ERROR_OK;
		} else {
			goto parse_error;
		}
	} else {
		/* A space was found, so we copy everything before it
		 * into the cmd parameter. */
		*cmd = xdcalloc(1, ptr - line + 1);
		memcpy(*cmd, line, ptr - line);
	}
	/* Now we loop until we find the end of the string, which is the \0
	 * character */
	state = STATE_NORMAL;
	do {
		ptr++;
		switch (state) {
			case STATE_NORMAL:
				if (*ptr != '-') {
					goto parse_error;
				} else {
					state = STATE_OPT_FOLLOWS;
				}
				break;
			case STATE_OPT_FOLLOWS:
				opt = *ptr;
				state = STATE_SEP_FOLLOWS;
				break;
			case STATE_SEP_FOLLOWS:
				if (*ptr != ' ') {
					goto parse_error;
				} else {
					state = STATE_VALUE_FOLLOWS_FIRST_CHAR;
					value_begin = ptr + 1;
				}
				break;
			case STATE_VALUE_FOLLOWS_FIRST_CHAR:
				if (*ptr == '"' && opt != '-') {
					value_begin = ptr + 1;
					state = STATE_QUOTED;
				} else {
					state = STATE_VALUE_FOLLOWS;
				}
				break;
			case STATE_VALUE_FOLLOWS:
				if ((*ptr == ' ' && opt != '-') || *ptr == '\0') {
					int opt_index = opt - 'a';

					if (opt == '-') {
						opt_index = 26;
					}

					if (!args->value[opt_index]) {
						args->value[opt_index] = xdebug_str_create(value_begin, ptr - value_begin);
						state = STATE_NORMAL;
					} else {
						goto duplicate_opts;
					}
				}
				break;
			case STATE_QUOTED:
				if (*ptr == '\\') {
					state = STATE_ESCAPED_CHAR_FOLLOWS;
				} else
				if (*ptr == '"') {
					int opt_index = opt - 'a';

					if (charescaped) {
						charescaped = 0;
						break;
					}
					if (opt == '-') {
						opt_index = 26;
					}

					if (!args->value[opt_index]) {
						int len = ptr - value_begin;

						args->value[opt_index] = xdebug_str_create(value_begin, len);
						xdebug_stripcslashes(args->value[opt_index]->d, &len);
						args->value[opt_index]->l = len;

						state = STATE_SKIP_CHAR;
					} else {
						goto duplicate_opts;
					}
				}
				break;
			case STATE_SKIP_CHAR:
				state = STATE_NORMAL;
				break;
			case STATE_ESCAPED_CHAR_FOLLOWS:
				state = STATE_QUOTED;
				break;

		}
	} while (*ptr);
	*ret_args = args;
	return XDEBUG_ERROR_OK;

parse_error:
	*ret_args = args;
	return XDEBUG_ERROR_PARSE;

duplicate_opts:
	*ret_args = args;
	return XDEBUG_ERROR_DUP_ARG;
}

