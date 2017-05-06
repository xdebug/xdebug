/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2016 Derick Rethans                               |
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

#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_set.h"
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
#if PHP_VERSION_ID >= 70000
	zend_op_array *op_array = &execute_data->func->op_array;
#else
	zend_op_array *op_array = execute_data->op_array;
#endif
	char *file = (char*) STR_NAME_VAL(op_array->filename);
	xdebug_func func_info;
	char *function_name;
#if PHP_VERSION_ID >= 70000
	long opnr = execute_data->opline - execute_data->func->op_array.opcodes;
#else
	long opnr = execute_data->opline - execute_data->op_array->opcodes;
#endif

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

int xdebug_check_branch_entry_handler(ZEND_USER_OPCODE_HANDLER_ARGS)
{
	if (XG(do_code_coverage) && XG(code_coverage_branch_check)) {
		const zend_op *cur_opcode;
#if PHP_VERSION_ID >= 70000
		cur_opcode = execute_data->opline;
#else
		cur_opcode = *EG(opline_ptr);
#endif

		xdebug_print_opcode_info('G', execute_data, cur_opcode TSRMLS_CC);
	}
	return ZEND_USER_OPCODE_DISPATCH;
}

#define XDEBUG_OPCODE_OVERRIDE(f) \
	int xdebug_##f##_handler(ZEND_USER_OPCODE_HANDLER_ARGS) \
	{ \
		return xdebug_common_override_handler(ZEND_USER_OPCODE_HANDLER_ARGS_PASSTHRU); \
	}


int xdebug_common_override_handler(ZEND_USER_OPCODE_HANDLER_ARGS)
{
	if (XG(do_code_coverage)) {
		const zend_op *cur_opcode;
		int      lineno;
		char    *file;

#if PHP_VERSION_ID >= 70000
		zend_op_array *op_array = &execute_data->func->op_array;
		cur_opcode = execute_data->opline;
		lineno = cur_opcode->lineno;
#else
		zend_op_array *op_array = execute_data->op_array;
		cur_opcode = *EG(opline_ptr);
		lineno = cur_opcode->lineno;
#endif
		file = (char*) STR_NAME_VAL(op_array->filename);

		xdebug_print_opcode_info('C', execute_data, cur_opcode TSRMLS_CC);
		xdebug_count_line(file, lineno, 0, 0 TSRMLS_CC);
	}
	return ZEND_USER_OPCODE_DISPATCH;
}

#if PHP_VERSION_ID >= 70000
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
#else
static int xdebug_is_static_call(const zend_op *cur_opcode, const zend_op *prev_opcode, const zend_op **found_opcode)
{
	const zend_op *opcode_ptr;

	opcode_ptr = prev_opcode;
	while (opcode_ptr->opcode == ZEND_FETCH_DIM_W || opcode_ptr->opcode == ZEND_FETCH_OBJ_W || opcode_ptr->opcode == ZEND_FETCH_W || opcode_ptr->opcode == ZEND_FETCH_RW) {
		opcode_ptr = opcode_ptr - 1;
	}
	opcode_ptr++;

	if (opcode_ptr->op1_type == IS_CONST && opcode_ptr->extended_value == ZEND_FETCH_STATIC_MEMBER)
	{
		*found_opcode = cur_opcode;
		return 1;
	}
	return 0;
}
#endif

static char *xdebug_find_var_name(zend_execute_data *execute_data TSRMLS_DC)
{
	const zend_op *cur_opcode, *next_opcode, *prev_opcode = NULL, *opcode_ptr;
	zval          *dimval;
	int            is_var;
#if PHP_VERSION_ID >= 70000
	zend_op_array *op_array = &execute_data->func->op_array;
#else
	int cv_len;
	zend_op_array *op_array = execute_data->op_array;
#endif
	xdebug_str     name = XDEBUG_STR_INITIALIZER;
	int            gohungfound = 0, is_static = 0;
	char          *zval_value = NULL;
	xdebug_var_export_options *options;
	const zend_op *static_opcode_ptr;

#if PHP_VERSION_ID >= 70000
	cur_opcode = execute_data->opline;
#else
	cur_opcode = *EG(opline_ptr);
#endif
	next_opcode = cur_opcode + 1;
	prev_opcode = cur_opcode - 1;

	if (cur_opcode->opcode == ZEND_QM_ASSIGN) {
#if PHP_VERSION_ID >= 70000
		xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->result.var)->val), 1);
