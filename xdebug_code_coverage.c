/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2018 Derick Rethans                               |
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
#include "xdebug_private.h"
#include "xdebug_filter.h"
#include "xdebug_set.h"
#include "xdebug_stack.h"
#include "xdebug_var.h"
#include "xdebug_branch_info.h"
#include "xdebug_code_coverage.h"
#include "xdebug_compat.h"
#include "xdebug_tracing.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

void xdebug_coverage_line_dtor(void *data)
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

void xdebug_coverage_file_dtor(void *data)
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

void xdebug_print_opcode_info(char type, zend_execute_data *execute_data, const zend_op *cur_opcode TSRMLS_DC)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	char *file = (char*) STR_NAME_VAL(op_array->filename);
	xdebug_func func_info;
	char *function_name;
	long opnr = execute_data->opline - execute_data->func->op_array.opcodes;

	xdebug_build_fname_from_oparray(&func_info, op_array TSRMLS_CC);
	function_name = xdebug_func_format(&func_info TSRMLS_CC);
	if (func_info.class) {
		xdfree(func_info.class);
	}
	if (func_info.function) {
		xdfree(func_info.function);
	}

	xdebug_branch_info_mark_reached(file, function_name, op_array, opnr TSRMLS_CC);
	xdfree(function_name);
}

int xdebug_check_branch_entry_handler(zend_execute_data *execute_data)
{
	zend_op_array *op_array = &execute_data->func->op_array;

	if (!op_array->reserved[XG(code_coverage_filter_offset)] && XG(do_code_coverage)) {
		const zend_op *cur_opcode;
		cur_opcode = execute_data->opline;

		xdebug_print_opcode_info('G', execute_data, cur_opcode TSRMLS_CC);
	}
	return ZEND_USER_OPCODE_DISPATCH;
}

#define XDEBUG_OPCODE_OVERRIDE(f) \
	int xdebug_##f##_handler(zend_execute_data *execute_data) \
	{ \
		return xdebug_common_override_handler(execute_data); \
	}


int xdebug_common_override_handler(zend_execute_data *execute_data)
{
	zend_op_array *op_array = &execute_data->func->op_array;

	if (!op_array->reserved[XG(code_coverage_filter_offset)] && XG(do_code_coverage)) {
		const zend_op *cur_opcode;
		int      lineno;
		char    *file;

		cur_opcode = execute_data->opline;
		lineno = cur_opcode->lineno;
		file = (char*) STR_NAME_VAL(op_array->filename);

		xdebug_print_opcode_info('C', execute_data, cur_opcode TSRMLS_CC);
		xdebug_count_line(file, lineno, 0, 0 TSRMLS_CC);
	}
	return ZEND_USER_OPCODE_DISPATCH;
}

static int xdebug_is_static_call(const zend_op *cur_opcode, const zend_op *prev_opcode, const zend_op **found_opcode)
{
	const zend_op *opcode_ptr;

	opcode_ptr = cur_opcode;
# if PHP_VERSION_ID >= 70100
	while (!(opcode_ptr->opcode == ZEND_EXT_STMT) && !((opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_W) || (opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_RW))) {
		opcode_ptr = opcode_ptr - 1;
	}
	if ((opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_W) || (opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_RW)) {
# else
	while (!(opcode_ptr->opcode == ZEND_EXT_STMT) && !((opcode_ptr->opcode == ZEND_FETCH_W) || (opcode_ptr->opcode == ZEND_FETCH_RW))) {
		opcode_ptr = opcode_ptr - 1;
	}
	if (((opcode_ptr->opcode == ZEND_FETCH_W) || (opcode_ptr->opcode == ZEND_FETCH_RW)) && opcode_ptr->extended_value == ZEND_FETCH_STATIC_MEMBER) {
# endif
		*found_opcode = opcode_ptr;
		return 1;
	}
	return 0;
}

static const zend_op *xdebug_find_referenced_opline(zend_execute_data *execute_data, const zend_op *cur_opcode, int op1_or_op2)
{
	int op_type = (op1_or_op2 == 1) ? cur_opcode->op1_type : cur_opcode->op2_type;

	if (op_type == IS_VAR) {
		size_t variable_number = (op1_or_op2 == 1) ? cur_opcode->op1.var : cur_opcode->op2.var;
		const zend_op *scan_opcode = cur_opcode;
		int found = 0;

		/* Scroll up until we find a RES of IS_VAR with the right value */
		do {
			scan_opcode--;
			if (scan_opcode->result_type == IS_VAR && scan_opcode->result.var == variable_number) {
				found = 1;
			}
		} while (!found);
		return scan_opcode;
	}
	return NULL;
}

static char *xdebug_find_var_name(zend_execute_data *execute_data, const zend_op *cur_opcode, const zend_op *lower_bound TSRMLS_DC)
{
	const zend_op *next_opcode, *prev_opcode = NULL, *opcode_ptr;
	zval          *dimval;
	int            is_var;
	zend_op_array *op_array = &execute_data->func->op_array;
	xdebug_str     name = XDEBUG_STR_INITIALIZER;
	int            gohungfound = 0, is_static = 0;
	xdebug_str    *zval_value = NULL;
	xdebug_var_export_options *options;
	const zend_op *static_opcode_ptr = NULL;

	next_opcode = cur_opcode + 1;
	prev_opcode = cur_opcode - 1;

	if (cur_opcode->opcode == ZEND_QM_ASSIGN) {
#if PHP_VERSION_ID >= 70000
		xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->result.var)->val), 1);
