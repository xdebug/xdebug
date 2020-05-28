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

#include "php_xdebug.h"
#include "zend_extensions.h"

#include "branch_info.h"
#include "code_coverage_private.h"

#include "base/filter.h"
#include "base/stack.h"
#include "lib/compat.h"
#include "lib/set.h"
#include "lib/var.h"
#include "tracing/tracing.h"

/* True globals */
int zend_xdebug_filter_offset = -1;
int zend_xdebug_cc_run_offset = -1;

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

static void xdebug_coverage_line_dtor(void *data)
{
	xdebug_coverage_line *line = (xdebug_coverage_line *) data;

	xdfree(line);
}

xdebug_coverage_file *xdebug_coverage_file_ctor(char *filename)
{
	xdebug_coverage_file *file;

	file = xdmalloc(sizeof(xdebug_coverage_file));
	file->name = xdstrdup(filename);
	file->lines = xdebug_hash_alloc(128, xdebug_coverage_line_dtor);
	file->functions = xdebug_hash_alloc(128, xdebug_coverage_function_dtor);
	file->has_branch_info = 0;

	return file;
}

static void xdebug_coverage_file_dtor(void *data)
{
	xdebug_coverage_file *file = (xdebug_coverage_file *) data;

	xdebug_hash_destroy(file->lines);
	xdebug_hash_destroy(file->functions);
	xdfree(file->name);
	xdfree(file);
}

xdebug_coverage_function *xdebug_coverage_function_ctor(char *function_name)
{
	xdebug_coverage_function *function;

	function = xdmalloc(sizeof(xdebug_coverage_function));
	function->name = xdstrdup(function_name);
	function->branch_info = NULL;

	return function;
}

void xdebug_coverage_function_dtor(void *data)
{
	xdebug_coverage_function *function = (xdebug_coverage_function *) data;

	if (function->branch_info) {
		xdebug_branch_info_free(function->branch_info);
	}
	xdfree(function->name);
	xdfree(function);
}

static char* xdebug_func_format(xdebug_func *func)
{
	switch (func->type) {
		case XFUNC_NORMAL:
			return xdstrdup(func->function);
		case XFUNC_MEMBER:
			return xdebug_sprintf("%s->%s", func->class, func->function);
		default:
			return xdstrdup("???");
	}
}

static void xdebug_print_opcode_info(char type, zend_execute_data *execute_data, const zend_op *cur_opcode)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	char *file = (char*) STR_NAME_VAL(op_array->filename);
	xdebug_func func_info;
	char *function_name;
	long opnr = execute_data->opline - execute_data->func->op_array.opcodes;

	xdebug_build_fname_from_oparray(&func_info, op_array);
	function_name = xdebug_func_format(&func_info);
	if (func_info.class) {
		xdfree(func_info.class);
	}
	if (func_info.function) {
		xdfree(func_info.function);
	}

	xdebug_branch_info_mark_reached(file, function_name, op_array, opnr);
	xdfree(function_name);
}

static int xdebug_check_branch_entry_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	const zend_op *cur_opcode = execute_data->opline;

	if (!op_array->reserved[XG_COV(code_coverage_filter_offset)] && XG_COV(code_coverage_active)) {
		xdebug_print_opcode_info('G', execute_data, cur_opcode);
	}

	return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}

static void xdebug_count_line(char *filename, int lineno, int executable, int deadcode)
{
	xdebug_coverage_file *file;
	xdebug_coverage_line *line;

	if (XG_COV(previous_filename) && strcmp(XG_COV(previous_filename), filename) == 0) {
		file = XG_COV(previous_file);
	} else {
		/* Check if the file already exists in the hash */
		if (!xdebug_hash_find(XG_COV(code_coverage_info), filename, strlen(filename), (void *) &file)) {
			/* The file does not exist, so we add it to the hash */
			file = xdebug_coverage_file_ctor(filename);

			xdebug_hash_add(XG_COV(code_coverage_info), filename, strlen(filename), file);
		}
		XG_COV(previous_filename) = file->name;
		XG_COV(previous_file) = file;
	}

	/* Check if the line already exists in the hash */
	if (!xdebug_hash_index_find(file->lines, lineno, (void *) &line)) {
		line = xdmalloc(sizeof(xdebug_coverage_line));
		line->lineno = lineno;
		line->count = 0;
		line->executable = 0;

		xdebug_hash_index_add(file->lines, lineno, line);
	}

	if (executable) {
		if (line->executable != 1 && deadcode) {
			line->executable = 2;
		} else {
			line->executable = 1;
		}
	} else {
		line->count++;
	}
}

int xdebug_common_override_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	const zend_op *cur_opcode = execute_data->opline;

	if (!op_array->reserved[XG_COV(code_coverage_filter_offset)] && XG_COV(code_coverage_active)) {
		int      lineno;
		char    *file;

		lineno = cur_opcode->lineno;
		file = (char*) STR_NAME_VAL(op_array->filename);

		xdebug_print_opcode_info('C', execute_data, cur_opcode);
		xdebug_count_line(file, lineno, 0, 0);
	}

	return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}

