/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2020 Derick Rethans                               |
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

#define XDEBUG_TRACE_FLAMEGRAPH_COST_FACTOR 100000;

#include "tracing_private.h"

typedef struct _flamegraph_stack_item
{
	char *prefix;
	int value;
	struct _flamegraph_stack_item *next;
} flamegraph_stack_item;

/* Implemented as a linked-list, but size will never be more than the current
   stack depth, so I guess this will not hurt performances too badly. */
typedef struct flamegraph_stack
{
	flamegraph_stack_item *head;
} flamegraph_stack;

typedef struct _xdebug_trace_flamegraph_context
{
	FILE *trace_file;
	char *trace_filename;
	int mode;
	flamegraph_stack *stack;
} xdebug_trace_flamegraph_context;

extern xdebug_trace_handler_t xdebug_trace_handler_flamegraph_cost;
extern xdebug_trace_handler_t xdebug_trace_handler_flamegraph_mem;
#endif