#else
		xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->result.var, &cv_len)), 1);
#endif
	}

	if (cur_opcode->op1_type == IS_VAR &&
			(next_opcode->op1_type == IS_VAR || cur_opcode->op2_type == IS_VAR) &&
			prev_opcode->opcode == ZEND_FETCH_RW &&
			prev_opcode->op1_type == IS_CONST &&
#if PHP_VERSION_ID >= 70000
			Z_TYPE_P(RT_CONSTANT_EX(op_array->literals, prev_opcode->op1)) == IS_STRING
#else
			Z_TYPE_P(prev_opcode->op1.zv) == IS_STRING
#endif
	) {
#if PHP_VERSION_ID >= 70000
		xdebug_str_add(&name, xdebug_sprintf("$%s", Z_STRVAL_P(RT_CONSTANT_EX(op_array->literals, prev_opcode->op1))), 1);
#else
		xdebug_str_add(&name, xdebug_sprintf("$%s", Z_STRVAL_P(prev_opcode->op1.zv)), 1);
#endif
	}

	is_static = xdebug_is_static_call(cur_opcode, prev_opcode, &static_opcode_ptr);
	options = xdebug_var_export_options_from_ini(TSRMLS_C);
	options->no_decoration = 1;

	if (cur_opcode->op1_type == IS_CV) {
#if PHP_VERSION_ID >= 70000
		xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->op1.var)->val), 1);
#else
		xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->op1.var, &cv_len)), 1);
#endif
	} else if (cur_opcode->op1_type == IS_VAR && cur_opcode->opcode == ZEND_ASSIGN && (prev_opcode->opcode == ZEND_FETCH_W || prev_opcode->opcode == ZEND_FETCH_RW)) {
		if (is_static) {
			xdebug_str_add(&name, xdebug_sprintf("self::"), 1);
		} else {
			zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, prev_opcode->op1_type, &prev_opcode->op1, &is_var), 0, options);
			xdebug_str_add(&name, xdebug_sprintf("$%s", zval_value), 1);
		}
	} else if (is_static) { /* todo : see if you can change this and the previous cases around */
		xdebug_str_add(&name, xdebug_sprintf("self::"), 1 );
	}
	if ((cur_opcode->opcode >= ZEND_ASSIGN_ADD && cur_opcode->opcode <= ZEND_ASSIGN_BW_XOR)
#if PHP_VERSION_ID >= 50600
		|| cur_opcode->opcode == ZEND_ASSIGN_POW
#endif
	) {
		if (cur_opcode->extended_value == ZEND_ASSIGN_OBJ) {
			zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var), 0, options);
			if (cur_opcode->op1_type == IS_UNUSED) {
				xdebug_str_add(&name, xdebug_sprintf("$this->%s", zval_value), 1);
			} else {
				xdebug_str_add(&name, xdebug_sprintf("->%s", zval_value), 1);
			}
		} else if (cur_opcode->extended_value == ZEND_ASSIGN_DIM) {
			zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var), 0, NULL);
			xdebug_str_add(&name,xdebug_sprintf("[%s]", zval_value), 1);
		}
	}
	if (cur_opcode->opcode >= ZEND_PRE_INC_OBJ && cur_opcode->opcode <= ZEND_POST_DEC_OBJ) {
		zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var), 0, options);
		xdebug_str_add(&name, xdebug_sprintf("$this->%s", zval_value), 1);
	}
	if (zval_value) {
		xdfree(zval_value);
		zval_value = NULL;
	}

	/* Scroll back to start of FETCHES */
	/* FIXME: See whether we can do this unroll looping only once - in is_static() */
	gohungfound = 0;
#if PHP_VERSION_ID >= 70000
	if (!is_static) {
#endif
		opcode_ptr = prev_opcode;
		while (opcode_ptr->opcode == ZEND_FETCH_DIM_W || opcode_ptr->opcode == ZEND_FETCH_OBJ_W || opcode_ptr->opcode == ZEND_FETCH_W || opcode_ptr->opcode == ZEND_FETCH_RW) {
			opcode_ptr = opcode_ptr - 1;
			gohungfound = 1;
		}
		opcode_ptr = opcode_ptr + 1;
#if PHP_VERSION_ID >= 70000
	} else { /* if we have a static method, we should already have found the first fetch */
		opcode_ptr = static_opcode_ptr;
		gohungfound = 1;
	}
