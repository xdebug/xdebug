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
#ifndef XDEBUG_TRACE_FLAMEGRAPH_H
#define XDEBUG_TRACE_FLAMEGRAPH_H

#include "tracing_private.h"

typedef struct _flamegraph_function
{
	xdebug_str *prefix;
	int         value;
} flamegraph_function;

typedef struct _xdebug_trace_flamegraph_context
{
	xdebug_file *trace_file;
	int          mode;
	xdebug_hash *functions;
} xdebug_trace_flamegraph_context;

extern xdebug_trace_handler_t xdebug_trace_handler_flamegraph_cost;
extern xdebug_trace_handler_t xdebug_trace_handler_flamegraph_mem;
#endif
