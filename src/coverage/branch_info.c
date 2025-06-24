/*
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2025 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to the 2-Clause BSD license which is     |
   | available through the LICENSE file, or online at                     |
   | http://opensource.org/licenses/bsd-license.php                       |
   +----------------------------------------------------------------------+
 */
#include <stdlib.h>
#include <math.h>

#include "php_xdebug.h"
#include "code_coverage_private.h"

#include "lib/hash.h"
#include "lib/log.h"
#include "lib/str.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

xdebug_branch_info *xdebug_branch_info_create(unsigned int size)
{
	xdebug_branch_info *tmp;

	tmp = calloc(1, sizeof(xdebug_branch_info));
	tmp->size = size;
	tmp->branches = calloc(size, sizeof(xdebug_branch));
	tmp->entry_points = xdebug_set_create(size);
	tmp->starts       = xdebug_set_create(size);
	tmp->ends         = xdebug_set_create(size);

	tmp->path_info.paths_count = 0;
	tmp->path_info.paths_size  = 0;
	tmp->path_info.paths = NULL;
	tmp->highest_out = 0;

	return tmp;
}

void xdebug_branch_info_free(xdebug_branch_info *branch_info)
{
	unsigned int i;

	for (i = 0; i < branch_info->path_info.paths_count; i++) {
		free(branch_info->path_info.paths[i]->elements);
		free(branch_info->path_info.paths[i]);
	}
	free(branch_info->path_info.paths);
	xdebug_hash_destroy(branch_info->path_info.path_hash);
	free(branch_info->branches);
	xdebug_set_free(branch_info->entry_points);
	xdebug_set_free(branch_info->starts);
	xdebug_set_free(branch_info->ends);
	free(branch_info);
}

void xdebug_branch_info_update(xdebug_branch_info *branch_info, unsigned int pos, unsigned int lineno, unsigned int outidx, unsigned int jump_pos, bool soft_fail)
{
	xdebug_set_add(branch_info->ends, pos);
	if (outidx < XDEBUG_BRANCH_MAX_OUTS) {
		branch_info->branches[pos].outs[outidx] = jump_pos;
		if (outidx + 1 > branch_info->branches[pos].outs_count) {
			branch_info->branches[pos].outs_count = outidx + 1;
		}
	}
	branch_info->branches[pos].start_lineno = lineno;
	branch_info->branches[pos].soft_fail = soft_fail;
}

static void only_leave_first_catch(zend_op_array *opa, xdebug_branch_info *branch_info, int position)
{
	unsigned int exit_jmp;
#if ZEND_USE_ABS_JMP_ADDR
	zend_op *base_address = &(opa->opcodes[0]);
#endif

	if (opa->opcodes[position].opcode == ZEND_FETCH_CLASS) {
		position++;
	}

	if (opa->opcodes[position].opcode != ZEND_CATCH) {
		return;
	}

	xdebug_set_remove(branch_info->entry_points, position);

	if (opa->opcodes[position].extended_value & ZEND_LAST_CATCH) {
		return;
	}
	exit_jmp = XDEBUG_ZNODE_JMP_LINE(opa->opcodes[position].op2, position, base_address);

	if (opa->opcodes[exit_jmp].opcode == ZEND_FETCH_CLASS) {
		exit_jmp++;
	}
	if (opa->opcodes[exit_jmp].opcode == ZEND_CATCH) {
		only_leave_first_catch(opa, branch_info, exit_jmp);
	}
}