#endif

	if (gohungfound) {
		do
		{
			if (opcode_ptr->op1_type == IS_UNUSED && opcode_ptr->opcode == ZEND_FETCH_OBJ_W) {
				xdebug_str_add(&name, "$this", 0);
			}
			if (opcode_ptr->op1_type == IS_CV) {
#if PHP_VERSION_ID >= 70000
				xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, opcode_ptr->op1.var)->val), 1);
#else
				xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, opcode_ptr->op1.var, &cv_len)), 1);
#endif
			}
#if PHP_VERSION_ID >= 70100
			if (opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_W || opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_R || opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_RW) {
				zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->op1_type, &opcode_ptr->op1, &is_var), 0, options);
				xdebug_str_add(&name, xdebug_sprintf("%s", zval_value), 1);
			}
#endif
			if (opcode_ptr->opcode == ZEND_FETCH_W) {
				zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->op1_type, &opcode_ptr->op1, &is_var), 0, options);
				xdebug_str_add(&name, xdebug_sprintf("%s", zval_value), 1);
			}
			if (is_static && opcode_ptr->opcode == ZEND_FETCH_RW) {
				zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->op1_type, &opcode_ptr->op1, &is_var), 0, options);
				xdebug_str_add(&name, xdebug_sprintf("%s", zval_value), 1);
			}
			if (opcode_ptr->opcode == ZEND_FETCH_DIM_W) {
#if PHP_VERSION_ID < 70000
				if (opcode_ptr->op2_type != IS_VAR) {
#endif
					zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->op2_type, &opcode_ptr->op2, &is_var), 0, NULL);
					xdebug_str_add(&name, xdebug_sprintf("[%s]", zval_value), 1);
#if PHP_VERSION_ID < 70000
				} else {
					xdebug_str_add(&name, xdebug_sprintf("[???]") , 1);
				}
#endif
			} else if (opcode_ptr->opcode == ZEND_FETCH_OBJ_W) {
				zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->op2_type, &opcode_ptr->op2, &is_var), 0, options);
				xdebug_str_add(&name, xdebug_sprintf("->%s", zval_value), 1);
			}
			opcode_ptr = opcode_ptr + 1;
			if (zval_value) {
				xdfree(zval_value);
				zval_value = NULL;
			}
		} while (opcode_ptr->opcode == ZEND_FETCH_DIM_W || opcode_ptr->opcode == ZEND_FETCH_OBJ_W || opcode_ptr->opcode == ZEND_FETCH_W || opcode_ptr->opcode == ZEND_FETCH_RW);
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
			xdebug_str_add(&name, xdebug_sprintf("[%s]", zval_value), 1);
			xdfree(zval_value);
		}
	}

	xdfree(options->runtime);
	xdfree(options);

	return name.d;
}

static int xdebug_common_assign_dim_handler(char *op, int do_cc, ZEND_USER_OPCODE_HANDLER_ARGS)
{
	char    *file;
#if PHP_VERSION_ID >= 70000
	zend_op_array *op_array = &execute_data->func->op_array;
#else
	zend_op_array *op_array = execute_data->op_array;
#endif
	int            lineno;
	const zend_op *cur_opcode, *next_opcode;
	char          *full_varname;
	zval          *val = NULL;
	int            is_var;
	function_stack_entry *fse;

#if PHP_VERSION_ID >= 70000
	cur_opcode = execute_data->opline;
#else
	cur_opcode = *EG(opline_ptr);
#endif
	next_opcode = cur_opcode + 1;
	file = (char*) STR_NAME_VAL(op_array->filename);
	lineno = cur_opcode->lineno;

	if (XG(do_code_coverage)) {
		xdebug_print_opcode_info('=', execute_data, cur_opcode TSRMLS_CC);

		if (do_cc) {
			xdebug_count_line(file, lineno, 0, 0 TSRMLS_CC);
		}
	}
	if (XG(do_trace) && XG(trace_context) && XG(collect_assignments)) {
		full_varname = xdebug_find_var_name(execute_data TSRMLS_CC);

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
		} else {
			val = xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var);
		}

		fse = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack)));
		if (XG(do_trace) && XG(trace_context) && XG(collect_assignments) && XG(trace_handler)->assignment) {
			XG(trace_handler)->assignment(XG(trace_context), fse, full_varname, val, op, file, lineno TSRMLS_CC);
		}
		xdfree(full_varname);
	}
	return ZEND_USER_OPCODE_DISPATCH;
}