static void prefill_from_opcode(char *fn, zend_op opcode, int deadcode)
{
	if (
		opcode.opcode != ZEND_NOP &&
		opcode.opcode != ZEND_EXT_NOP &&
		opcode.opcode != ZEND_RECV &&
		opcode.opcode != ZEND_RECV_INIT
#if PHP_VERSION_ID < 70400
		&& opcode.opcode != ZEND_VERIFY_ABSTRACT_CLASS
		&& opcode.opcode != ZEND_ADD_INTERFACE
#endif
		&& opcode.opcode != ZEND_OP_DATA
		&& opcode.opcode != ZEND_TICKS
		&& opcode.opcode != ZEND_FAST_CALL
		&& opcode.opcode != ZEND_RECV_VARIADIC
	) {
		xdebug_count_line(fn, opcode.lineno, 1, deadcode);
	}
}

#define XDEBUG_ZNODE_ELEM(node,var) node.var

static int xdebug_find_jumps(zend_op_array *opa, unsigned int position, size_t *jump_count, int *jumps)
{
#if ZEND_USE_ABS_JMP_ADDR
	zend_op *base_address = &(opa->opcodes[0]);
#endif

	zend_op opcode = opa->opcodes[position];
	if (opcode.opcode == ZEND_JMP) {
		jumps[0] = XDEBUG_ZNODE_JMP_LINE(opcode.op1, position, base_address);
		*jump_count = 1;
		return 1;
	} else if (
		opcode.opcode == ZEND_JMPZ ||
		opcode.opcode == ZEND_JMPNZ ||
		opcode.opcode == ZEND_JMPZ_EX ||
		opcode.opcode == ZEND_JMPNZ_EX
	) {
		jumps[0] = position + 1;
		jumps[1] = XDEBUG_ZNODE_JMP_LINE(opcode.op2, position, base_address);
		*jump_count = 2;
		return 1;
	} else if (opcode.opcode == ZEND_JMPZNZ) {
		jumps[0] = XDEBUG_ZNODE_JMP_LINE(opcode.op2, position, base_address);
		jumps[1] = position + ((int32_t) opcode.extended_value / (int32_t) sizeof(zend_op));
		*jump_count = 2;
		return 1;
	} else if (opcode.opcode == ZEND_FE_FETCH_R || opcode.opcode == ZEND_FE_FETCH_RW) {
		jumps[0] = position + 1;
		jumps[1] = position + (opcode.extended_value / sizeof(zend_op));
		*jump_count = 2;
		return 1;
	} else if (opcode.opcode == ZEND_FE_RESET_R || opcode.opcode == ZEND_FE_RESET_RW) {
		jumps[0] = position + 1;
		jumps[1] = XDEBUG_ZNODE_JMP_LINE(opcode.op2, position, base_address);
		*jump_count = 2;
		return 1;
	} else if (opcode.opcode == ZEND_CATCH) {
		*jump_count = 2;
		jumps[0] = position + 1;
#if PHP_VERSION_ID >= 70300
		if (!(opcode.extended_value & ZEND_LAST_CATCH)) {
			jumps[1] = XDEBUG_ZNODE_JMP_LINE(opcode.op2, position, base_address);
#else
		if (!opcode.result.num) {
			jumps[1] = position + (opcode.extended_value / sizeof(zend_op));
#endif
			if (jumps[1] == jumps[0]) {
				jumps[1] = XDEBUG_JMP_NOT_SET;
				*jump_count = 1;
			}
		} else {
			jumps[1] = XDEBUG_JMP_EXIT;
		}
		return 1;
	} else if (opcode.opcode == ZEND_GOTO) {
		jumps[0] = XDEBUG_ZNODE_JMP_LINE(opcode.op1, position, base_address);
		*jump_count = 1;
		return 1;

	} else if (opcode.opcode == ZEND_FAST_CALL) {
		jumps[0] = XDEBUG_ZNODE_JMP_LINE(opcode.op1, position, base_address);
		jumps[1] = position + 1;
		*jump_count = 2;
		return 1;
	} else if (opcode.opcode == ZEND_FAST_RET) {
		jumps[0] = XDEBUG_JMP_EXIT;
		*jump_count = 1;
		return 1;
	} else if (
		opcode.opcode == ZEND_GENERATOR_RETURN ||
		opcode.opcode == ZEND_EXIT ||
		opcode.opcode == ZEND_THROW ||
		opcode.opcode == ZEND_RETURN
	) {
		jumps[0] = XDEBUG_JMP_EXIT;
		*jump_count = 1;
		return 1;
#if PHP_VERSION_ID >= 70200
	} else if (
		opcode.opcode == ZEND_SWITCH_LONG ||
		opcode.opcode == ZEND_SWITCH_STRING
	) {
		zval *array_value;
		HashTable *myht;
		zval *val;

# if PHP_VERSION_ID >= 70300
		array_value = RT_CONSTANT(&opa->opcodes[position], opcode.op2);
# else
		array_value = RT_CONSTANT_EX(opa->literals, opcode.op2);
# endif
		myht = Z_ARRVAL_P(array_value);

		/* All 'case' statements */
		ZEND_HASH_FOREACH_VAL_IND(myht, val) {
			if (*jump_count < XDEBUG_BRANCH_MAX_OUTS - 2) {
				jumps[*jump_count] = position + (val->value.lval / sizeof(zend_op));
				(*jump_count)++;
			}
		} ZEND_HASH_FOREACH_END();

		/* The 'default' case */
		jumps[*jump_count] = position + (opcode.extended_value / sizeof(zend_op));
		(*jump_count)++;

		/* The 'next' opcode */
		jumps[*jump_count] = position + 1;
		(*jump_count)++;

		return 1;
#endif
	}

	return 0;
}

static void xdebug_analyse_branch(zend_op_array *opa, unsigned int position, xdebug_set *set, xdebug_branch_info *branch_info)
{
	/* fprintf(stderr, "Branch analysis from position: %d\n", position); */

	if (branch_info) {
		xdebug_set_add(branch_info->starts, position);
		branch_info->branches[position].start_lineno = opa->opcodes[position].lineno;
	}

	/* First we see if the branch has been visited, if so we bail out. */
	if (xdebug_set_in(set, position)) {
		return;
	}

	/* fprintf(stderr, "XDEBUG Adding %d\n", position); */
	/* Loop over the opcodes until the end of the array, or until a jump point has been found */
	xdebug_set_add(set, position);
	while (position < opa->last) {
		size_t jump_count = 0;
		int    jumps[XDEBUG_BRANCH_MAX_OUTS];
		size_t i;

		/* See if we have a jump instruction */
		if (xdebug_find_jumps(opa, position, &jump_count, jumps)) {
			for (i = 0; i < jump_count; i++) {
				if (jumps[i] == XDEBUG_JMP_EXIT || jumps[i] != XDEBUG_JMP_NOT_SET) {
					if (branch_info) {
						xdebug_branch_info_update(branch_info, position, opa->opcodes[position].lineno, i, jumps[i]);
					}
					if (jumps[i] != XDEBUG_JMP_EXIT) {
						xdebug_analyse_branch(opa, jumps[i], set, branch_info);
					}
				}
			}
			break;
		}

		/* See if we have a throw instruction */
		if (opa->opcodes[position].opcode == ZEND_THROW) {
			/* fprintf(stderr, "Throw found at %d\n", position); */
			if (branch_info) {
				xdebug_set_add(branch_info->ends, position);
				branch_info->branches[position].start_lineno = opa->opcodes[position].lineno;
			}
			break;
		}

		/* See if we have an exit instruction */
		if (opa->opcodes[position].opcode == ZEND_EXIT) {
			/* fprintf(stderr, "X* Return found\n"); */
			if (branch_info) {
				xdebug_set_add(branch_info->ends, position);
				branch_info->branches[position].start_lineno = opa->opcodes[position].lineno;
			}
			break;
		}
		/* See if we have a return instruction */
		if (
			opa->opcodes[position].opcode == ZEND_RETURN
			|| opa->opcodes[position].opcode == ZEND_RETURN_BY_REF
		) {
			/*(fprintf(stderr, "XDEBUG Return found\n");)*/
			if (branch_info) {
				xdebug_set_add(branch_info->ends, position);
				branch_info->branches[position].start_lineno = opa->opcodes[position].lineno;
			}
			break;
		}

		position++;
		/*(fprintf(stderr, "XDEBUG Adding %d\n", position);)*/
		xdebug_set_add(set, position);
	}
}

static void xdebug_analyse_oparray(zend_op_array *opa, xdebug_set *set, xdebug_branch_info *branch_info)
{
	unsigned int position = 0;

	while (position < opa->last) {
		if (position == 0) {
			xdebug_analyse_branch(opa, position, set, branch_info);
			if (branch_info) {
				xdebug_set_add(branch_info->entry_points, position);
			}
		} else if (opa->opcodes[position].opcode == ZEND_CATCH) {
			xdebug_analyse_branch(opa, position, set, branch_info);
			if (branch_info) {
				xdebug_set_add(branch_info->entry_points, position);
			}
		}
		position++;
	}
	if (branch_info) {
		xdebug_set_add(branch_info->ends, opa->last-1);
		branch_info->branches[opa->last-1].start_lineno = opa->opcodes[opa->last-1].lineno;
	}
}

static void prefill_from_oparray(char *filename, zend_op_array *op_array)
{
	unsigned int i;
	xdebug_set *set = NULL;
	xdebug_branch_info *branch_info = NULL;

	op_array->reserved[XG_COV(dead_code_analysis_tracker_offset)] = (void*) XG_COV(dead_code_last_start_id);

	/* Check for abstract methods and simply return from this function in those
	 * cases. */
	if (op_array->fn_flags & ZEND_ACC_ABSTRACT) {
		return;
	}

	/* Check whether this function should be filtered out */
	{
/*
		function_stack_entry tmp_fse;
		tmp_fse.filename = STR_NAME_VAL(op_array->filename);
		xdebug_build_fname_from_oparray(&tmp_fse.function, op_array);
		printf("    - PREFIL FILTERED FOR %s (%s::%s): %s\n",
			tmp_fse.filename, tmp_fse.function.class, tmp_fse.function.function,
			op_array->reserved[XG_COV(code_coverage_filter_offset)] ? "YES" : "NO");
*/
		if (op_array->reserved[XG_COV(code_coverage_filter_offset)]) {
			return;
		}
	}

	/* Run dead code analysis if requested */
	if (XG_COV(code_coverage_dead_code_analysis) && (op_array->fn_flags & ZEND_ACC_DONE_PASS_TWO)) {
		set = xdebug_set_create(op_array->last);
		if (XG_COV(code_coverage_branch_check)) {
			branch_info = xdebug_branch_info_create(op_array->last);
		}

		xdebug_analyse_oparray(op_array, set, branch_info);
	}

	/* The normal loop then finally */
	for (i = 0; i < op_array->last; i++) {
		zend_op opcode = op_array->opcodes[i];
		prefill_from_opcode(filename, opcode, set ? !xdebug_set_in(set, i) : 0);
	}

	if (set) {
		xdebug_set_free(set);
	}
	if (branch_info) {
		char *function_name;
		xdebug_func func_info;

		xdebug_build_fname_from_oparray(&func_info, op_array);
		function_name = xdebug_func_format(&func_info);

		if (func_info.class) {
			xdfree(func_info.class);
		}
		if (func_info.function) {
			xdfree(func_info.function);
		}

		xdebug_branch_post_process(op_array, branch_info);
		xdebug_branch_find_paths(branch_info);
		xdebug_branch_info_add_branches_and_paths(filename, (char*) function_name, branch_info);

		xdfree(function_name);
	}
}

static int prefill_from_function_table(zend_op_array *opa)
{
	if (opa->type == ZEND_USER_FUNCTION) {
		if ((long) opa->reserved[XG_COV(dead_code_analysis_tracker_offset)] < XG_COV(dead_code_last_start_id)) {
			prefill_from_oparray((char*) STR_NAME_VAL(opa->filename), opa);
		}
	}

	return ZEND_HASH_APPLY_KEEP;
}

/* Set correct int format to use */
#if SIZEOF_ZEND_LONG == 4
# define XDEBUG_PTR_KEY_LEN 8
# define XDEBUG_PTR_KEY_FMT "%08X"
#else
# define XDEBUG_PTR_KEY_LEN 16
# define XDEBUG_PTR_KEY_FMT "%016lX"
#endif

static int prefill_from_class_table(zend_class_entry *ce)
{
	if (ce->type == ZEND_USER_CLASS) {
		zend_op_array *val;

		ZEND_HASH_FOREACH_PTR(&ce->function_table, val) {
			prefill_from_function_table(val);
		} ZEND_HASH_FOREACH_END();
	}

	return ZEND_HASH_APPLY_KEEP;
}

static void xdebug_prefill_code_coverage(zend_op_array *op_array)
{
	zend_op_array    *function_op_array;
	zend_class_entry *class_entry;

	if ((long) op_array->reserved[XG_COV(dead_code_analysis_tracker_offset)] < XG_COV(dead_code_last_start_id)) {
		prefill_from_oparray((char*) STR_NAME_VAL(op_array->filename), op_array);
	}

	ZEND_HASH_REVERSE_FOREACH_PTR(CG(function_table), function_op_array) {
		if (_idx == XG_COV(prefill_function_count)) {
			break;
		}
		prefill_from_function_table(function_op_array);
	} ZEND_HASH_FOREACH_END();
	XG_COV(prefill_function_count) = CG(function_table)->nNumUsed;

	ZEND_HASH_REVERSE_FOREACH_PTR(CG(class_table), class_entry) {
		if (_idx == XG_COV(prefill_class_count)) {
			break;
		}
		prefill_from_class_table(class_entry);
	} ZEND_HASH_FOREACH_END();
	XG_COV(prefill_class_count) = CG(class_table)->nNumUsed;
}

void xdebug_code_coverage_start_of_function(zend_op_array *op_array, char *function_name)
{
	xdebug_path *path = xdebug_path_new(NULL);

	xdebug_prefill_code_coverage(op_array);
	xdebug_path_info_add_path_for_level(XG_COV(paths_stack), path, XG_BASE(level));

	if (XG_COV(branches).size == 0 || XG_BASE(level) >= XG_COV(branches).size) {
		XG_COV(branches).size = XG_BASE(level) + 32;
		XG_COV(branches).last_branch_nr = realloc(XG_COV(branches).last_branch_nr, sizeof(int) * XG_COV(branches.size));
	}

	XG_COV(branches).last_branch_nr[XG_BASE(level)] = -1;
}

void xdebug_code_coverage_end_of_function(zend_op_array *op_array, char *file_name, char *function_name)
{
	xdebug_str str = XDEBUG_STR_INITIALIZER;
	xdebug_path *path = xdebug_path_info_get_path_for_level(XG_COV(paths_stack), XG_BASE(level));

	if (!path) {
		return;
	}

	xdebug_create_key_for_path(path, &str);

	xdebug_branch_info_mark_end_of_function_reached(file_name, function_name, str.d, str.l);

	xdfree(str.d);

	if (path) {
		xdebug_path_free(path);
	}
}

PHP_FUNCTION(xdebug_start_code_coverage)
{
	zend_long options = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &options) == FAILURE) {
		return;
	}
	XG_COV(code_coverage_unused) = (options & XDEBUG_CC_OPTION_UNUSED);
	XG_COV(code_coverage_dead_code_analysis) = (options & XDEBUG_CC_OPTION_DEAD_CODE);
	XG_COV(code_coverage_branch_check) = (options & XDEBUG_CC_OPTION_BRANCH_CHECK);

	if (!XINI_COV(enable)) {
		php_error(E_WARNING, "Code coverage needs to be enabled in php.ini by setting 'xdebug.coverage_enable' to '1'.");
		RETURN_FALSE;
	} else {
		XG_COV(code_coverage_active) = 1;
		RETURN_TRUE;
	}
}