#else
		xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->result.var, &cv_len)), 1);
#endif
	}

	is_static = xdebug_is_static_call(cur_opcode, prev_opcode, &static_opcode_ptr);
	options = xdebug_var_export_options_from_ini(TSRMLS_C);
	options->no_decoration = 1;

	if (cur_opcode->op1_type == IS_CV) {
		if (!lower_bound) {
			xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->op1.var)->val), 1);
		}
	} else if (cur_opcode->op1_type == IS_VAR && cur_opcode->opcode == ZEND_ASSIGN && (prev_opcode->opcode == ZEND_FETCH_W || prev_opcode->opcode == ZEND_FETCH_RW)) {
		if (is_static) {
			xdebug_str_add(&name, xdebug_sprintf("self::"), 1);
		} else {
			zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, prev_opcode->op1_type, &prev_opcode->op1, &is_var), 0, options);
			xdebug_str_addc(&name, '$');
			xdebug_str_add_str(&name, zval_value);
			xdebug_str_free(zval_value);
		}
	} else if (is_static) { /* todo : see if you can change this and the previous cases around */
		xdebug_str_add(&name, xdebug_sprintf("self::"), 1 );
	}
	if ((cur_opcode->opcode >= ZEND_ASSIGN_ADD && cur_opcode->opcode <= ZEND_ASSIGN_BW_XOR)
		|| cur_opcode->opcode == ZEND_ASSIGN_POW
	) {
		if (cur_opcode->extended_value == ZEND_ASSIGN_OBJ) {
			zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var), 0, options);
			if (cur_opcode->op1_type == IS_UNUSED) {
				xdebug_str_addl(&name, "$this->", 7, 0);
			} else {
				xdebug_str_addl(&name, "->", 2, 0);
			}
			xdebug_str_add_str(&name, zval_value);
			xdebug_str_free(zval_value);
		} else if (cur_opcode->extended_value == ZEND_ASSIGN_DIM) {
			zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var), 0, NULL);
			xdebug_str_addc(&name, '[');
			xdebug_str_add_str(&name, zval_value);
			xdebug_str_addc(&name, ']');
			xdebug_str_free(zval_value);
		}
	}
	if (cur_opcode->opcode >= ZEND_PRE_INC_OBJ && cur_opcode->opcode <= ZEND_POST_DEC_OBJ) {
		zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var), 0, options);
		xdebug_str_addl(&name, "$this->", 7, 0);
		xdebug_str_add_str(&name, zval_value);
		xdebug_str_free(zval_value);
	}

	/* Scroll back to start of FETCHES */
	/* FIXME: See whether we can do this unroll looping only once - in is_static() */
	gohungfound = 0;
	if (!is_static) {
		if (cur_opcode == lower_bound) {
			gohungfound = 1;
		}
		opcode_ptr = prev_opcode;
		while ((opcode_ptr >= lower_bound) && (opcode_ptr->opcode == ZEND_FETCH_DIM_W || opcode_ptr->opcode == ZEND_FETCH_OBJ_W || opcode_ptr->opcode == ZEND_FETCH_W || opcode_ptr->opcode == ZEND_FETCH_RW)) {
			opcode_ptr = opcode_ptr - 1;
			gohungfound = 1;
		}
		opcode_ptr = opcode_ptr + 1;
	} else { /* if we have a static method, we should already have found the first fetch */
		opcode_ptr = static_opcode_ptr;
		gohungfound = 1;
	}

	if (gohungfound) {
		int cv_found = 0;

		do
		{
			if (opcode_ptr->op1_type == IS_UNUSED && opcode_ptr->opcode == ZEND_FETCH_OBJ_W) {
				xdebug_str_add(&name, "$this", 0);
			}
			if (opcode_ptr->op1_type == IS_CV) {
				xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, opcode_ptr->op1.var)->val), 1);
			}
