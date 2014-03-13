/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2014 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <derick@xdebug.org>                          |
   +----------------------------------------------------------------------+
 */

#ifndef __HAVE_XDEBUG_BRANCH_INFO_H__
#define __HAVE_XDEBUG_BRANCH_INFO_H__

#include "xdebug_set.h"

typedef struct _xdebug_branch {
	unsigned int start_lineno;
	unsigned int end_lineno;
	unsigned int end_op;
	unsigned int out[2];
} xdebug_branch;

typedef struct _xdebug_path {
	unsigned int elements_count;
	unsigned int elements_size;
	unsigned int *elements;
} xdebug_path;

typedef struct _xdebug_branch_info {
	unsigned int  size;
	xdebug_set      *starts;
	xdebug_set      *ends;
	xdebug_branch   *branches;

	unsigned int  paths_count;
	unsigned int  paths_size;
	xdebug_path    **paths;
} xdebug_branch_info;

xdebug_branch_info *xdebug_branch_info_create(unsigned int size);

void xdebug_branch_info_update(xdebug_branch_info *branch_info, unsigned int pos, unsigned int lineno, unsigned int outidx, unsigned int jump_pos);
void xdebug_branch_post_process(xdebug_branch_info *branch_info);
void xdebug_branch_find_paths(xdebug_branch_info *branch_info);

void xdebug_branch_info_dump(zend_op_array *opa, xdebug_branch_info *branch_info TSRMLS_DC);
void xdebug_branch_info_add_branches_and_paths(char *filename, xdebug_branch_info *branch_info TSRMLS_DC);
void xdebug_branch_info_free(xdebug_branch_info *branch_info);

#endif