PHP_FUNCTION(xdebug_stop_code_coverage)
{
	zend_long cleanup = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &cleanup) == FAILURE) {
		return;
	}
	if (XG_COV(code_coverage_active)) {
		if (cleanup) {
			XG_COV(previous_filename) = NULL;
			XG_COV(previous_file) = NULL;
			XG_COV(previous_mark_filename) = NULL;
			XG_COV(previous_mark_file) = NULL;
			xdebug_hash_destroy(XG_COV(code_coverage_info));
			XG_COV(code_coverage_info) = xdebug_hash_alloc(32, xdebug_coverage_file_dtor);
			XG_COV(dead_code_last_start_id)++;
			xdebug_path_info_dtor(XG_COV(paths_stack));
			XG_COV(paths_stack) = xdebug_path_info_ctor();
		}
		XG_COV(code_coverage_active) = 0;
		RETURN_TRUE;
	}
	RETURN_FALSE;
}


static int xdebug_lineno_cmp(const void *a, const void *b)
{
	Bucket *f = (Bucket *) a;
	Bucket *s = (Bucket *) b;

	if (f->h < s->h) {
		return -1;
	} else if (f->h > s->h) {
		return 1;
	} else {
		return 0;
	}
}


static void add_line(void *ret, xdebug_hash_element *e)
{
	xdebug_coverage_line *line = (xdebug_coverage_line*) e->ptr;
	zval                 *retval = (zval*) ret;

	if (line->executable && (line->count == 0)) {
		add_index_long(retval, line->lineno, -line->executable);
	} else {
		add_index_long(retval, line->lineno, 1);
	}
}