#if PHP_VERSION_ID >= 70100
			if (opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_W || opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_R || opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_RW) {
				zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->op1_type, &opcode_ptr->op1, &is_var), 0, options);
				xdebug_str_add_str(&name, zval_value);
				xdebug_str_free(zval_value);
			}
#endif
			if (opcode_ptr->opcode == ZEND_FETCH_W) {
				zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->op1_type, &opcode_ptr->op1, &is_var), 0, options);
				xdebug_str_add_str(&name, zval_value);
				xdebug_str_free(zval_value);
			}
			if (is_static && opcode_ptr->opcode == ZEND_FETCH_RW) {
				zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->op1_type, &opcode_ptr->op1, &is_var), 0, options);
				xdebug_str_add_str(&name, zval_value);
				xdebug_str_free(zval_value);
			}
			if (opcode_ptr->opcode == ZEND_FETCH_DIM_W) {
				zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->op2_type, &opcode_ptr->op2, &is_var), 0, NULL);
				xdebug_str_addc(&name, '[');
				if (zval_value) {
					xdebug_str_add_str(&name, zval_value);
				}
				xdebug_str_addc(&name, ']');
				xdebug_str_free(zval_value);
			} else if (opcode_ptr->opcode == ZEND_FETCH_OBJ_W) {
				zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->op2_type, &opcode_ptr->op2, &is_var), 0, options);
				xdebug_str_addl(&name, "->", 2, 0);
				xdebug_str_add_str(&name, zval_value);
				xdebug_str_free(zval_value);
			}
			opcode_ptr = opcode_ptr + 1;
			if (opcode_ptr->op1_type == IS_CV) {
				cv_found = 1;
			}
		} while (!cv_found && (opcode_ptr->opcode == ZEND_FETCH_DIM_W || opcode_ptr->opcode == ZEND_FETCH_OBJ_W || opcode_ptr->opcode == ZEND_FETCH_W || opcode_ptr->opcode == ZEND_FETCH_RW));
	}

	if (cur_opcode->opcode == ZEND_ASSIGN_OBJ) {
		if (cur_opcode->op1_type == IS_UNUSED) {
			xdebug_str_add(&name, "$this", 0);
		}
		dimval = xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var);
		xdebug_str_add(&name, xdebug_sprintf("->%s", Z_STRVAL_P(dimval)), 1);
	}

	if (cur_opcode->opcode == ZEND_ASSIGN_DIM) {
		if (next_opcode->opcode == ZEND_OP_DATA && cur_opcode->op2_type == IS_UNUSED) {
			xdebug_str_add(&name, "[]", 0);
		} else {
			zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->op2_type, &opcode_ptr->op2, &is_var), 0, NULL);
			xdebug_str_addc(&name, '[');
			xdebug_str_add_str(&name, zval_value);
			xdebug_str_addc(&name, ']');
			xdfree(zval_value);
		}
	}

	xdfree(options->runtime);
	xdfree(options);

	return name.d;
}

