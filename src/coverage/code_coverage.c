/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2021 Derick Rethans                               |
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

#include "php_xdebug.h"
#include "zend_extensions.h"

#include "branch_info.h"
#include "code_coverage_private.h"

#include "base/base.h"
#include "base/filter.h"
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

xdebug_coverage_file *xdebug_coverage_file_ctor(zend_string *filename)
{
	xdebug_coverage_file *file;

	file = xdmalloc(sizeof(xdebug_coverage_file));
	file->name = zend_string_copy(filename);
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
	zend_string_release(file->name);
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

static void xdebug_func_format(char *buffer, size_t buffer_size, xdebug_func *func)
{
	if (func->type == XFUNC_NORMAL) {
		int len = strlen(func->function);

		if (len + 1 > buffer_size) {
			goto error;
		}
		memcpy(buffer, func->function, len);
		buffer[len] = '\0';
		return;
	}

	if (func->type == XFUNC_MEMBER) {
		int func_len = strlen(func->function);
		int len = ZSTR_LEN(func->object_class) + 2 + func_len;

		if (len + 1 > buffer_size) {
			goto error;
		}
		memcpy(buffer, ZSTR_VAL(func->object_class), ZSTR_LEN(func->object_class));
		memcpy(buffer + ZSTR_LEN(func->object_class), "->", 2);
		memcpy(buffer + ZSTR_LEN(func->object_class) + 2, func->function, func_len);
		buffer[len] = '\0';
		return;
	}

error:
	memcpy(buffer, "?", 1);
	buffer[1] = '\0';
}

static void xdebug_build_fname_from_oparray(xdebug_func *tmp, zend_op_array *opa)
{
	int closure = 0;

	memset(tmp, 0, sizeof(xdebug_func));

	if (opa->function_name) {
		if (opa->fn_flags & ZEND_ACC_CLOSURE) {
			tmp->function = xdebug_wrap_closure_location_around_function_name(opa, STR_NAME_VAL(opa->function_name));
			closure = 1;
		} else {
			tmp->function = xdstrdup(STR_NAME_VAL(opa->function_name));
		}
	} else {
		tmp->function = xdstrdup("{main}");
		tmp->type = XFUNC_MAIN;
	}

	if (opa->scope && !closure) {
		tmp->type = XFUNC_MEMBER;
		tmp->object_class = zend_string_copy(opa->scope->name);
	} else {
		tmp->type = XFUNC_NORMAL;
	}
}

static void xdebug_print_opcode_info(zend_execute_data *execute_data, const zend_op *cur_opcode)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	xdebug_func func_info;
	char function_name[1024];
	long opnr = execute_data->opline - execute_data->func->op_array.opcodes;

	xdebug_build_fname_from_oparray(&func_info, op_array);
	xdebug_func_format(function_name, sizeof(function_name), &func_info);
	if (func_info.object_class) {
		zend_string_release(func_info.object_class);
	}
	if (func_info.scope_class) {
		zend_string_release(func_info.scope_class);
	}
	if (func_info.function) {
		xdfree(func_info.function);
	}

	xdebug_branch_info_mark_reached(op_array->filename, function_name, op_array, opnr);
}

static int xdebug_check_branch_entry_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	const zend_op *cur_opcode = execute_data->opline;

	if (!op_array->reserved[XG_COV(code_coverage_filter_offset)] && XG_COV(code_coverage_active)) {
		xdebug_print_opcode_info(execute_data, cur_opcode);
	}

	return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}

