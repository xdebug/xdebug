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

#ifndef __HAVE_CMD_PARSER_H__
#define __HAVE_CMD_PARSER_H__

#include "php_xdebug.h"
#include "src/lib/usefulstuff.h"

/* Argument structure */
typedef struct xdebug_dbgp_arg {
	xdebug_str *value[27]; /* one extra for - */
} xdebug_dbgp_arg;

#define CMD_OPTION_SET(opt)        (!!(opt == '-' ? args->value[26] : args->value[(opt) - 'a']))
#define CMD_OPTION_CHAR(opt)       (opt == '-' ? args->value[26]->d : args->value[(opt) - 'a']->d)
#define CMD_OPTION_LEN(opt)        (opt == '-' ? args->value[26]->l : args->value[(opt) - 'a']->l)
#define CMD_OPTION_XDEBUG_STR(opt) (opt == '-' ? args->value[26] : args->value[(opt) - 'a'])

void xdebug_cmd_arg_dtor(xdebug_dbgp_arg *arg);
int xdebug_cmd_parse(const char *line, char **cmd, xdebug_dbgp_arg **ret_args);

#define XDEBUG_ERROR_OK                              0
#define XDEBUG_ERROR_PARSE                           1
#define XDEBUG_ERROR_DUP_ARG                         2
#define XDEBUG_ERROR_INVALID_ARGS                    3
#define XDEBUG_ERROR_UNIMPLEMENTED                   4
#define XDEBUG_ERROR_COMMAND_UNAVAILABLE             5

#define XDEBUG_ERROR_STEP_DEBUG_MODE_NOT_ENABLED   400
#define XDEBUG_ERROR_STEP_DEBUG_NOT_STARTED        401

#endif