static int xdebug_common_assign_dim_handler(const char *op, int do_cc, zend_execute_data *execute_data)
{
	char    *file;
	zend_op_array *op_array = &execute_data->func->op_array;
	int            lineno;
	const zend_op *cur_opcode, *next_opcode;
	zval          *val = NULL;
	char          *right_full_varname = NULL;
	int            is_var;
	function_stack_entry *fse;

	cur_opcode = execute_data->opline;
	next_opcode = cur_opcode + 1;
	file = (char*) STR_NAME_VAL(op_array->filename);
	lineno = cur_opcode->lineno;

	/* TODO TEST FOR ASSIGNMENTS IN FILTERING */
//	if (xdebug_is_top_stack_frame_filtered(XDEBUG_FILTER_CODE_COVERAGE)) {
//		return ZEND_USER_OPCODE_DISPATCH;
//	}

	if (!op_array->reserved[XG(code_coverage_filter_offset)] && XG(do_code_coverage)) {
		xdebug_print_opcode_info('=', execute_data, cur_opcode TSRMLS_CC);

		if (do_cc) {
			xdebug_count_line(file, lineno, 0, 0 TSRMLS_CC);
		}
	}
	if (XG(do_trace) && XG(trace_context) && XG(collect_assignments)) {
		char *full_varname;

		if (cur_opcode->opcode == ZEND_QM_ASSIGN && cur_opcode->result_type != IS_CV) {
			return ZEND_USER_OPCODE_DISPATCH;
		}

		full_varname = xdebug_find_var_name(execute_data, execute_data->opline, NULL TSRMLS_CC);

		if (cur_opcode->opcode >= ZEND_PRE_INC && cur_opcode->opcode <= ZEND_POST_DEC) {
			char *tmp_varname;

			switch (cur_opcode->opcode) {
				case ZEND_PRE_INC:  tmp_varname = xdebug_sprintf("++%s", full_varname); break;
				case ZEND_POST_INC: tmp_varname = xdebug_sprintf("%s++", full_varname); break;
				case ZEND_PRE_DEC:  tmp_varname = xdebug_sprintf("--%s", full_varname); break;
				case ZEND_POST_DEC: tmp_varname = xdebug_sprintf("%s--", full_varname); break;
			}
			xdfree(full_varname);
			full_varname = tmp_varname;

			val = xdebug_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1, &is_var);
		} else if (cur_opcode->opcode >= ZEND_PRE_INC_OBJ && cur_opcode->opcode <= ZEND_POST_DEC_OBJ) {
			char *tmp_varname;

			switch (cur_opcode->opcode) {
				case ZEND_PRE_INC_OBJ:  tmp_varname = xdebug_sprintf("++%s", full_varname); break;
				case ZEND_POST_INC_OBJ: tmp_varname = xdebug_sprintf("%s++", full_varname); break;
				case ZEND_PRE_DEC_OBJ:  tmp_varname = xdebug_sprintf("--%s", full_varname); break;
				case ZEND_POST_DEC_OBJ: tmp_varname = xdebug_sprintf("%s--", full_varname); break;
			}
			xdfree(full_varname);
			full_varname = tmp_varname;

			val = xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var);
		} else if (next_opcode->opcode == ZEND_OP_DATA) {
			val = xdebug_get_zval(execute_data, next_opcode->op1_type, &next_opcode->op1, &is_var);
		} else if (cur_opcode->opcode == ZEND_QM_ASSIGN) {
			val = xdebug_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1, &is_var);
		} else if (cur_opcode->opcode == ZEND_ASSIGN_REF) {
			if (cur_opcode->op2_type == IS_CV) {
				right_full_varname = xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->op2.var)->val);
			} else {
				const zend_op *referenced_opline = xdebug_find_referenced_opline(execute_data, cur_opcode, 2);
#if PHP_VERSION_ID <= 70100
				const zend_op *previous_opline = xdebug_find_referenced_opline(execute_data, cur_opcode, 1);
				right_full_varname = xdebug_find_var_name(execute_data, referenced_opline, previous_opline + 1);
#else
				right_full_varname = xdebug_find_var_name(execute_data, referenced_opline, NULL);
#endif
			}
		} else {
			val = xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var);
		}

		fse = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack)));
		if (XG(do_trace) && XG(trace_context) && XG(collect_assignments) && XG(trace_handler)->assignment) {
			XG(trace_handler)->assignment(XG(trace_context), fse, full_varname, val, right_full_varname, op, file, lineno TSRMLS_CC);
		}
		xdfree(full_varname);
	}
	return ZEND_USER_OPCODE_DISPATCH;
}

#define XDEBUG_OPCODE_OVERRIDE_ASSIGN(f,o,cc) \
	int xdebug_##f##_handler(zend_execute_data *execute_data) \
	{ \
		return xdebug_common_assign_dim_handler((o), (cc), execute_data); \
	}

XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign,"=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(qm_assign,"=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_add,"+=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_sub,"-=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_mul,"*=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_div,"/=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_mod,"%=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_pow,"**=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_sl,"<<=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_sr,">>=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_inc,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_inc,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_dec,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_dec,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_inc_obj,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_inc_obj,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_dec_obj,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_dec_obj,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_concat,".=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_bw_or,"|=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_bw_and,"&=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_bw_xor,"^=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_dim,"=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_obj,"=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_ref,"=&",1)