void xdebug_branch_post_process(zend_op_array *opa, xdebug_branch_info *branch_info)
{
	unsigned int i;
	int          in_branch = 0, last_start = -1;
#if ZEND_USE_ABS_JMP_ADDR
	zend_op *base_address = &(opa->opcodes[0]);
#endif

	/* Figure out which CATCHes are chained, and hence which ones should be
	 * considered entry points */
	for (i = 0; i < branch_info->entry_points->size; i++) {
		if (xdebug_set_in(branch_info->entry_points, i) && opa->opcodes[i].opcode == ZEND_CATCH) {
#if ZEND_USE_ABS_JMP_ADDR
# if PHP_VERSION_ID >= 80200
			if (opa->opcodes[i].op2.jmp_addr != (void*) -1) {
# else
			if (opa->opcodes[i].op2.jmp_addr != NULL) {
# endif
#else
			if (opa->opcodes[i].op2.jmp_offset != 0) {
#endif
				only_leave_first_catch(opa, branch_info, XDEBUG_ZNODE_JMP_LINE(opa->opcodes[i].op2, i, base_address));
			}
		}
	}

	for (i = 0; i < branch_info->starts->size; i++) {
		if (xdebug_set_in(branch_info->starts, i)) {
			if (in_branch) {
				branch_info->branches[last_start].outs_count = 1;
				branch_info->branches[last_start].outs[0] = i;
				branch_info->branches[last_start].end_op = i-1;
				branch_info->branches[last_start].end_lineno = branch_info->branches[i].start_lineno;
			}
			last_start = i;
			in_branch = 1;
		}
		if (xdebug_set_in(branch_info->ends, i)) {
			size_t j;

			for (j = 0; j < branch_info->branches[i].outs_count; j++) {
				branch_info->branches[last_start].outs[j] = branch_info->branches[i].outs[j];
			}
			branch_info->branches[last_start].outs_count = branch_info->branches[i].outs_count;
			branch_info->branches[last_start].end_op = i;
			branch_info->branches[last_start].end_lineno = branch_info->branches[i].start_lineno;
			in_branch = 0;
		}
	}
}

static void xdebug_path_add(xdebug_path *path, unsigned int nr)
{
	if (!path) {
		return;
	}
	if (path->elements_count == path->elements_size) {
		path->elements_size += 32;
		path->elements = realloc(path->elements, sizeof(unsigned int) * path->elements_size);
	}
	path->elements[path->elements_count] = nr;
	path->elements_count++;
}

static void xdebug_path_info_add_path(xdebug_path_info *path_info, xdebug_path *path)
{
	if (path_info->paths_count == path_info->paths_size) {
		path_info->paths_size += 32;
		path_info->paths = realloc(path_info->paths, sizeof(xdebug_path*) * path_info->paths_size);
	}
	path_info->paths[path_info->paths_count] = path;
	path_info->paths_count++;
}

static void xdebug_path_info_make_sure_level_exists(xdebug_path_info *path_info, unsigned int level)
{
	unsigned int i = 0, orig_size;

	orig_size = path_info->paths_size;

	if (level >= path_info->paths_size) {
		path_info->paths_size = level + 32;
		path_info->paths = realloc(path_info->paths, sizeof(xdebug_path*) * path_info->paths_size);

		for (i = orig_size; i < XG_COV(branches).size; i++) {
			XG_COV(branches).last_branch_nr[i] = -1;
		}

		for (i = orig_size; i < path_info->paths_size; i++) {
			path_info->paths[i] = NULL;
		}
	}
}

void xdebug_path_info_add_path_for_level(xdebug_path_info *path_info, xdebug_path *path, unsigned int level)
{
	xdebug_path_info_make_sure_level_exists(path_info, level);
	path_info->paths[level] = path;
}

xdebug_path *xdebug_path_info_get_path_for_level(xdebug_path_info *path_info, unsigned int level)
{
	xdebug_path_info_make_sure_level_exists(path_info, level);
	return path_info->paths[level];
}

xdebug_path *xdebug_path_new(xdebug_path *old_path)
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

void xdebug_path_free(xdebug_path *path)
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
	unsigned int last;
	xdebug_path *new_path;
	int found = 0;
	size_t i = 0;

	if (branch_info->path_info.paths_count >= XDEBUG_MAX_PATHS) {
		return;
	}

	new_path = xdebug_path_new(prev_path);
	xdebug_path_add(new_path, nr);

	last = xdebug_branch_find_last_element(new_path);

	for (i = 0; i < branch_info->branches[nr].outs_count; i++) {
		int out = branch_info->branches[nr].outs[i];
		if (out != 0 && out != XDEBUG_JMP_EXIT && !xdebug_path_exists(new_path, last, out)) {
			xdebug_branch_find_path(out, branch_info, new_path);
			found = 1;
		}
	}

	if (found) {
		xdebug_path_free(new_path);
		return;
	}

	xdebug_path_info_add_path(&(branch_info->path_info), new_path);
}

xdebug_path_info *xdebug_path_info_ctor(void)
{
	xdebug_path_info *tmp;

	tmp = xdmalloc(sizeof(xdebug_path_info));
	tmp->paths_count = 0;
	tmp->paths_size = 0;
	tmp->paths = NULL;
	tmp->path_hash = NULL;

	return tmp;
}

void xdebug_path_info_dtor(xdebug_path_info *path_info)
{
	unsigned int i;

	for (i = 0; i < path_info->paths_count; i++) {
		xdebug_path_free(path_info->paths[i]);
	}
	xdfree(path_info->paths);
	path_info->paths = NULL;
	if (path_info->path_hash) {
		xdebug_hash_destroy(path_info->path_hash);
		path_info->path_hash = NULL;
	}

	xdfree(path_info);
}

void xdebug_create_key_for_path(xdebug_path *path, xdebug_str *str)
{
	unsigned int i;
	char temp_nr[16];

	for (i = 0; i < path->elements_count; i++) {
		snprintf(temp_nr, 15, "%u:", path->elements[i]);
		xdebug_str_add(str, temp_nr, 0);
	}
}

void xdebug_branch_find_paths(xdebug_branch_info *branch_info)
{
	unsigned int i;

	for (i = 0; i < branch_info->entry_points->size; i++) {
		if (xdebug_set_in(branch_info->entry_points, i)) {
			xdebug_branch_find_path(i, branch_info, NULL);
		}
	}

	branch_info->path_info.path_hash = xdebug_hash_alloc(128, NULL);

	for (i = 0; i < branch_info->path_info.paths_count; i++) {
		xdebug_str str = XDEBUG_STR_INITIALIZER;
		xdebug_create_key_for_path(branch_info->path_info.paths[i], &str);
		xdebug_hash_add(branch_info->path_info.path_hash, str.d, str.l, branch_info->path_info.paths[i]);
		xdfree(str.d);
	}
}

static int fetch_mark_file(zend_string *filename, xdebug_coverage_file **file)
{
	if (XG_COV(previous_mark_filename) && zend_string_equals(XG_COV(previous_mark_filename), filename)) {
		*file = XG_COV(previous_mark_file);
		return 1;
	}

	if (!xdebug_hash_find(XG_COV(info), ZSTR_VAL(filename), ZSTR_LEN(filename), (void *) file)) {
		return 0;
	}

	if (XG_COV(previous_mark_filename)) {
		zend_string_release(XG_COV(previous_mark_filename));
	}
	XG_COV(previous_mark_filename) = zend_string_copy((*file)->name);
	XG_COV(previous_mark_file) = *file;

	return 1;
}

void xdebug_branch_info_mark_reached(zend_string *filename, char *function_name, zend_op_array *op_array, long opcode_nr)
{
	xdebug_coverage_file *file;
	xdebug_coverage_analysis_function *analysis_function = NULL;
	xdebug_coverage_runtime_function  *runtime_function;
	xdebug_branch_info *branch_info;

	/* Find the information for the given filename */
	if (!fetch_mark_file(filename, &file)) {
		return;
	}

	/* If there is no branch info, we don't have to do more */
	if (!file->has_branch_info) {
		return;
	}

	/* Check if the function already exists in the hash */
	if (!xdebug_hash_find(file->analysis.functions, function_name, strlen(function_name), (void *) &analysis_function)) {
		return;
	}

	/* Find the runtime function, and if it does not exist, create it */
	if (!xdebug_hash_find(file->runtime.functions, function_name, strlen(function_name), (void *) &runtime_function)) {
		runtime_function = xdebug_coverage_runtime_function_ctor(function_name, analysis_function);
		xdebug_hash_add(file->runtime.functions, function_name, strlen(function_name), runtime_function);
	}

	branch_info = analysis_function->branch_info;

	if (opcode_nr != 0 && xdebug_set_in(branch_info->entry_points, opcode_nr)) {
		xdebug_code_coverage_end_of_function(op_array, filename, function_name);
		xdebug_code_coverage_start_of_function(op_array, function_name);
	}

	if (xdebug_set_in(branch_info->starts, opcode_nr)) {
		char *key;
		void *dummy;
		function_stack_entry *tail_fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));

		/* Mark out for previous branch, if one is set */
		if (XG_COV(branches).last_branch_nr[XDEBUG_VECTOR_COUNT(XG_BASE(stack))] != -1) {
			size_t i = 0;

			for (i = 0; i < branch_info->branches[XG_COV(branches).last_branch_nr[XDEBUG_VECTOR_COUNT(XG_BASE(stack))]].outs_count; i++) {
				if (branch_info->branches[XG_COV(branches).last_branch_nr[XDEBUG_VECTOR_COUNT(XG_BASE(stack))]].outs[i] == opcode_nr) {
//printf("\nOUT HIT: %p (%d) (*%ld) %ld\n", runtime_function->hit_branch, XG_COV(branches).last_branch_nr[XDEBUG_VECTOR_COUNT(XG_BASE(stack))], branch_info->highest_out, i + 1);
					xdebug_set_add(runtime_function->hit_branch, (XG_COV(branches).last_branch_nr[XDEBUG_VECTOR_COUNT(XG_BASE(stack))] * (1 + branch_info->highest_out)) + i + 1);
				}
			}
		}

		key = xdebug_sprintf("%d:%d:%d", opcode_nr, XG_COV(branches).last_branch_nr[XDEBUG_VECTOR_COUNT(XG_BASE(stack))], tail_fse->function_nr);

		if (!xdebug_hash_find(XG_COV(visited_branches), key, strlen(key), (void*) &dummy)) {
			xdebug_path_add(XG_COV(paths_stack)->paths[XDEBUG_VECTOR_COUNT(XG_BASE(stack))], opcode_nr);
			xdebug_hash_add(XG_COV(visited_branches), key, strlen(key), NULL);
		}
		xdfree(key);

//printf("\nIN  HIT: %p (%ld) (*%ld) %d\n", runtime_function->hit_branch, opcode_nr, branch_info->highest_out, 0);
		xdebug_set_add(runtime_function->hit_branch, opcode_nr * (1 + branch_info->highest_out));

		XG_COV(branches).last_branch_nr[XDEBUG_VECTOR_COUNT(XG_BASE(stack))] = opcode_nr;
	}
}