static void add_branches(zval *retval, xdebug_branch_info *branch_info)
{
	zval *branches, *branch, *out, *out_hit;
	unsigned int i;

	XDEBUG_MAKE_STD_ZVAL(branches);
	array_init(branches);

	for (i = 0; i < branch_info->starts->size; i++) {
		if (xdebug_set_in(branch_info->starts, i)) {
			size_t j = 0;

			XDEBUG_MAKE_STD_ZVAL(branch);
			array_init(branch);
			add_assoc_long(branch, "op_start", i);
			add_assoc_long(branch, "op_end", branch_info->branches[i].end_op);
			add_assoc_long(branch, "line_start", branch_info->branches[i].start_lineno);
			add_assoc_long(branch, "line_end", branch_info->branches[i].end_lineno);

			add_assoc_long(branch, "hit", branch_info->branches[i].hit);

			XDEBUG_MAKE_STD_ZVAL(out);
			array_init(out);
			for (j = 0; j < branch_info->branches[i].outs_count; j++) {
				if (branch_info->branches[i].outs[j]) {
					add_index_long(out, j, branch_info->branches[i].outs[j]);
				}
			}
			add_assoc_zval(branch, "out", out);

			XDEBUG_MAKE_STD_ZVAL(out_hit);
			array_init(out_hit);
			for (j = 0; j < branch_info->branches[i].outs_count; j++) {
				if (branch_info->branches[i].outs[j]) {
					add_index_long(out_hit, j, branch_info->branches[i].outs_hit[j]);
				}
			}
			add_assoc_zval(branch, "out_hit", out_hit);

			add_index_zval(branches, i, branch);
			efree(out_hit);
			efree(out);
			efree(branch);
		}
	}

	add_assoc_zval_ex(retval, "branches", HASH_KEY_SIZEOF("branches"), branches);

	efree(branches);
}