void xdebug_count_line(char *filename, int lineno, int executable, int deadcode TSRMLS_DC)
{
	xdebug_coverage_file *file;
	xdebug_coverage_line *line;

	if (XG(previous_filename) && strcmp(XG(previous_filename), filename) == 0) {
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

static void prefill_from_opcode(char *fn, zend_op opcode, int deadcode TSRMLS_DC)
{
	if (
		opcode.opcode != ZEND_NOP &&
		opcode.opcode != ZEND_EXT_NOP &&
		opcode.opcode != ZEND_RECV &&
		opcode.opcode != ZEND_RECV_INIT
		&& opcode.opcode != ZEND_VERIFY_ABSTRACT_CLASS
		&& opcode.opcode != ZEND_OP_DATA
		&& opcode.opcode != ZEND_ADD_INTERFACE
		&& opcode.opcode != ZEND_TICKS
		&& opcode.opcode != ZEND_FAST_CALL
		&& opcode.opcode != ZEND_RECV_VARIADIC
	) {
		xdebug_count_line(fn, opcode.lineno, 1, deadcode TSRMLS_CC);
	}
}

#define XDEBUG_ZNODE_ELEM(node,var) node.var
#if ZEND_USE_ABS_JMP_ADDR
# define XDEBUG_ZNODE_JMP_LINE(node, opline, base)  (int32_t)(((long)((node).jmp_addr) - (long)(base_address)) / sizeof(zend_op))
#else
# define XDEBUG_ZNODE_JMP_LINE(node, opline, base)  (int32_t)(((int32_t)((node).jmp_offset) / sizeof(zend_op)) + (opline))
#endif

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
		if (!opcode.result.num) {
#if PHP_VERSION_ID >= 70100
			jumps[1] = position + (opcode.extended_value / sizeof(zend_op));
#else
			jumps[1] = opcode.extended_value;
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

		array_value = RT_CONSTANT_EX(opa->literals, opcode.op2);
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

static void xdebug_analyse_branch(zend_op_array *opa, unsigned int position, xdebug_set *set, xdebug_branch_info *branch_info TSRMLS_DC)
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
						xdebug_analyse_branch(opa, jumps[i], set, branch_info TSRMLS_CC);
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

static void xdebug_analyse_oparray(zend_op_array *opa, xdebug_set *set, xdebug_branch_info *branch_info TSRMLS_DC)
{
	unsigned int position = 0;

	while (position < opa->last) {
		if (position == 0) {
			xdebug_analyse_branch(opa, position, set, branch_info TSRMLS_CC);
			if (branch_info) {
				xdebug_set_add(branch_info->entry_points, position);
			}
		} else if (opa->opcodes[position].opcode == ZEND_CATCH) {
			xdebug_analyse_branch(opa, position, set, branch_info TSRMLS_CC);
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

void xdebug_build_fname_from_oparray(xdebug_func *tmp, zend_op_array *opa TSRMLS_DC)
{
	int closure = 0;

	memset(tmp, 0, sizeof(xdebug_func));

	if (opa->function_name) {
		if (strcmp(STR_NAME_VAL(opa->function_name), "{closure}") == 0) {
			tmp->function = xdebug_sprintf(
				"{closure:%s:%d-%d}",
				STR_NAME_VAL(opa->filename),
				opa->line_start,
				opa->line_end
			);
			closure = 1;
		} else {
			tmp->function = xdstrdup(STR_NAME_VAL(opa->function_name));
		}
	} else {
		tmp->function = xdstrdup("{main}");
	}

	if (opa->scope && !closure) {
		tmp->type = XFUNC_MEMBER;
		tmp->class = xdstrdup(STR_NAME_VAL(opa->scope->name));
	} else {
		tmp->type = XFUNC_NORMAL;
	}
}

char* xdebug_func_format(xdebug_func *func TSRMLS_DC)
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

static void prefill_from_oparray(char *filename, zend_op_array *op_array TSRMLS_DC)
{
	unsigned int i;
	xdebug_set *set = NULL;
	xdebug_branch_info *branch_info = NULL;

	op_array->reserved[XG(dead_code_analysis_tracker_offset)] = (void*) XG(dead_code_last_start_id);

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
		xdebug_build_fname_from_oparray(&tmp_fse.function, op_array TSRMLS_CC);
		printf("    - PREFIL FILTERED FOR %s (%s::%s): %s\n",
			tmp_fse.filename, tmp_fse.function.class, tmp_fse.function.function,
			op_array->reserved[XG(code_coverage_filter_offset)] ? "YES" : "NO");
*/
		if (op_array->reserved[XG(code_coverage_filter_offset)]) {
			return;
		}
	}

	/* Run dead code analysis if requested */
	if (XG(code_coverage_dead_code_analysis) && (op_array->fn_flags & ZEND_ACC_DONE_PASS_TWO)) {
		set = xdebug_set_create(op_array->last);
		if (XG(code_coverage_branch_check)) {
			branch_info = xdebug_branch_info_create(op_array->last);
		}

		xdebug_analyse_oparray(op_array, set, branch_info TSRMLS_CC);
	}

	/* The normal loop then finally */
	for (i = 0; i < op_array->last; i++) {
		zend_op opcode = op_array->opcodes[i];
		prefill_from_opcode(filename, opcode, set ? !xdebug_set_in(set, i) : 0 TSRMLS_CC);
	}

	if (set) {
		xdebug_set_free(set);
	}
	if (branch_info) {
		char *function_name;
		xdebug_func func_info;

		xdebug_build_fname_from_oparray(&func_info, op_array TSRMLS_CC);
		function_name = xdebug_func_format(&func_info TSRMLS_CC);

		if (func_info.class) {
			xdfree(func_info.class);
		}
		if (func_info.function) {
			xdfree(func_info.function);
		}

		xdebug_branch_post_process(op_array, branch_info);
		xdebug_branch_find_paths(branch_info);
		xdebug_branch_info_add_branches_and_paths(filename, (char*) function_name, branch_info TSRMLS_CC);

		xdfree(function_name);
	}
}

static int prefill_from_function_table(zend_op_array *opa)
{
	if (opa->type == ZEND_USER_FUNCTION) {
		if ((long) opa->reserved[XG(dead_code_analysis_tracker_offset)] < XG(dead_code_last_start_id)) {
			prefill_from_oparray((char*) STR_NAME_VAL(opa->filename), opa TSRMLS_CC);
		}
	}

	return ZEND_HASH_APPLY_KEEP;
}

static int prefill_from_class_table(zend_class_entry *class_entry)
{
	zend_class_entry *ce;

	ce = class_entry;

	if (ce->type == ZEND_USER_CLASS) {
		if (!(ce->ce_flags & ZEND_XDEBUG_VISITED)) {
			zend_op_array *val;
			ce->ce_flags |= ZEND_XDEBUG_VISITED;
			ZEND_HASH_INC_APPLY_COUNT(&ce->function_table);
			ZEND_HASH_FOREACH_PTR(&ce->function_table, val) {
				prefill_from_function_table(val);
			} ZEND_HASH_FOREACH_END();
			ZEND_HASH_DEC_APPLY_COUNT(&ce->function_table);
		}
	}

	return ZEND_HASH_APPLY_KEEP;
}

void xdebug_prefill_code_coverage(zend_op_array *op_array TSRMLS_DC)
{
	zend_op_array    *function_op_array;
	zend_class_entry *class_entry;

	if ((long) op_array->reserved[XG(dead_code_analysis_tracker_offset)] < XG(dead_code_last_start_id)) {
		prefill_from_oparray((char*) STR_NAME_VAL(op_array->filename), op_array TSRMLS_CC);
	}

	ZEND_HASH_INC_APPLY_COUNT(CG(function_table));
	ZEND_HASH_FOREACH_PTR(CG(function_table), function_op_array) {
		prefill_from_function_table(function_op_array);
	} ZEND_HASH_FOREACH_END();
	ZEND_HASH_DEC_APPLY_COUNT(CG(function_table));

	ZEND_HASH_INC_APPLY_COUNT(CG(class_table));
	ZEND_HASH_FOREACH_PTR(CG(class_table), class_entry) {
		prefill_from_class_table(class_entry);
	} ZEND_HASH_FOREACH_END();
	ZEND_HASH_DEC_APPLY_COUNT(CG(class_table));
}

void xdebug_code_coverage_start_of_function(zend_op_array *op_array, char *function_name TSRMLS_DC)
{
	xdebug_path *path = xdebug_path_new(NULL);

	xdebug_prefill_code_coverage(op_array TSRMLS_CC);
	xdebug_path_info_add_path_for_level(XG(paths_stack), path, XG(level) TSRMLS_CC);

	if (XG(branches).size == 0 || XG(level) >= XG(branches).size) {
		XG(branches).size = XG(level) + 32;
		XG(branches).last_branch_nr = realloc(XG(branches).last_branch_nr, sizeof(int) * XG(branches.size));
	}

	XG(branches).last_branch_nr[XG(level)] = -1;
}

void xdebug_code_coverage_end_of_function(zend_op_array *op_array, char *file_name, char *function_name TSRMLS_DC)
{
	xdebug_str str = XDEBUG_STR_INITIALIZER;
	xdebug_path *path = xdebug_path_info_get_path_for_level(XG(paths_stack), XG(level) TSRMLS_CC);

	if (!path) {
		return;
	}

	xdebug_create_key_for_path(path, &str);

	xdebug_branch_info_mark_end_of_function_reached(file_name, function_name, str.d, str.l TSRMLS_CC);

	xdfree(str.d);

	if (path) {
		xdebug_path_free(path);
	}
}

PHP_FUNCTION(xdebug_start_code_coverage)
{
	zend_long options = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &options) == FAILURE) {
		return;
	}
	XG(code_coverage_unused) = (options & XDEBUG_CC_OPTION_UNUSED);
	XG(code_coverage_dead_code_analysis) = (options & XDEBUG_CC_OPTION_DEAD_CODE);
	XG(code_coverage_branch_check) = (options & XDEBUG_CC_OPTION_BRANCH_CHECK);

	if (!XG(extended_info)) {
		php_error(E_WARNING, "You can only use code coverage when you leave the setting of 'xdebug.extended_info' to the default '1'.");
		RETURN_FALSE;
	} else if (!XG(code_coverage)) {
		php_error(E_WARNING, "Code coverage needs to be enabled in php.ini by setting 'xdebug.coverage_enable' to '1'.");
		RETURN_FALSE;
	} else {
		XG(do_code_coverage) = 1;
		RETURN_TRUE;
	}
}

PHP_FUNCTION(xdebug_stop_code_coverage)
{
	zend_long cleanup = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &cleanup) == FAILURE) {
		return;
	}
	if (XG(do_code_coverage)) {
		if (cleanup) {
			XG(previous_filename) = NULL;
			XG(previous_file) = NULL;
			XG(previous_mark_filename) = NULL;
			XG(previous_mark_file) = NULL;
			xdebug_hash_destroy(XG(code_coverage));
			XG(code_coverage) = xdebug_hash_alloc(32, xdebug_coverage_file_dtor);
			XG(dead_code_last_start_id)++;
			xdebug_path_info_dtor(XG(paths_stack));
			XG(paths_stack) = xdebug_path_info_ctor();
		}
		XG(do_code_coverage) = 0;
		RETURN_TRUE;
	}
	RETURN_FALSE;
}


static int xdebug_lineno_cmp(const void *a, const void *b TSRMLS_DC)
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

static void add_branches(zval *retval, xdebug_branch_info *branch_info TSRMLS_DC)
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

static void add_paths(zval *retval, xdebug_branch_info *branch_info TSRMLS_DC)
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
	TSRMLS_FETCH();

	XDEBUG_MAKE_STD_ZVAL(function_info);
	array_init(function_info);

	if (function->branch_info) {
		add_branches(function_info, function->branch_info TSRMLS_CC);
		add_paths(function_info, function->branch_info TSRMLS_CC);
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
	TSRMLS_FETCH();

	/* Add all the lines */
	XDEBUG_MAKE_STD_ZVAL(lines);
	array_init(lines);

	xdebug_hash_apply(file->lines, (void *) lines, add_line);

	/* Sort on linenumber */
	target_hash = HASH_OF(lines);
	zend_hash_sort(target_hash, xdebug_lineno_cmp, 0 TSRMLS_CC);

	/* Add the branch and path info */
	if (file->has_branch_info) {
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
	xdebug_hash_apply(XG(code_coverage), (void *) return_value, add_file);
}

PHP_FUNCTION(xdebug_get_function_count)
{
	RETURN_LONG(XG(function_count));
}

PHP_FUNCTION(xdebug_code_coverage_started)
{
	RETURN_BOOL(XG(do_code_coverage));
}