#define XDEBUG_OPCODE_OVERRIDE_ASSIGN(f,o,cc) \
	int xdebug_##f##_handler(ZEND_USER_OPCODE_HANDLER_ARGS) \
	{ \
		return xdebug_common_assign_dim_handler((o), (cc), ZEND_USER_OPCODE_HANDLER_ARGS_PASSTHRU); \
	}

XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign,"=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(qm_assign,"=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_add,"+=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_sub,"-=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_mul,"*=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_div,"/=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_mod,"%=",0)
#if PHP_VERSION_ID >= 50600
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_pow,"**=",0)
#endif
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

void xdebug_count_line(char *filename, int lineno, int executable, int deadcode TSRMLS_DC)
{
	xdebug_coverage_file *file;
	xdebug_coverage_line *line;

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
#if PHP_VERSION_ID >= 50500
		&& opcode.opcode != ZEND_FAST_CALL
#endif
#if PHP_VERSION_ID >= 50600
		&& opcode.opcode != ZEND_RECV_VARIADIC
#endif
	) {
		xdebug_count_line(fn, opcode.lineno, 1, deadcode TSRMLS_CC);
	}
}

#if PHP_VERSION_ID < 70000
static zend_brk_cont_element* xdebug_find_brk_cont(zend_uint nest_levels, int array_offset, zend_op_array *op_array)
{
	zend_brk_cont_element *jmp_to;

	do {
		if (array_offset == -1) {
			/* broken break/continue in code */
			return NULL;
		}
		jmp_to = &op_array->brk_cont_array[array_offset];
		array_offset = jmp_to->parent;
	} while (--nest_levels > 0);
	return jmp_to;
}
#endif

#define XDEBUG_ZNODE_ELEM(node,var) node.var
#if ZEND_USE_ABS_JMP_ADDR
# define XDEBUG_ZNODE_JMP_LINE(node, opline, base)  (int32_t)(((long)((node).jmp_addr) - (long)(base_address)) / sizeof(zend_op))
#else
# define XDEBUG_ZNODE_JMP_LINE(node, opline, base)  (int32_t)(((int32_t)((node).jmp_offset) / sizeof(zend_op)) + (opline))
#endif

