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
   | Authors: Derick Rethans <derick@xdebug.org>                          |
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

#define XDEBUG_BRANCH_MAX_OUTS 64

typedef struct _xdebug_branch {
	unsigned int  start_lineno;
	unsigned int  end_lineno;
	unsigned int  end_op;
	unsigned char hit;
	unsigned int  outs_count;
	int           outs[XDEBUG_BRANCH_MAX_OUTS];
	unsigned char outs_hit[XDEBUG_BRANCH_MAX_OUTS];
} xdebug_branch;

typedef struct _xdebug_path {
	unsigned int elements_count;
	unsigned int elements_size;
	unsigned int *elements;
	unsigned char hit;
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

	xdebug_path_info path_info; /* The paths that can be created out of these branches */
} xdebug_branch_info;

xdebug_branch_info *xdebug_branch_info_create(unsigned int size);

void xdebug_branch_info_update(xdebug_branch_info *branch_info, unsigned int pos, unsigned int lineno, unsigned int outidx, unsigned int jump_pos);
void xdebug_branch_post_process(zend_op_array *opa, xdebug_branch_info *branch_info);
void xdebug_branch_find_paths(xdebug_branch_info *branch_info);

void xdebug_branch_info_dump(zend_op_array *opa, xdebug_branch_info *branch_info);
void xdebug_branch_info_add_branches_and_paths(char *filename, char *function_name, xdebug_branch_info *branch_info);
void xdebug_branch_info_free(xdebug_branch_info *branch_info);

xdebug_path *xdebug_path_new(xdebug_path *old_path);
void xdebug_path_free(xdebug_path *path);

xdebug_path_info *xdebug_path_info_ctor(void);
void xdebug_path_info_dtor(xdebug_path_info *path_info);

void xdebug_path_info_add_path_for_level(xdebug_path_info *path_info, xdebug_path *path, unsigned int level);
xdebug_path *xdebug_path_info_get_path_for_level(xdebug_path_info *path_info, unsigned int level);

void xdebug_create_key_for_path(xdebug_path *path, xdebug_str *str);

void xdebug_branch_info_mark_reached(char *filename, char *function_name, zend_op_array *op_array, long opcode_nr);
void xdebug_branch_info_mark_end_of_function_reached(char *filename, char *function_name, char *key, int key_len);
#endif