static void add_paths(zval *retval, xdebug_branch_info *branch_info)
{
	zval *paths, *path, *path_container;
	unsigned int i, j;

	XDEBUG_MAKE_STD_ZVAL(paths);
	array_init(paths);

	for (i = 0; i < branch_info->path_info.paths_count; i++) {
		XDEBUG_MAKE_STD_ZVAL(path);
		array_init(path);

		XDEBUG_MAKE_STD_ZVAL(path_container);
		array_init(path_container);

		for (j = 0; j < branch_info->path_info.paths[i]->elements_count; j++) {
			add_next_index_long(path, branch_info->path_info.paths[i]->elements[j]);
		}

		add_assoc_zval(path_container, "path", path);
		add_assoc_long(path_container, "hit", branch_info->path_info.paths[i]->hit);

		add_next_index_zval(paths, path_container);

		efree(path_container);
		efree(path);
	}

	add_assoc_zval_ex(retval, "paths", HASH_KEY_SIZEOF("paths"), paths);

	efree(paths);
}

static void add_cc_function(void *ret, xdebug_hash_element *e)
{
	xdebug_coverage_function *function = (xdebug_coverage_function*) e->ptr;
	zval                     *retval = (zval*) ret;
	zval                     *function_info;

	XDEBUG_MAKE_STD_ZVAL(function_info);
	array_init(function_info);

	if (function->branch_info) {
		add_branches(function_info, function->branch_info);
		add_paths(function_info, function->branch_info);
	}

	add_assoc_zval_ex(retval, function->name, HASH_KEY_STRLEN(function->name), function_info);

	efree(function_info);
}

