/*
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2014 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to the 2-Clause BSD license which is     |
   | available through the LICENSE file, or online at                     |
   | http://opensource.org/licenses/bsd-license.php                       |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@derickrethans.nl>                   |
   +----------------------------------------------------------------------+
 */
#include <stdlib.h>
#include <math.h>
#include "php_xdebug.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

xdebug_branch_info *xdebug_branch_info_create(unsigned int size)
{
	xdebug_branch_info *tmp;

	tmp = calloc(1, sizeof(xdebug_branch_info));
	tmp->size = size;
	tmp->branches = calloc(size, sizeof(xdebug_branch));
	tmp->starts = xdebug_set_create(size);
	tmp->ends   = xdebug_set_create(size);

	tmp->paths_count = 0;
	tmp->paths_size  = 0;
	tmp->paths = NULL;

	return tmp;
}

void xdebug_branch_info_free(xdebug_branch_info *branch_info)
{
	unsigned int i;

	for (i = 0; i < branch_info->paths_count; i++) {
		free(branch_info->paths[i]->elements);
		free(branch_info->paths[i]);
	}
	free(branch_info->paths);
	free(branch_info->branches);
	xdebug_set_free(branch_info->starts);
	xdebug_set_free(branch_info->ends);
	free(branch_info);
}

void xdebug_branch_info_update(xdebug_branch_info *branch_info, unsigned int pos, unsigned int lineno, unsigned int outidx, unsigned int jump_pos)
{
	xdebug_set_add(branch_info->ends, pos);
	branch_info->branches[pos].out[outidx] = jump_pos;
	branch_info->branches[pos].start_lineno = lineno;
}

void xdebug_branch_post_process(xdebug_branch_info *branch_info)
{
	unsigned int i;
	int          in_branch = 0, last_start = -1;

	for (i = 0; i < branch_info->starts->size; i++) {
		if (xdebug_set_in(branch_info->starts, i)) {
			if (in_branch) {
				branch_info->branches[last_start].out[0] = i;
				branch_info->branches[last_start].end_op = i-1;
				branch_info->branches[last_start].end_lineno = branch_info->branches[i].start_lineno;
			}
			last_start = i;
			in_branch = 1;
		}
		if (xdebug_set_in(branch_info->ends, i)) {
			branch_info->branches[last_start].out[0] = branch_info->branches[i].out[0];
			branch_info->branches[last_start].out[1] = branch_info->branches[i].out[1];
			branch_info->branches[last_start].end_op = i;
			branch_info->branches[last_start].end_lineno = branch_info->branches[i].start_lineno;
			in_branch = 0;
		}
	}
}

static void xdebug_path_add(xdebug_path *path, unsigned int nr)
{
	if (path->elements_count == path->elements_size) {
		path->elements_size += 32;
		path->elements = realloc(path->elements, sizeof(unsigned int) * path->elements_size);
	}
	path->elements[path->elements_count] = nr;
	path->elements_count++;
}

static void xdebug_branch_info_add_path(xdebug_branch_info *branch_info, xdebug_path *path)
{
	if (branch_info->paths_count == branch_info->paths_size) {
		branch_info->paths_size += 32;
		branch_info->paths = realloc(branch_info->paths, sizeof(xdebug_path*) * branch_info->paths_size);
	}
	branch_info->paths[branch_info->paths_count] = path;
	branch_info->paths_count++;
}

static xdebug_path *xdebug_path_new(xdebug_path *old_path)
{
	xdebug_path *tmp;
	tmp = calloc(1, sizeof(xdebug_path));

	if (old_path) {
		unsigned i;

		for (i = 0; i < old_path->elements_count; i++) {
			xdebug_path_add(tmp, old_path->elements[i]);
		}
	}
	return tmp;
}

static void xdebug_path_free(xdebug_path *path)
{
	if (path->elements) {
		free(path->elements);
	}
	free(path);
}

static unsigned int xdebug_branch_find_last_element(xdebug_path *path)
{
	return path->elements[path->elements_count-1];
}

static int xdebug_path_exists(xdebug_path *path, unsigned int elem1, unsigned int elem2)
{
	unsigned int i;

	for (i = 0; i < path->elements_count - 1; i++) {
		if (path->elements[i] == elem1 && path->elements[i + 1] == elem2) {
			return 1;
		}
	}
	return 0;
}

static void xdebug_branch_find_path(unsigned int nr, xdebug_branch_info *branch_info, xdebug_path *prev_path)
{
	unsigned int out0, out1, last;
	xdebug_path *new_path;
	int found = 0;

	if (branch_info->paths_count > 65535) {
		return;
	}

	new_path = xdebug_path_new(prev_path);
	xdebug_path_add(new_path, nr);
	out0 = branch_info->branches[nr].out[0];
	out1 = branch_info->branches[nr].out[1];

	last = xdebug_branch_find_last_element(new_path);

	if (out0 != 0 && !xdebug_path_exists(new_path, last, out0)) {
		xdebug_branch_find_path(out0, branch_info, new_path);
		found = 1;
	}
	if (out1 != 0 && !xdebug_path_exists(new_path, last, out1)) {
		xdebug_branch_find_path(out1, branch_info, new_path);
		found = 1;
	}
	if (!found) {
		xdebug_branch_info_add_path(branch_info, new_path);
	} else {
		xdebug_path_free(new_path);
	}
}

void xdebug_branch_find_paths(xdebug_branch_info *branch_info)
{
	xdebug_branch_find_path(0, branch_info, NULL);
}

void xdebug_branch_info_dump(zend_op_array *opa, xdebug_branch_info *branch_info TSRMLS_DC)
{
	unsigned int i, j;

	for (i = 0; i < branch_info->starts->size; i++) {
		if (xdebug_set_in(branch_info->starts, i)) {
			printf("branch: #%3d; line: %5d-%5d; sop: %5d; eop: %5d",
				i,
				branch_info->branches[i].start_lineno,
				branch_info->branches[i].end_lineno,
				i,
				branch_info->branches[i].end_op
			);
			if (branch_info->branches[i].out[0]) {
				printf("; out1: %3d", branch_info->branches[i].out[0]);
			}
			if (branch_info->branches[i].out[1]) {
				printf("; out2: %3d", branch_info->branches[i].out[1]);
			}
			printf("\n");
		}
	}

	for (i = 0; i < branch_info->paths_count; i++) {
		printf("path #%d: ", i + 1);
		for (j = 0; j < branch_info->paths[i]->elements_count; j++) {
			printf("%d, ", branch_info->paths[i]->elements[j]);
		}
		printf("\n");
	}
}

void xdebug_branch_info_add_branches_and_paths(char *filename, xdebug_branch_info *branch_info TSRMLS_DC)
{
	xdebug_coverage_file *file;

	if (strcmp(XG(previous_filename), filename) == 0) {
		file = XG(previous_file);
	} else {
		/* Check if the file already exists in the hash */
		if (!xdebug_hash_find(XG(code_coverage), filename, strlen(filename), (void *) &file)) {
			/* The file does not exist, so we add it to the hash */
			file = xdebug_coverage_file_ctor(filename);

			xdebug_hash_add(XG(code_coverage), filename, strlen(filename), file);
		}
		XG(previous_filename) = file->name;
		XG(previous_file) = file;
	}

	file->branch_info = branch_info;
}