static int xdebug_find_jump(zend_op_array *opa, unsigned int position, long *jmp1, long *jmp2)
{
#if PHP_VERSION_ID < 70000 || ZEND_USE_ABS_JMP_ADDR
	zend_op *base_address = &(opa->opcodes[0]);
#endif

	zend_op opcode = opa->opcodes[position];
	if (opcode.opcode == ZEND_JMP) {
#if PHP_VERSION_ID >= 70000
		*jmp1 = XDEBUG_ZNODE_JMP_LINE(opcode.op1, position, base_address);
#else
		*jmp1 = ((long) XDEBUG_ZNODE_ELEM(opcode.op1, jmp_addr) - (long) base_address) / sizeof(zend_op);
#endif
		return 1;
	} else if (
		opcode.opcode == ZEND_JMPZ ||
		opcode.opcode == ZEND_JMPNZ ||
		opcode.opcode == ZEND_JMPZ_EX ||
		opcode.opcode == ZEND_JMPNZ_EX
	) {
		*jmp1 = position + 1;
#if PHP_VERSION_ID >= 70000
		*jmp2 = XDEBUG_ZNODE_JMP_LINE(opcode.op2, position, base_address);
#else
		*jmp2 = ((long) XDEBUG_ZNODE_ELEM(opcode.op2, jmp_addr) - (long) base_address) / sizeof(zend_op);
#endif
		return 1;
	} else if (opcode.opcode == ZEND_JMPZNZ) {
#if PHP_VERSION_ID >= 70000
		*jmp1 = XDEBUG_ZNODE_JMP_LINE(opcode.op2, position, base_address);
		*jmp2 = position + ((int32_t) opcode.extended_value / (int32_t) sizeof(zend_op));
#else
		*jmp1 = XDEBUG_ZNODE_ELEM(opcode.op2, opline_num);
		*jmp2 = opcode.extended_value;
#endif
		return 1;
#if PHP_VERSION_ID < 70000
	} else if (opcode.opcode == ZEND_BRK || opcode.opcode == ZEND_CONT) {
		zend_brk_cont_element *el;

		if (opcode.op2_type == IS_CONST
#ifdef ZEND_ENGINE_2
		    && XDEBUG_ZNODE_ELEM(opcode.op1, jmp_addr) != (zend_op*) 0xFFFFFFFF
#endif
		) {
			el = xdebug_find_brk_cont(Z_LVAL_P(opcode.op2.zv), XDEBUG_ZNODE_ELEM(opcode.op1, opline_num), opa);
			if (el) {
				*jmp1 = opcode.opcode == ZEND_BRK ? el->brk : el->cont;
				return 1;
			} else {
				/* broken break/continue in code */
				return 0;
			}
		}
#endif
#if PHP_VERSION_ID >= 70000
	} else if (opcode.opcode == ZEND_FE_FETCH_R || opcode.opcode == ZEND_FE_FETCH_RW) {
		*jmp1 = position + 1;
		*jmp2 = position + (opcode.extended_value / sizeof(zend_op));
		return 1;
	} else if (opcode.opcode == ZEND_FE_RESET_R || opcode.opcode == ZEND_FE_RESET_RW) {
#else
	} else if (opcode.opcode == ZEND_FE_RESET || opcode.opcode == ZEND_FE_FETCH) {
#endif
		*jmp1 = position + 1;
#if PHP_VERSION_ID >= 70000
		*jmp2 = XDEBUG_ZNODE_JMP_LINE(opcode.op2, position, base_address);
#else
		*jmp2 = XDEBUG_ZNODE_ELEM(opcode.op2, opline_num);
#endif
		return 1;
	} else if (opcode.opcode == ZEND_CATCH) {
		*jmp1 = position + 1;
		if (!opcode.result.num) {
#if PHP_VERSION_ID >= 70100
			*jmp2 = position + (opcode.extended_value / sizeof(zend_op));
#else
			*jmp2 = opcode.extended_value;
#endif
			if (*jmp2 == *jmp1) {
				*jmp2 = XDEBUG_JMP_NOT_SET;
			}
		} else {
			*jmp2 = XDEBUG_JMP_EXIT;
		}
		return 1;
	} else if (opcode.opcode == ZEND_GOTO) {
#if PHP_VERSION_ID >= 70000
		*jmp1 = XDEBUG_ZNODE_JMP_LINE(opcode.op1, position, base_address);
#else
		*jmp1 = ((long) XDEBUG_ZNODE_ELEM(opcode.op1, jmp_addr) - (long) base_address) / sizeof(zend_op);
#endif
		return 1;

#if PHP_VERSION_ID >= 50500
	} else if (opcode.opcode == ZEND_FAST_CALL) {
#if PHP_VERSION_ID >= 70000
		*jmp1 = XDEBUG_ZNODE_JMP_LINE(opcode.op1, position, base_address);
#else
		*jmp1 = ((long) XDEBUG_ZNODE_ELEM(opcode.op1, jmp_addr) - (long) base_address) / sizeof(zend_op);
#endif
		*jmp2 = position + 1;
		return 1;
	} else if (opcode.opcode == ZEND_FAST_RET) {
		*jmp1 = XDEBUG_JMP_EXIT;
		return 1;
#endif

	} else if (
#if PHP_VERSION_ID >= 50500
		opcode.opcode == ZEND_GENERATOR_RETURN ||
#endif
		opcode.opcode == ZEND_EXIT ||
		opcode.opcode == ZEND_THROW ||
		opcode.opcode == ZEND_RETURN
	) {
		*jmp1 = XDEBUG_JMP_EXIT;
		return 1;
	}
	return 0;
}