static void add_file(void *ret, xdebug_hash_element *e)
{
	xdebug_coverage_file *file = (xdebug_coverage_file*) e->ptr;
	zval                 *retval = (zval*) ret;
	zval                 *lines, *functions, *file_info;
	HashTable            *target_hash;

	/* Add all the lines */
	XDEBUG_MAKE_STD_ZVAL(lines);
	array_init(lines);

	xdebug_hash_apply(file->lines, (void *) lines, add_line);

	/* Sort on linenumber */
	target_hash = HASH_OF(lines);
	zend_hash_sort(target_hash, xdebug_lineno_cmp, 0);

	/* Add the branch and path info */
	if (XG_COV(code_coverage_branch_check)) {
		XDEBUG_MAKE_STD_ZVAL(file_info);
		array_init(file_info);

		XDEBUG_MAKE_STD_ZVAL(functions);
		array_init(functions);

		xdebug_hash_apply(file->functions, (void *) functions, add_cc_function);

		add_assoc_zval_ex(file_info, "lines", HASH_KEY_SIZEOF("lines"), lines);
		add_assoc_zval_ex(file_info, "functions", HASH_KEY_SIZEOF("functions"), functions);

		add_assoc_zval_ex(retval, file->name, HASH_KEY_STRLEN(file->name), file_info);
		efree(functions);
		efree(file_info);
	} else {
		add_assoc_zval_ex(retval, file->name, HASH_KEY_STRLEN(file->name), lines);
	}

	efree(lines);
}

PHP_FUNCTION(xdebug_get_code_coverage)
{
	array_init(return_value);
	if (XG_COV(code_coverage_info)) {
		xdebug_hash_apply(XG_COV(code_coverage_info), (void *) return_value, add_file);
	}
}

PHP_FUNCTION(xdebug_get_function_count)
{
	RETURN_LONG(XG_BASE(function_count));
}

PHP_FUNCTION(xdebug_code_coverage_started)
{
	RETURN_BOOL(XG_COV(code_coverage_active));
}

void xdebug_init_coverage_globals(xdebug_coverage_globals_t *xg)
{
	xg->previous_filename    = NULL;
	xg->previous_file        = NULL;
	xg->previous_mark_filename = NULL;
	xg->previous_mark_file     = NULL;
	xg->paths_stack = NULL;
	xg->branches.size        = 0;
	xg->branches.last_branch_nr = NULL;
	xg->code_coverage_active = 0;

	/* Get reserved offset */
	xg->dead_code_analysis_tracker_offset = zend_xdebug_cc_run_offset;
	xg->dead_code_last_start_id = 1;
	xg->code_coverage_filter_offset = zend_xdebug_filter_offset;
}

void xdebug_coverage_count_line_if_active(zend_op_array *op_array, char *file, int lineno)
{
	if (XG_COV(code_coverage_active) && !op_array->reserved[XG_COV(code_coverage_filter_offset)]) {
		xdebug_count_line(file, lineno, 0, 0);
	}
}