/* TODO: Store paths hit differently */
void xdebug_branch_info_mark_end_of_function_reached(zend_string *filename, char *function_name, char *key, int key_len)
{
	xdebug_coverage_file *file;
	xdebug_coverage_analysis_function *analysis_function;
	xdebug_coverage_runtime_function  *runtime_function;
	xdebug_path *path;

	/* Find the information for the given filename */
	if (!fetch_mark_file(filename, &file)) {
		return;
	}

	/* If there is no branch info, we don't have to do more */
	if (!file->has_branch_info) {
		return;
	}

	/* Check if the function has been analysed, and hence exists in the hash */
	if (!xdebug_hash_find(file->analysis.functions, function_name, strlen(function_name), (void *) &analysis_function)) {
		return;
	}

	/* Check whether we know something about the current path */
	if (!xdebug_hash_find(analysis_function->branch_info->path_info.path_hash, key, key_len, (void *) &path)) {
		return;
	}

	/* Find the runtime function, and if it does not exist, create it */
	if (!xdebug_hash_find(file->runtime.functions, function_name, strlen(function_name), (void *) &runtime_function)) {
		runtime_function = xdebug_coverage_runtime_function_ctor(function_name, analysis_function);
		xdebug_hash_add(file->runtime.functions, function_name, strlen(function_name), runtime_function);
	}

	xdebug_hash_add(runtime_function->hit_paths, key, key_len, NULL);
}

void xdebug_branch_info_add_branches_and_paths(xdebug_coverage_file *file, char *function_name, xdebug_branch_info *branch_info)
{
	xdebug_coverage_analysis_function *analysis_function;

	/* TODO: Check if creating things here is actually useful */
	/* Check if the function already exists in the hash */
	if (!xdebug_hash_find(file->analysis.functions, function_name, strlen(function_name), (void *) &analysis_function)) {
		/* The function does not exist, so we add it to the hash */
		analysis_function = xdebug_coverage_analysis_function_ctor(function_name);

		xdebug_hash_add(file->analysis.functions, function_name, strlen(function_name), analysis_function);
	}

	if (branch_info) {
		file->has_branch_info = 1;
	}
	analysis_function->branch_info = branch_info;
}