static void xdebug_count_line(zend_string *filename, int lineno, int executable, int deadcode)
{
	xdebug_coverage_file *file;
	xdebug_coverage_line *line;

	if (XG_COV(previous_filename) && zend_string_equals(XG_COV(previous_filename), filename)) {
		file = XG_COV(previous_file);
	} else {
		/* Check if the file already exists in the hash */
		if (!xdebug_hash_find(XG_COV(code_coverage_info), ZSTR_VAL(filename), ZSTR_LEN(filename), (void *) &file)) {
			/* The file does not exist, so we add it to the hash */
			file = xdebug_coverage_file_ctor(filename);

			xdebug_hash_add(XG_COV(code_coverage_info), ZSTR_VAL(filename), ZSTR_LEN(filename), file);
		}
		if (XG_COV(previous_filename)) {
			zend_string_release(XG_COV(previous_filename));
		}
		XG_COV(previous_filename) = zend_string_copy(file->name);
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

static int xdebug_common_override_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	const zend_op *cur_opcode = execute_data->opline;

	if (!op_array->reserved[XG_COV(code_coverage_filter_offset)] && XG_COV(code_coverage_active)) {
		int      lineno;

		lineno = cur_opcode->lineno;

		xdebug_print_opcode_info(execute_data, cur_opcode);
		xdebug_count_line(op_array->filename, lineno, 0, 0);
	}

	return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}

static int xdebug_coverage_include_or_eval_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	const zend_op *opline = execute_data->opline;

	xdebug_coverage_record_if_active(execute_data, op_array);

	return xdebug_call_original_opcode_handler_if_set(opline->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}

static void prefill_from_opcode(zend_string *filename, zend_op opcode, int deadcode)
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
		xdebug_count_line(filename, opcode.lineno, 1, deadcode);
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
#if PHP_VERSION_ID >= 80000
		opcode.opcode == ZEND_MATCH_ERROR ||
#endif
		opcode.opcode == ZEND_RETURN
	) {
		jumps[0] = XDEBUG_JMP_EXIT;
		*jump_count = 1;
		return 1;
	} else if (
# if PHP_VERSION_ID >= 80000
		opcode.opcode == ZEND_MATCH ||
# endif
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

# if PHP_VERSION_ID >= 80000
		if (opcode.opcode != ZEND_MATCH) {
# endif
			/* The 'next' opcode */
			jumps[*jump_count] = position + 1;
			(*jump_count)++;
# if PHP_VERSION_ID >= 80000
		}
# endif

		return 1;
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

static void prefill_from_oparray(zend_string *filename, zend_op_array *op_array)
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
		char function_name[1024];
		xdebug_func func_info;

		xdebug_build_fname_from_oparray(&func_info, op_array);
		xdebug_func_format(function_name, sizeof(function_name), &func_info);

		if (func_info.object_class) {
			zend_string_release(func_info.object_class);
		}
		if (func_info.scope_class) {
			zend_string_release(func_info.scope_class);
		}
		if (func_info.function) {
			xdfree(func_info.function);
		}

		xdebug_branch_post_process(op_array, branch_info);
		xdebug_branch_find_paths(branch_info);
		xdebug_branch_info_add_branches_and_paths(filename, (char*) function_name, branch_info);
	}

#if PHP_VERSION_ID >= 80100
	if (!op_array->num_dynamic_func_defs) {
		return;
	}

	for (i = 0; i < op_array->num_dynamic_func_defs; i++) {
		prefill_from_oparray(filename, op_array->dynamic_func_defs[i]);
	}
#endif
}