static void xdebug_analyse_branch(zend_op_array *opa, unsigned int position, xdebug_set *set, xdebug_branch_info *branch_info TSRMLS_DC)
{
	long jump_pos1 = XDEBUG_JMP_NOT_SET;
	long jump_pos2 = XDEBUG_JMP_NOT_SET;

	/*(fprintf(stderr, "Branch analysis from position: %d\n", position);)*/

	if (branch_info) {
		xdebug_set_add(branch_info->starts, position);
		branch_info->branches[position].start_lineno = opa->opcodes[position].lineno;
	}

	/* First we see if the branch has been visited, if so we bail out. */
	if (xdebug_set_in(set, position)) {
		return;
	}

	/*(fprintf(stderr, "XDEBUG Adding %d\n", position);)*/
	/* Loop over the opcodes until the end of the array, or until a jump point has been found */
	xdebug_set_add(set, position);
	while (position < opa->last) {
		jump_pos1 = XDEBUG_JMP_NOT_SET;
		jump_pos2 = XDEBUG_JMP_NOT_SET;

		/* See if we have a jump instruction */
		if (xdebug_find_jump(opa, position, &jump_pos1, &jump_pos2)) {
			/*(fprintf(stderr, "XDEBUG Jump found. Position 1 = %d", jump_pos1);)*/
			if (jump_pos2 != XDEBUG_JMP_NOT_SET) {
				/*(fprintf(stderr, ", Position 2 = %d\n", jump_pos2))*/;
			} else {
				/*(fprintf(stderr, "\n"))*/;
			}
			if (jump_pos1 == XDEBUG_JMP_EXIT || jump_pos1 != XDEBUG_JMP_NOT_SET) {
				if (branch_info) {
					xdebug_branch_info_update(branch_info, position, opa->opcodes[position].lineno, 0, jump_pos1);
				}
				if (jump_pos1 != XDEBUG_JMP_EXIT) {
					xdebug_analyse_branch(opa, jump_pos1, set, branch_info TSRMLS_CC);
				}
			}
			if (jump_pos2 == XDEBUG_JMP_EXIT || jump_pos2 != XDEBUG_JMP_NOT_SET) {
				if (branch_info) {
					xdebug_branch_info_update(branch_info, position, opa->opcodes[position].lineno, 1, jump_pos2);
				}
				if (jump_pos2 != XDEBUG_JMP_EXIT) {
					xdebug_analyse_branch(opa, jump_pos2, set, branch_info TSRMLS_CC);
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
#if PHP_VERSION_ID >= 70000
	if (op_array->fn_flags & ZEND_ACC_ABSTRACT) {
#else
	if (op_array->last >= 3 && op_array->opcodes[op_array->last - 3].opcode == ZEND_RAISE_ABSTRACT_ERROR) {
#endif
		return;
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

#if PHP_VERSION_ID >= 70000
static int prefill_from_function_table(zend_op_array *opa)
#else
static int prefill_from_function_table(zend_op_array *opa TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
#endif
{
	if (opa->type == ZEND_USER_FUNCTION) {
		if ((long) opa->reserved[XG(dead_code_analysis_tracker_offset)] < XG(dead_code_last_start_id)) {
			prefill_from_oparray((char*) STR_NAME_VAL(opa->filename), opa TSRMLS_CC);
		}
	}

	return ZEND_HASH_APPLY_KEEP;
}

#if PHP_VERSION_ID >= 70000
static int prefill_from_class_table(zend_class_entry *class_entry)
#else
static int prefill_from_class_table(zend_class_entry **class_entry TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
#endif
{
#if PHP_VERSION_ID < 70000
	char *new_filename;
#endif
	zend_class_entry *ce;

#if PHP_VERSION_ID >= 70000
	ce = class_entry;
#else
	ce = *class_entry;
#endif

#if PHP_VERSION_ID < 70000
	new_filename = va_arg(args, char*);
#endif
	if (ce->type == ZEND_USER_CLASS) {
		if (!(ce->ce_flags & ZEND_XDEBUG_VISITED)) {
#if PHP_VERSION_ID >= 70000
			zend_op_array *val;
#endif
			ce->ce_flags |= ZEND_XDEBUG_VISITED;
#if PHP_VERSION_ID >= 70000
			ZEND_HASH_INC_APPLY_COUNT(&ce->function_table);
			ZEND_HASH_FOREACH_PTR(&ce->function_table, val) {
				prefill_from_function_table(val);
			} ZEND_HASH_FOREACH_END();
			ZEND_HASH_DEC_APPLY_COUNT(&ce->function_table);
#else
			zend_hash_apply_with_arguments(&ce->function_table TSRMLS_CC, (apply_func_args_t) prefill_from_function_table, 1, new_filename);
#endif
		}
	}

	return ZEND_HASH_APPLY_KEEP;
}

void xdebug_prefill_code_coverage(zend_op_array *op_array TSRMLS_DC)
{
#if PHP_VERSION_ID >= 70000
	zend_op_array    *function_op_array;
	zend_class_entry *class_entry;
#endif

	if ((long) op_array->reserved[XG(dead_code_analysis_tracker_offset)] < XG(dead_code_last_start_id)) {
		prefill_from_oparray((char*) STR_NAME_VAL(op_array->filename), op_array TSRMLS_CC);
	}

#if PHP_VERSION_ID >= 70000
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
#else
	zend_hash_apply_with_arguments(CG(function_table)  TSRMLS_CC, (apply_func_args_t) prefill_from_function_table, 1, STR_NAME_VAL(op_array->filename));
	zend_hash_apply_with_arguments(CG(class_table) TSRMLS_CC, (apply_func_args_t) prefill_from_class_table, 1, STR_NAME_VAL(op_array->filename));
#endif
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
	zppLONG options = 0;

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
	zppLONG cleanup = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &cleanup) == FAILURE) {
		return;
	}
	if (XG(do_code_coverage)) {
		if (cleanup) {
			XG(previous_filename) = "";
			XG(previous_file) = NULL;
			XG(previous_mark_filename) = "";
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
#if PHP_VERSION_ID >= 70000
	Bucket *f = (Bucket *) a;
	Bucket *s = (Bucket *) b;
#else
	Bucket *f = *((Bucket **) a);
	Bucket *s = *((Bucket **) b);
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

static void add_branches(zval *retval, xdebug_branch_info *branch_info TSRMLS_DC)
{
	zval *branches, *branch, *out, *out_hit;
	unsigned int i;

	XDEBUG_MAKE_STD_ZVAL(branches);
	array_init(branches);

	for (i = 0; i < branch_info->starts->size; i++) {
		if (xdebug_set_in(branch_info->starts, i)) {
			XDEBUG_MAKE_STD_ZVAL(branch);
			array_init(branch);
			add_assoc_long(branch, "op_start", i);
			add_assoc_long(branch, "op_end", branch_info->branches[i].end_op);
			add_assoc_long(branch, "line_start", branch_info->branches[i].start_lineno);
			add_assoc_long(branch, "line_end", branch_info->branches[i].end_lineno);

			add_assoc_long(branch, "hit", branch_info->branches[i].hit);

			XDEBUG_MAKE_STD_ZVAL(out);
			array_init(out);
			if (branch_info->branches[i].out[0]) {
				add_index_long(out, 0, branch_info->branches[i].out[0]);
			}
			if (branch_info->branches[i].out[1]) {
				add_index_long(out, 1, branch_info->branches[i].out[1]);
			}
			add_assoc_zval(branch, "out", out);

			XDEBUG_MAKE_STD_ZVAL(out_hit);
			array_init(out_hit);
			if (branch_info->branches[i].out[0]) {
				add_index_long(out_hit, 0, branch_info->branches[i].out_hit[0]);
			}
			if (branch_info->branches[i].out[1]) {
				add_index_long(out_hit, 1, branch_info->branches[i].out_hit[1]);
			}
			add_assoc_zval(branch, "out_hit", out_hit);

			add_index_zval(branches, i, branch);
#if PHP_VERSION_ID >= 70000
			efree(out_hit);
			efree(out);
			efree(branch);
#endif
		}
	}

	add_assoc_zval_ex(retval, "branches", HASH_KEY_SIZEOF("branches"), branches);

#if PHP_VERSION_ID >= 70000
	efree(branches);
#endif
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

#if PHP_VERSION_ID >= 70000
		efree(path_container);
		efree(path);
#endif
	}

	add_assoc_zval_ex(retval, "paths", HASH_KEY_SIZEOF("paths"), paths);

#if PHP_VERSION_ID >= 70000
	efree(paths);
#endif
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

#if PHP_VERSION_ID >= 70000
	efree(function_info);
#endif
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
#if PHP_VERSION_ID >= 70000
	zend_hash_sort(target_hash, xdebug_lineno_cmp, 0 TSRMLS_CC);
#else
	zend_hash_sort(target_hash, zend_qsort, xdebug_lineno_cmp, 0 TSRMLS_CC);
#endif

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
#if PHP_VERSION_ID >= 70000
		efree(functions);
		efree(file_info);
#endif
	} else {
		add_assoc_zval_ex(retval, file->name, HASH_KEY_STRLEN(file->name), lines);
	}

#if PHP_VERSION_ID >= 70000
	efree(lines);
#endif
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
