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

#ifndef __HAVE_XDEBUG_BRANCH_INFO_H__
#define __HAVE_XDEBUG_BRANCH_INFO_H__

#include "zend_compile.h"

#include "lib/hash.h"
#include "lib/set.h"
#include "lib/str.h"

#if ZEND_USE_ABS_JMP_ADDR
# define XDEBUG_ZNODE_JMP_LINE(node, opline, base)  (int32_t)(((long)((node).jmp_addr) - (long)(base_address)) / sizeof(zend_op))
#else
# define XDEBUG_ZNODE_JMP_LINE(node, opline, base)  (int32_t)(((int32_t)((node).jmp_offset) / sizeof(zend_op)) + (opline))
#endif

#define XDEBUG_JMP_NOT_SET (INT_MAX-1)
#define XDEBUG_JMP_EXIT    (INT_MAX-2)

#define XDEBUG_MAX_PATHS       4096
#define XDEBUG_BRANCH_MAX_OUTS   64

typedef struct _xdebug_branch {
	unsigned int  start_lineno;
	unsigned int  end_lineno;
	unsigned int  end_op;
	unsigned int  outs_count;
	int           outs[XDEBUG_BRANCH_MAX_OUTS];
} xdebug_branch;

typedef struct _xdebug_path {
	unsigned int elements_count;
	unsigned int elements_size;
	unsigned int *elements;
} xdebug_path;

/* Contains information for paths that belong to a set of branches (as stored in xdebug_branch_info) */
typedef struct _xdebug_path_info {
	unsigned int     paths_count; /* The number of collected paths */
	unsigned int     paths_size;  /* The amount of slots allocated for storing paths */
	xdebug_path    **paths;       /* An array of possible paths */
	xdebug_hash     *path_hash;   /* A hash where each path's key is the sequence of followed branches, pointing to a path in the paths array */
} xdebug_path_info;

/* Contains all the branch information for a specific function */
typedef struct _xdebug_branch_info {
	unsigned int     size;     /* The number of stored branches */
	xdebug_set      *entry_points; /* A set that contains all the entry points into the function */
	xdebug_set      *starts;   /* A set of opcodes nrs where each branch starts */
	xdebug_set      *ends;     /* A set of opcodes nrs where each ends starts */
	xdebug_branch   *branches; /* Information about each branch */
	size_t           highest_out;

	xdebug_path_info path_info; /* The paths that can be created out of these branches */
} xdebug_branch_info;
#endif