void xdebug_coverage_count_line_if_branch_check_active(zend_op_array *op_array, char *file, int lineno)
{
	if (XG_COV(code_coverage_active) && XG_COV(code_coverage_branch_check)) {
		xdebug_coverage_count_line_if_active(op_array, file, lineno);
	}
}

void xdebug_coverage_record_assign_if_active(zend_execute_data *execute_data, zend_op_array *op_array, int do_cc)
{
	if (!op_array->reserved[XG_COV(code_coverage_filter_offset)] && XG_COV(code_coverage_active)) {
		const zend_op *cur_opcode = execute_data->opline;
		xdebug_print_opcode_info('=', execute_data, cur_opcode);

		if (do_cc) {
			char *file   = (char*) STR_NAME_VAL(op_array->filename);
			int   lineno = cur_opcode->lineno;

			xdebug_count_line(file, lineno, 0, 0);
		}
	}
}

void xdebug_coverage_record_include_if_active(zend_execute_data *execute_data, zend_op_array *op_array)
{
	if (!op_array->reserved[XG_COV(code_coverage_filter_offset)] && XG_COV(code_coverage_active)) {
		const zend_op *cur_opcode = execute_data->opline;
		xdebug_print_opcode_info('I', execute_data, cur_opcode);
	}
}

void xdebug_coverage_record_silence_if_active(zend_execute_data *execute_data, zend_op_array *op_array)
{
	if (!op_array->reserved[XG_COV(code_coverage_filter_offset)] && XG_COV(code_coverage_active)) {
		const zend_op *cur_opcode = execute_data->opline;
		xdebug_print_opcode_info('S', execute_data, cur_opcode);
	}
}

void xdebug_coverage_compile_file(zend_op_array *op_array)
{
	if (XG_COV(code_coverage_active) && XG_COV(code_coverage_unused) && (op_array->fn_flags & ZEND_ACC_DONE_PASS_TWO)) {
		xdebug_prefill_code_coverage(op_array);
	}
}

int xdebug_coverage_execute_ex(function_stack_entry *fse, zend_op_array *op_array, char **tmp_file_name, char **tmp_function_name)
{
	xdebug_func func_info;

	if (!fse->filtered_code_coverage && XG_COV(code_coverage_active) && XG_COV(code_coverage_unused)) {
		*tmp_file_name = xdstrdup(STR_NAME_VAL(op_array->filename));
		xdebug_build_fname_from_oparray(&func_info, op_array);
		*tmp_function_name = xdebug_func_format(&func_info);
		xdebug_code_coverage_start_of_function(op_array, *tmp_function_name);

		if (func_info.class) {
			xdfree(func_info.class);
		}
		if (func_info.function) {
			xdfree(func_info.function);
		}
		return 1;
	}

	return 0;
}

void xdebug_coverage_execute_ex_end(function_stack_entry *fse, zend_op_array *op_array, char *tmp_file_name, char *tmp_function_name)
{
	/* Check which path has been used */
	if (!fse->filtered_code_coverage && XG_COV(code_coverage_active) && XG_COV(code_coverage_unused)) {
		xdebug_code_coverage_end_of_function(op_array, tmp_file_name, tmp_function_name);
		xdfree(tmp_function_name);
		xdfree(tmp_file_name);
	}
}

void xdebug_coverage_init_oparray(zend_op_array *op_array)
{
	op_array->reserved[XG_COV(dead_code_analysis_tracker_offset)] = 0;

	if (XG_BASE(filter_type_code_coverage) != XDEBUG_FILTER_NONE) {
		function_stack_entry tmp_fse;

		tmp_fse.filename = STR_NAME_VAL(op_array->filename);
		xdebug_build_fname_from_oparray(&tmp_fse.function, op_array);
		xdebug_filter_run_internal(&tmp_fse, XDEBUG_FILTER_CODE_COVERAGE, &tmp_fse.filtered_code_coverage, XG_BASE(filter_type_code_coverage), XG_BASE(filters_code_coverage));
		xdebug_func_dtor_by_ref(&tmp_fse.function);

		op_array->reserved[XG_COV(code_coverage_filter_offset)] = (void*) tmp_fse.filtered_code_coverage;
	}
}

#if PHP_VERSION_ID >= 70200
static int xdebug_switch_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	const zend_op *cur_opcode = execute_data->opline;

	if (XG_COV(code_coverage_active)) {
		execute_data->opline++;
		return ZEND_USER_OPCODE_CONTINUE;
	}

	return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}
#endif

#define XDEBUG_OPCODE_OVERRIDE(f) \
	int xdebug_##f##_handler(zend_execute_data *execute_data) \
	{ \
		return xdebug_common_override_handler(execute_data); \
	}