static int prefill_from_function_table(zend_op_array *opa)
{
	if (opa->type == ZEND_USER_FUNCTION) {
		if ((long) opa->reserved[XG_COV(dead_code_analysis_tracker_offset)] < XG_COV(dead_code_last_start_id)) {
			prefill_from_oparray(opa->filename, opa);
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
		prefill_from_oparray(op_array->filename, op_array);
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
	xdebug_path_info_add_path_for_level(XG_COV(paths_stack), path, XDEBUG_VECTOR_COUNT(XG_BASE(stack)));

	if (XG_COV(branches).size == 0 || XDEBUG_VECTOR_COUNT(XG_BASE(stack)) >= XG_COV(branches).size) {
		XG_COV(branches).size = XDEBUG_VECTOR_COUNT(XG_BASE(stack)) + 32;
		XG_COV(branches).last_branch_nr = realloc(XG_COV(branches).last_branch_nr, sizeof(int) * XG_COV(branches.size));
	}

	XG_COV(branches).last_branch_nr[XDEBUG_VECTOR_COUNT(XG_BASE(stack))] = -1;
}

void xdebug_code_coverage_end_of_function(zend_op_array *op_array, zend_string *filename, char *function_name)
{
	xdebug_str str = XDEBUG_STR_INITIALIZER;
	xdebug_path *path = xdebug_path_info_get_path_for_level(XG_COV(paths_stack), XDEBUG_VECTOR_COUNT(XG_BASE(stack)));

	if (!path || !path->elements) {
		return;
	}

	xdebug_create_key_for_path(path, &str);

	xdebug_branch_info_mark_end_of_function_reached(filename, function_name, str.d, str.l);

	xdfree(str.d);

	if (path) {
		xdebug_path_free(path);
	}
}

PHP_FUNCTION(xdebug_start_code_coverage)
{
	zend_long options = 0;

	if (!XDEBUG_MODE_IS(XDEBUG_MODE_COVERAGE)) {
		php_error(E_WARNING, "Code coverage needs to be enabled in php.ini by setting 'xdebug.mode' to 'coverage'");
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &options) == FAILURE) {
		return;
	}

	XG_COV(code_coverage_unused) = (options & XDEBUG_CC_OPTION_UNUSED);
	XG_COV(code_coverage_dead_code_analysis) = (options & XDEBUG_CC_OPTION_DEAD_CODE);
	XG_COV(code_coverage_branch_check) = (options & XDEBUG_CC_OPTION_BRANCH_CHECK);

	XG_COV(code_coverage_active) = 1;
	RETURN_TRUE;
}

PHP_FUNCTION(xdebug_stop_code_coverage)
{
	zend_long cleanup = 1;

	if (!XDEBUG_MODE_IS(XDEBUG_MODE_COVERAGE)) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &cleanup) == FAILURE) {
		return;
	}
	if (!XG_COV(code_coverage_active)) {
		RETURN_FALSE;
	}

	if (cleanup) {
		if (XG_COV(previous_filename)) {
			zend_string_release(XG_COV(previous_filename));
		}
		XG_COV(previous_filename) = NULL;
		XG_COV(previous_file) = NULL;
		if (XG_COV(previous_mark_filename)) {
			zend_string_release(XG_COV(previous_mark_filename));
		}
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

#if PHP_VERSION_ID >= 80000
static int xdebug_lineno_cmp(Bucket *f, Bucket *s)
{
#else
static int xdebug_lineno_cmp(const void *a, const void *b)
{
	Bucket *f = (Bucket *) a;
	Bucket *s = (Bucket *) b;
#endif
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

		add_assoc_zval_ex(retval, ZSTR_VAL(file->name), ZSTR_LEN(file->name), file_info);
		efree(functions);
		efree(file_info);
	} else {
		add_assoc_zval_ex(retval, ZSTR_VAL(file->name), ZSTR_LEN(file->name), lines);
	}

	efree(lines);
}

PHP_FUNCTION(xdebug_get_code_coverage)
{
	array_init(return_value);

	if (!XG_COV(code_coverage_info)) {
		return;
	}

	xdebug_hash_apply(XG_COV(code_coverage_info), (void *) return_value, add_file);
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

void xdebug_coverage_count_line_if_active(zend_op_array *op_array, zend_string *file, int lineno)
{
	if (XG_COV(code_coverage_active) && !op_array->reserved[XG_COV(code_coverage_filter_offset)]) {
		xdebug_count_line(file, lineno, 0, 0);
	}
}

void xdebug_coverage_count_line_if_branch_check_active(zend_op_array *op_array, zend_string *file, int lineno)
{
	if (XG_COV(code_coverage_active) && XG_COV(code_coverage_branch_check)) {
		xdebug_coverage_count_line_if_active(op_array, file, lineno);
	}
}

void xdebug_coverage_record_if_active(zend_execute_data *execute_data, zend_op_array *op_array)
{
	if (!op_array->reserved[XG_COV(code_coverage_filter_offset)] && XG_COV(code_coverage_active)) {
		xdebug_print_opcode_info(execute_data, execute_data->opline);
	}
}

void xdebug_coverage_compile_file(zend_op_array *op_array)
{
	if (XG_COV(code_coverage_active) && XG_COV(code_coverage_unused) && (op_array->fn_flags & ZEND_ACC_DONE_PASS_TWO)) {
		xdebug_prefill_code_coverage(op_array);
	}
}

int xdebug_coverage_execute_ex(function_stack_entry *fse, zend_op_array *op_array, zend_string **tmp_filename, char **tmp_function_name)
{
	xdebug_func func_info;

	if (!fse->filtered_code_coverage && XG_COV(code_coverage_active) && XG_COV(code_coverage_unused)) {
		char buffer[1024];

		*tmp_filename = zend_string_copy(op_array->filename);
		xdebug_build_fname_from_oparray(&func_info, op_array);
		xdebug_func_format(buffer, sizeof(buffer), &func_info);
		*tmp_function_name = xdstrdup(buffer);
		xdebug_code_coverage_start_of_function(op_array, *tmp_function_name);

		if (func_info.object_class) {
			zend_string_release(func_info.object_class);
		}
		if (func_info.scope_class) {
			zend_string_release(func_info.scope_class);
		}
		if (func_info.function) {
			xdfree(func_info.function);
		}
		return 1;
	}

	return 0;
}

void xdebug_coverage_execute_ex_end(function_stack_entry *fse, zend_op_array *op_array, zend_string *tmp_filename, char *tmp_function_name)
{
	/* Check which path has been used */
	if (!fse->filtered_code_coverage && XG_COV(code_coverage_active) && XG_COV(code_coverage_unused)) {
		xdebug_code_coverage_end_of_function(op_array, tmp_filename, tmp_function_name);
	}
	xdfree(tmp_function_name);
	zend_string_release(tmp_filename);
}

void xdebug_coverage_init_oparray(zend_op_array *op_array)
{
	function_stack_entry tmp_fse;

	if (XG_BASE(filter_type_code_coverage) == XDEBUG_FILTER_NONE) {
		op_array->reserved[XG_COV(dead_code_analysis_tracker_offset)] = 0;
		return;
	}

	tmp_fse.filename = op_array->filename;
	xdebug_build_fname_from_oparray(&tmp_fse.function, op_array);
	xdebug_filter_run_internal(&tmp_fse, XDEBUG_FILTER_CODE_COVERAGE, &tmp_fse.filtered_code_coverage, XG_BASE(filter_type_code_coverage), XG_BASE(filters_code_coverage));
	xdebug_func_dtor_by_ref(&tmp_fse.function);

	op_array->reserved[XG_COV(code_coverage_filter_offset)] = (void*) (size_t) tmp_fse.filtered_code_coverage;
}

static int xdebug_switch_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	const zend_op *cur_opcode = execute_data->opline;

	if (!XG_COV(code_coverage_active)) {
		return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
	}

	execute_data->opline++;
	return ZEND_USER_OPCODE_CONTINUE;
}


void xdebug_coverage_minit(INIT_FUNC_ARGS)
{
	int i;

	/* Get reserved offsets */
#if PHP_VERSION_ID >= 80000
	zend_xdebug_cc_run_offset = zend_get_resource_handle(XDEBUG_NAME);
	zend_xdebug_filter_offset = zend_get_resource_handle(XDEBUG_NAME);
#else
	zend_extension dummy_ext;
	zend_xdebug_cc_run_offset = zend_get_resource_handle(&dummy_ext);
	zend_xdebug_filter_offset = zend_get_resource_handle(&dummy_ext);
#endif

	xdebug_register_with_opcode_multi_handler(ZEND_ASSIGN, xdebug_common_override_handler);
	xdebug_register_with_opcode_multi_handler(ZEND_ASSIGN_DIM, xdebug_common_override_handler);
	xdebug_register_with_opcode_multi_handler(ZEND_ASSIGN_OBJ, xdebug_common_override_handler);
	xdebug_register_with_opcode_multi_handler(ZEND_QM_ASSIGN, xdebug_common_override_handler);
	xdebug_register_with_opcode_multi_handler(ZEND_INCLUDE_OR_EVAL, xdebug_coverage_include_or_eval_handler);

	/* Overload opcodes for code coverage */
	xdebug_set_opcode_handler(ZEND_JMP, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_JMPZ, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_JMPZ_EX, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_JMPNZ, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_IS_IDENTICAL, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_IS_NOT_IDENTICAL, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_IS_EQUAL, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_IS_NOT_EQUAL, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_IS_SMALLER, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_IS_SMALLER_OR_EQUAL, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_BOOL_NOT, xdebug_common_override_handler);

	xdebug_set_opcode_handler(ZEND_ADD, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_SUB, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_MUL, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_DIV, xdebug_common_override_handler);

	xdebug_set_opcode_handler(ZEND_ADD_ARRAY_ELEMENT, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_RETURN, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_RETURN_BY_REF, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_EXT_STMT, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_SEND_VAR, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_SEND_VAR_NO_REF, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_SEND_VAR_NO_REF_EX, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_SEND_REF, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_SEND_VAL, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_SEND_VAL_EX, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_SEND_VAR_EX, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_NEW, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_EXT_FCALL_BEGIN, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_INIT_METHOD_CALL, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_INIT_STATIC_METHOD_CALL, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_INIT_FCALL, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_INIT_NS_FCALL_BY_NAME, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_CATCH, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_BOOL, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_INIT_ARRAY, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FETCH_DIM_R, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FETCH_DIM_W, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FETCH_OBJ_R, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FETCH_OBJ_W, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FETCH_OBJ_FUNC_ARG, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FETCH_DIM_FUNC_ARG, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FETCH_STATIC_PROP_FUNC_ARG, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FETCH_DIM_UNSET, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FETCH_OBJ_UNSET, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FETCH_CLASS, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FETCH_CONSTANT, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FETCH_CLASS_CONSTANT, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_CONCAT, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FAST_CONCAT, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_ISSET_ISEMPTY_DIM_OBJ, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_ISSET_ISEMPTY_PROP_OBJ, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_CASE, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_DECLARE_LAMBDA_FUNCTION, xdebug_common_override_handler);
#if PHP_VERSION_ID < 70400
	xdebug_set_opcode_handler(ZEND_ADD_TRAIT, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_BIND_TRAITS, xdebug_common_override_handler);
#endif
	xdebug_set_opcode_handler(ZEND_INSTANCEOF, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_FAST_RET, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_ROPE_ADD, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_ROPE_END, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_COALESCE, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_TYPE_CHECK, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_GENERATOR_CREATE, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_BIND_STATIC, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_BIND_LEXICAL, xdebug_common_override_handler);
#if PHP_VERSION_ID >= 70400
	xdebug_set_opcode_handler(ZEND_DECLARE_CLASS, xdebug_common_override_handler);
	xdebug_set_opcode_handler(ZEND_DECLARE_CLASS_DELAYED, xdebug_common_override_handler);
#endif
	xdebug_set_opcode_handler(ZEND_SWITCH_STRING, xdebug_switch_handler);
	xdebug_set_opcode_handler(ZEND_SWITCH_LONG, xdebug_switch_handler);

	/* Override all the other opcodes so that we can mark when we hit a branch
	 * start one */
	for (i = 0; i < 256; i++) {
		if (i == ZEND_HANDLE_EXCEPTION) {
			continue;
		}
		if (!xdebug_isset_opcode_handler(i)) {
			xdebug_set_opcode_handler(i, xdebug_check_branch_entry_handler);
		}
	}
}

void xdebug_coverage_register_constants(INIT_FUNC_ARGS)
{
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

	if (XG_COV(previous_filename)) {
		zend_string_release(XG_COV(previous_filename));
		XG_COV(previous_filename) = NULL;
	}
	if (XG_COV(previous_mark_filename)) {
		zend_string_release(XG_COV(previous_mark_filename));
		XG_COV(previous_mark_filename) = NULL;
	}
}