void xdebug_coverage_minit(INIT_FUNC_ARGS)
{
	zend_extension dummy_ext;

	/* Get reserved offsets */
	zend_xdebug_cc_run_offset = zend_get_resource_handle(&dummy_ext);
	zend_xdebug_filter_offset = zend_get_resource_handle(&dummy_ext);

	/* Overload opcodes for code coverage */
	if (XINI_COV(enable)) {
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_JMP);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_JMPZ);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_JMPZ_EX);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_JMPNZ);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_IDENTICAL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_NOT_IDENTICAL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_EQUAL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_NOT_EQUAL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_SMALLER);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_SMALLER_OR_EQUAL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BOOL_NOT);

		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ADD);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SUB);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_MUL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_DIV);

		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ADD_ARRAY_ELEMENT);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_RETURN);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_RETURN_BY_REF);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_EXT_STMT);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAR);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAR_NO_REF);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAR_NO_REF_EX);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_REF);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAL_EX);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAR_EX);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_NEW);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_EXT_FCALL_BEGIN);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INIT_METHOD_CALL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INIT_STATIC_METHOD_CALL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INIT_FCALL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INIT_NS_FCALL_BY_NAME);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_CATCH);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BOOL);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INIT_ARRAY);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_R);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_W);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_R);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_W);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_FUNC_ARG);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_FUNC_ARG);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_STATIC_PROP_FUNC_ARG);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_UNSET);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_UNSET);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_CLASS);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_CONSTANT);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_CLASS_CONSTANT);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_CONCAT);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ISSET_ISEMPTY_DIM_OBJ);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ISSET_ISEMPTY_PROP_OBJ);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_CASE);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_DECLARE_LAMBDA_FUNCTION);
#if PHP_VERSION_ID < 70400
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ADD_TRAIT);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BIND_TRAITS);
#endif
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INSTANCEOF);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FAST_RET);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ROPE_ADD);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ROPE_END);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_COALESCE);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_TYPE_CHECK);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_GENERATOR_CREATE);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BIND_STATIC);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BIND_LEXICAL);
#if PHP_VERSION_ID >= 70400
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_DECLARE_CLASS);
		XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_DECLARE_CLASS_DELAYED);
#endif
#if PHP_VERSION_ID >= 70200
		xdebug_set_opcode_handler(ZEND_SWITCH_STRING, xdebug_switch_handler);
		xdebug_set_opcode_handler(ZEND_SWITCH_LONG, xdebug_switch_handler);
#endif
	}

	/* Override all the other opcodes so that we can mark when we hit a branch
	 * start one */
	if (XINI_COV(enable)) {
		int i;

		for (i = 0; i < 256; i++) {
			if (i == ZEND_HANDLE_EXCEPTION) {
				continue;
			}
			if (!xdebug_isset_opcode_handler(i)) {
				xdebug_set_opcode_handler(i, xdebug_check_branch_entry_handler);
			}
		}
	}

	REGISTER_LONG_CONSTANT("XDEBUG_CC_UNUSED", XDEBUG_CC_OPTION_UNUSED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_CC_DEAD_CODE", XDEBUG_CC_OPTION_DEAD_CODE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_CC_BRANCH_CHECK", XDEBUG_CC_OPTION_BRANCH_CHECK, CONST_CS | CONST_PERSISTENT);
}

void xdebug_coverage_rinit(void)
{
	XG_COV(code_coverage_active) = 0;
	XG_COV(code_coverage_info) = xdebug_hash_alloc(32, xdebug_coverage_file_dtor);
	XG_COV(dead_code_analysis_tracker_offset) = zend_xdebug_cc_run_offset;
	XG_COV(dead_code_last_start_id) = 1;
	XG_COV(code_coverage_filter_offset) = zend_xdebug_filter_offset;
	XG_COV(previous_filename) = NULL;
	XG_COV(previous_file) = NULL;
	XG_COV(prefill_function_count) = 0;
	XG_COV(prefill_class_count) = 0;

	/* Initialize visited classes and branches hash */
	XG_COV(visited_branches) = xdebug_hash_alloc(2048, NULL);

	XG_COV(paths_stack) = xdebug_path_info_ctor();
	XG_COV(branches).size = 0;
	XG_COV(branches).last_branch_nr = NULL;
}

void xdebug_coverage_post_deactivate(void)
{
	XG_COV(code_coverage_active) = 0;

	xdebug_hash_destroy(XG_COV(code_coverage_info));
	XG_COV(code_coverage_info) = NULL;

	xdebug_hash_destroy(XG_COV(visited_branches));
	XG_COV(visited_branches) = NULL;

	/* Clean up path coverage array */
	if (XG_COV(paths_stack)) {
		xdebug_path_info_dtor(XG_COV(paths_stack));
		XG_COV(paths_stack) = NULL;
	}
	if (XG_COV(branches).last_branch_nr) {
		free(XG_COV(branches).last_branch_nr);
		XG_COV(branches).last_branch_nr = NULL;
		XG_COV(branches).size = 0;
	}
	XG_COV(previous_mark_filename) = NULL;
}
