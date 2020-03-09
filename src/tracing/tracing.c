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
#include "php.h"
#include "ext/standard/php_string.h"

#include "php_xdebug.h"
#include "tracing_private.h"
#include "trace_textual.h"
#include "trace_computerized.h"
#include "trace_html.h"

#include "lib/compat.h"
#include "lib/str.h"
#include "lib/var_export_line.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

static xdebug_trace_handler_t *xdebug_select_trace_handler(int options)
{
	xdebug_trace_handler_t *tmp;

	switch (XINI_TRACE(trace_format)) {
		case 0: tmp = &xdebug_trace_handler_textual; break;
		case 1: tmp = &xdebug_trace_handler_computerized; break;
		case 2: tmp = &xdebug_trace_handler_html; break;
		default:
			php_error(E_NOTICE, "A wrong value for xdebug.trace_format was selected (%d), defaulting to the textual format", (int) XINI_TRACE(trace_format));
			tmp = &xdebug_trace_handler_textual; break;
	}

	if (options & XDEBUG_TRACE_OPTION_COMPUTERIZED) {
		tmp = &xdebug_trace_handler_computerized;
	}
	if (options & XDEBUG_TRACE_OPTION_HTML) {
		tmp = &xdebug_trace_handler_html;
	}

	return tmp;
}

FILE *xdebug_trace_open_file(char *fname, char *script_filename, long options, char **used_fname)
{
	FILE *file;
	char *filename;

	if (fname && strlen(fname)) {
		filename = xdstrdup(fname);
	} else {
		if (!strlen(XINI_TRACE(trace_output_name)) ||
			xdebug_format_output_filename(&fname, XINI_TRACE(trace_output_name), script_filename) <= 0
		) {
			/* Invalid or empty xdebug.trace_output_name */
			return NULL;
		}
		if (IS_SLASH(XINI_TRACE(trace_output_dir)[strlen(XINI_TRACE(trace_output_dir)) - 1])) {
			filename = xdebug_sprintf("%s%s", XINI_TRACE(trace_output_dir), fname);
		} else {
			filename = xdebug_sprintf("%s%c%s", XINI_TRACE(trace_output_dir), DEFAULT_SLASH, fname);
		}
		xdfree(fname);
	}
	if (options & XDEBUG_TRACE_OPTION_APPEND) {
		file = xdebug_fopen(filename, "a", (options & XDEBUG_TRACE_OPTION_NAKED_FILENAME) ? NULL : "xt", used_fname);
	} else {
		file = xdebug_fopen(filename, "w", (options & XDEBUG_TRACE_OPTION_NAKED_FILENAME) ? NULL : "xt", used_fname);
	}
	xdfree(filename);

	return file;
}

static char* xdebug_start_trace(char* fname, char *script_filename, long options)
{
	if (XG_TRACE(trace_context)) {
		return NULL;
	}

	XG_TRACE(trace_handler) = xdebug_select_trace_handler(options);
	XG_TRACE(trace_context) = (void*) XG_TRACE(trace_handler)->init(fname, script_filename, options);

	if (XG_TRACE(trace_context)) {
		XG_TRACE(trace_handler)->write_header(XG_TRACE(trace_context));
		return xdstrdup(XG_TRACE(trace_handler)->get_filename(XG_TRACE(trace_context)));
	}

	return NULL;
}

static void xdebug_stop_trace(void)
{
	if (XG_TRACE(trace_context)) {
		XG_TRACE(trace_handler)->write_footer(XG_TRACE(trace_context));
		XG_TRACE(trace_handler)->deinit(XG_TRACE(trace_context));
		XG_TRACE(trace_context) = NULL;
	}
}

PHP_FUNCTION(xdebug_start_trace)
{
	char *fname = NULL;
	size_t fname_len = 0;
	char *trace_fname;
	zend_long options = XINI_TRACE(trace_options);

	if (!XG_TRACE(trace_context)) {
		function_stack_entry *fse;

		if (zend_parse_parameters(ZEND_NUM_ARGS(), "|sl", &fname, &fname_len, &options) == FAILURE) {
			return;
		}

		fse = xdebug_get_stack_frame(0);

		if ((trace_fname = xdebug_start_trace(fname, fse->filename, options)) != NULL) {
			RETVAL_STRING(trace_fname);
			xdfree(trace_fname);
			return;
		} else {
			php_error(E_NOTICE, "Trace could not be started");
		}

		RETURN_FALSE;
	} else {
		php_error(E_NOTICE, "Function trace already started");
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_stop_trace)
{
	if (XG_TRACE(trace_context)) {
		RETVAL_STRING(XG_TRACE(trace_handler)->get_filename(XG_TRACE(trace_context)));
		xdebug_stop_trace();
	} else {
		RETVAL_FALSE;
		php_error(E_NOTICE, "Function trace was not started");
	}
}

PHP_FUNCTION(xdebug_get_tracefile_name)
{
	if (XG_TRACE(trace_context) && XG_TRACE(trace_handler) && XG_TRACE(trace_handler)->get_filename) {
		RETVAL_STRING(XG_TRACE(trace_handler)->get_filename(XG_TRACE(trace_context)));
	} else {
		RETURN_FALSE;
	}
}

static int xdebug_include_or_eval_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	const zend_op *opline = execute_data->opline;

	xdebug_coverage_record_include_if_active(execute_data, op_array);
	if (opline->extended_value == ZEND_EVAL) {
		zval *inc_filename;
		zval tmp_inc_filename;
		int  is_var;

		inc_filename = xdebug_get_zval(execute_data, opline->op1_type, &opline->op1, &is_var);

		/* If there is no inc_filename, we're just bailing out instead */
		if (!inc_filename) {
			return xdebug_call_original_opcode_handler_if_set(opline->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
		}

		if (Z_TYPE_P(inc_filename) != IS_STRING) {
			tmp_inc_filename = *inc_filename;
			zval_copy_ctor(&tmp_inc_filename);
			convert_to_string(&tmp_inc_filename);
			inc_filename = &tmp_inc_filename;
		}

		/* Now let's store this info */
		if (XG_BASE(last_eval_statement)) {
			efree(XG_BASE(last_eval_statement));
		}
		XG_BASE(last_eval_statement) = estrndup(Z_STRVAL_P(inc_filename), Z_STRLEN_P(inc_filename));

		if (inc_filename == &tmp_inc_filename) {
			zval_dtor(&tmp_inc_filename);
		}
	}

	return xdebug_call_original_opcode_handler_if_set(opline->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}

#if PHP_VERSION_ID >= 70400
static const char *get_assign_operation(uint32_t extended_value)
{
	switch (extended_value) {
		case ZEND_ADD:    return "+=";
		case ZEND_SUB:    return "-=";
		case ZEND_MUL:    return "*=";
		case ZEND_DIV:    return "/=";
		case ZEND_MOD:    return "%=";
		case ZEND_SL:     return "<<=";
		case ZEND_SR:     return ">>=";
		case ZEND_CONCAT: return ".=";
		case ZEND_BW_OR:  return "|=";
		case ZEND_BW_AND: return "&=";
		case ZEND_BW_XOR: return "^=";
		case ZEND_POW:    return "**=";
		default:
			return "";
	}
}
#endif

static int xdebug_is_static_call(const zend_op *cur_opcode, const zend_op *prev_opcode, const zend_op **found_opcode)
{
	const zend_op *opcode_ptr;

	opcode_ptr = cur_opcode;

# if PHP_VERSION_ID >= 70400
	if (
		(opcode_ptr->opcode == ZEND_ASSIGN_STATIC_PROP) || (opcode_ptr->opcode == ZEND_ASSIGN_STATIC_PROP_REF) ||
		(opcode_ptr->opcode == ZEND_PRE_INC_STATIC_PROP) || (opcode_ptr->opcode == ZEND_PRE_DEC_STATIC_PROP) ||
		(opcode_ptr->opcode == ZEND_POST_INC_STATIC_PROP) || (opcode_ptr->opcode == ZEND_POST_DEC_STATIC_PROP)
	) {
		*found_opcode = opcode_ptr;
		return 1;
	}
# endif
	while (!(opcode_ptr->opcode == ZEND_EXT_STMT) && !((opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_W) || (opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_RW))) {
		opcode_ptr = opcode_ptr - 1;
	}
	if ((opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_W) || (opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_RW)) {
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

static int is_fetch_op(const zend_op *op)
{
	return (
		op->opcode == ZEND_FETCH_DIM_W || op->opcode == ZEND_FETCH_DIM_RW ||
		op->opcode == ZEND_FETCH_OBJ_W || op->opcode == ZEND_FETCH_OBJ_RW ||
		op->opcode == ZEND_FETCH_W || op->opcode == ZEND_FETCH_RW
	);
}

static char *xdebug_find_var_name(zend_execute_data *execute_data, const zend_op *cur_opcode, const zend_op *lower_bound)
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
		xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->result.var)->val), 1);
	}

	is_static = xdebug_is_static_call(cur_opcode, prev_opcode, &static_opcode_ptr);
	options = xdebug_var_export_options_from_ini();
	options->no_decoration = 1;

	if (cur_opcode->op1_type == IS_CV) {
		if (!lower_bound) {
			xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->op1.var)->val), 1);
		}
	} else if (cur_opcode->op1_type == IS_VAR && cur_opcode->opcode == ZEND_ASSIGN && (prev_opcode->opcode == ZEND_FETCH_W || prev_opcode->opcode == ZEND_FETCH_RW)) {
		if (is_static) {
			xdebug_str_add(&name, xdebug_sprintf("self::"), 1);
		} else {
			zval_value = xdebug_get_zval_value_line(xdebug_get_zval_with_opline(execute_data, prev_opcode, prev_opcode->op1_type, &prev_opcode->op1, &is_var), 0, options);
			xdebug_str_addc(&name, '$');
			xdebug_str_add_str(&name, zval_value);
			xdebug_str_free(zval_value);
		}
	} else if (is_static) { /* todo : see if you can change this and the previous cases around */
		xdebug_str_add(&name, xdebug_sprintf("self::"), 1 );
	}
	if (cur_opcode->opcode >= ZEND_PRE_INC_OBJ && cur_opcode->opcode <= ZEND_POST_DEC_OBJ) {
		zval_value = xdebug_get_zval_value_line(xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var), 0, options);
		xdebug_str_addl(&name, "$this->", 7, 0);
		xdebug_str_add_str(&name, zval_value);
		xdebug_str_free(zval_value);
	}
#if PHP_VERSION_ID >= 70400
	if (cur_opcode->opcode >= ZEND_PRE_INC_STATIC_PROP && cur_opcode->opcode <= ZEND_POST_DEC_STATIC_PROP) {
		zval_value = xdebug_get_zval_value_line(xdebug_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1, &is_var), 0, options);
		xdebug_str_add_str(&name, zval_value);
		xdebug_str_free(zval_value);
	}
#endif

	/* Scroll back to start of FETCHES */
	/* FIXME: See whether we can do this unroll looping only once - in is_static() */
	gohungfound = 0;
	if (!is_static) {
		if (cur_opcode == lower_bound) {
			gohungfound = 1;
		}
		opcode_ptr = prev_opcode;
		while ((opcode_ptr >= lower_bound) && is_fetch_op(opcode_ptr)) {
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
			if (
				opcode_ptr->op1_type == IS_UNUSED &&
				(opcode_ptr->opcode == ZEND_FETCH_OBJ_W || opcode_ptr->opcode == ZEND_FETCH_OBJ_RW)
			) {
				xdebug_str_add(&name, "$this", 0);
			}
			if (opcode_ptr->op1_type == IS_CV) {
				xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, opcode_ptr->op1.var)->val), 1);
			}
			if (opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_W || opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_R || opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_RW) {
				zval_value = xdebug_get_zval_value_line(xdebug_get_zval_with_opline(execute_data, opcode_ptr, opcode_ptr->op1_type, &opcode_ptr->op1, &is_var), 0, options);
				xdebug_str_add_str(&name, zval_value);
				xdebug_str_free(zval_value);
			}
			if (opcode_ptr->opcode == ZEND_FETCH_W) {
				zval_value = xdebug_get_zval_value_line(xdebug_get_zval_with_opline(execute_data, opcode_ptr, opcode_ptr->op1_type, &opcode_ptr->op1, &is_var), 0, options);
				xdebug_str_add_str(&name, zval_value);
				xdebug_str_free(zval_value);
			}
			if (is_static && opcode_ptr->opcode == ZEND_FETCH_RW) {
				zval_value = xdebug_get_zval_value_line(xdebug_get_zval_with_opline(execute_data, opcode_ptr, opcode_ptr->op1_type, &opcode_ptr->op1, &is_var), 0, options);
				xdebug_str_add_str(&name, zval_value);
				xdebug_str_free(zval_value);
			}
			if (opcode_ptr->opcode == ZEND_FETCH_DIM_W || opcode_ptr->opcode == ZEND_FETCH_DIM_RW) {
				zval_value = xdebug_get_zval_value_line(xdebug_get_zval_with_opline(execute_data, opcode_ptr, opcode_ptr->op2_type, &opcode_ptr->op2, &is_var), 0, NULL);
				xdebug_str_addc(&name, '[');
				if (zval_value) {
					xdebug_str_add_str(&name, zval_value);
				}
				xdebug_str_addc(&name, ']');
				xdebug_str_free(zval_value);
			} else if (opcode_ptr->opcode == ZEND_FETCH_OBJ_W || opcode_ptr->opcode == ZEND_FETCH_OBJ_RW) {
				zval_value = xdebug_get_zval_value_line(xdebug_get_zval_with_opline(execute_data, opcode_ptr, opcode_ptr->op2_type, &opcode_ptr->op2, &is_var), 0, options);
				xdebug_str_addl(&name, "->", 2, 0);
				xdebug_str_add_str(&name, zval_value);
				xdebug_str_free(zval_value);
			}
			opcode_ptr = opcode_ptr + 1;
			if (opcode_ptr->op1_type == IS_CV) {
				cv_found = 1;
			}
		} while (!cv_found && is_fetch_op(opcode_ptr));
	}

	if (
		(cur_opcode->opcode == ZEND_ASSIGN_OBJ)
#if PHP_VERSION_ID >= 70400
		|| (cur_opcode->opcode == ZEND_ASSIGN_OBJ_REF)
#endif
	) {
		if (cur_opcode->op1_type == IS_UNUSED) {
			xdebug_str_add(&name, "$this", 0);
		}
		dimval = xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var);
		xdebug_str_add(&name, xdebug_sprintf("->%s", Z_STRVAL_P(dimval)), 1);
	}
#if PHP_VERSION_ID >= 70400
	if (cur_opcode->opcode == ZEND_ASSIGN_STATIC_PROP_REF) {
		dimval = xdebug_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1, &is_var);
		xdebug_str_add(&name, xdebug_sprintf("%s", Z_STRVAL_P(dimval)), 1);
	}
	if (cur_opcode->opcode == ZEND_ASSIGN_DIM_OP) {
		zval_value = xdebug_get_zval_value_line(xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var), 0, NULL);
		xdebug_str_addc(&name, '[');
		xdebug_str_add_str(&name, zval_value);
		xdebug_str_addc(&name, ']');
		xdebug_str_free(zval_value);
	}
	if (cur_opcode->opcode == ZEND_ASSIGN_OBJ_OP) {
		zval_value = xdebug_get_zval_value_line(xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var), 0, options);
		if (cur_opcode->op1_type == IS_UNUSED) {
			xdebug_str_addl(&name, "$this->", 7, 0);
		} else {
			xdebug_str_addl(&name, "->", 2, 0);
		}
		xdebug_str_add_str(&name, zval_value);
		xdebug_str_free(zval_value);
	}
	if (cur_opcode->opcode == ZEND_ASSIGN_STATIC_PROP_OP) {
		zval_value = xdebug_get_zval_value_line(xdebug_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1, &is_var), 0, options);
		xdebug_str_addl(&name, "self::", 6, 0);
		xdebug_str_add_str(&name, zval_value);
		xdebug_str_free(zval_value);
	}
#else
	if ((cur_opcode->opcode >= ZEND_ASSIGN_ADD && cur_opcode->opcode <= ZEND_ASSIGN_BW_XOR)
		|| cur_opcode->opcode == ZEND_ASSIGN_POW
	) {
		if (cur_opcode->extended_value == ZEND_ASSIGN_DIM) {
			zval_value = xdebug_get_zval_value_line(xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var), 0, NULL);
			xdebug_str_addc(&name, '[');
			xdebug_str_add_str(&name, zval_value);
			xdebug_str_addc(&name, ']');
			xdebug_str_free(zval_value);
		}
		if (cur_opcode->extended_value == ZEND_ASSIGN_OBJ) {
			zval_value = xdebug_get_zval_value_line(xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var), 0, options);
			if (cur_opcode->op1_type == IS_UNUSED) {
				xdebug_str_addl(&name, "$this->", 7, 0);
			} else {
				xdebug_str_addl(&name, "->", 2, 0);
			}
			xdebug_str_add_str(&name, zval_value);
			xdebug_str_free(zval_value);
		}
	}
#endif

	if (cur_opcode->opcode == ZEND_ASSIGN_DIM) {
		if (next_opcode->opcode == ZEND_OP_DATA && cur_opcode->op2_type == IS_UNUSED) {
			xdebug_str_add(&name, "[]", 0);
		} else {
			zval_value = xdebug_get_zval_value_line(xdebug_get_zval_with_opline(execute_data, opcode_ptr, opcode_ptr->op2_type, &opcode_ptr->op2, &is_var), 0, NULL);
			xdebug_str_addc(&name, '[');
			xdebug_str_add_str(&name, zval_value);
			xdebug_str_addc(&name, ']');
			xdebug_str_free(zval_value);
		}
	}

#if PHP_VERSION_ID >= 70400
	if (cur_opcode->opcode == ZEND_ASSIGN_STATIC_PROP) {
		dimval = xdebug_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1, &is_var);
		xdebug_str_add(&name, xdebug_sprintf("%s", Z_STRVAL_P(dimval)), 1);
	}
#endif

	xdfree(options->runtime);
	xdfree(options);

	return name.d;
}

static int xdebug_common_assign_dim_handler(const char *op, int do_cc, XDEBUG_OPCODE_HANDLER_ARGS)
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
//		return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
//	}

	xdebug_coverage_record_assign_if_active(execute_data, op_array, do_cc);

	if (XG_TRACE(trace_context) && XINI_BASE(collect_assignments)) {
		char *full_varname;

		if (cur_opcode->opcode == ZEND_QM_ASSIGN && cur_opcode->result_type != IS_CV) {
			return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
		}
		full_varname = xdebug_find_var_name(execute_data, execute_data->opline, NULL);

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
#if PHP_VERSION_ID >= 70400
		} else if (cur_opcode->opcode >= ZEND_PRE_INC_STATIC_PROP && cur_opcode->opcode <= ZEND_POST_DEC_STATIC_PROP) {
			char *tmp_varname;

			switch (cur_opcode->opcode) {
				case ZEND_PRE_INC_STATIC_PROP:  tmp_varname = xdebug_sprintf("++%s", full_varname); break;
				case ZEND_POST_INC_STATIC_PROP: tmp_varname = xdebug_sprintf("%s++", full_varname); break;
				case ZEND_PRE_DEC_STATIC_PROP:  tmp_varname = xdebug_sprintf("--%s", full_varname); break;
				case ZEND_POST_DEC_STATIC_PROP: tmp_varname = xdebug_sprintf("%s--", full_varname); break;
			}
			xdfree(full_varname);
			full_varname = tmp_varname;

			val = xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var);
#endif
		} else if (
			(next_opcode->opcode == ZEND_OP_DATA)
#if PHP_VERSION_ID >= 70400
			&& (cur_opcode->opcode != ZEND_ASSIGN_OBJ_REF)
			&& (cur_opcode->opcode != ZEND_ASSIGN_STATIC_PROP_REF)
#endif
		) {
			val = xdebug_get_zval_with_opline(execute_data, next_opcode, next_opcode->op1_type, &next_opcode->op1, &is_var);
		} else if (cur_opcode->opcode == ZEND_QM_ASSIGN) {
			val = xdebug_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1, &is_var);
		} else if (cur_opcode->opcode == ZEND_ASSIGN_REF) {
			if (cur_opcode->op2_type == IS_CV) {
				right_full_varname = xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->op2.var)->val);
			} else {
				const zend_op *referenced_opline = xdebug_find_referenced_opline(execute_data, cur_opcode, 2);
				right_full_varname = xdebug_find_var_name(execute_data, referenced_opline, NULL);
			}
#if PHP_VERSION_ID >= 70400
		} else if (cur_opcode->opcode == ZEND_ASSIGN_OBJ_REF) {
			if (cur_opcode->op2_type == IS_CV) {
				right_full_varname = xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->op2.var)->val);
			} else {
				const zend_op *referenced_opline = xdebug_find_referenced_opline(execute_data, next_opcode, 1);
				right_full_varname = xdebug_find_var_name(execute_data, referenced_opline, NULL);
			}
		} else if (cur_opcode->opcode == ZEND_ASSIGN_STATIC_PROP_REF) {
			if (cur_opcode->op2_type == IS_CV) {
				right_full_varname = xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->op2.var)->val);
			} else {
				const zend_op *referenced_opline = xdebug_find_referenced_opline(execute_data, next_opcode, 1);
				right_full_varname = xdebug_find_var_name(execute_data, referenced_opline, NULL);
			}
#endif
		} else {
			val = xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var);
		}

		fse = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack)));
		if (XG_TRACE(trace_context) && XINI_BASE(collect_assignments) && XG_TRACE(trace_handler)->assignment) {
			XG_TRACE(trace_handler)->assignment(XG_TRACE(trace_context), fse, full_varname, val, right_full_varname, op, file, lineno);
		}
		xdfree(full_varname);
	}

	return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}

XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign,"=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(qm_assign,"=",1)
#if PHP_VERSION_ID >= 70400
XDEBUG_OPCODE_OVERRIDE_ASSIGN_OP(assign_op,0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN_OP(assign_dim_op,0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN_OP(assign_obj_op,0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN_OP(assign_static_prop_op,0)
#else
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_add,"+=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_sub,"-=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_mul,"*=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_div,"/=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_mod,"%=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_sl,"<<=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_sr,">>=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_bw_or,"|=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_bw_and,"&=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_bw_xor,"^=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_pow,"**=",0)
#endif
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_inc,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_inc,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_dec,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_dec,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_inc_obj,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_inc_obj,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_dec_obj,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_dec_obj,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_concat,".=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_dim,"=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_obj,"=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_ref,"=&",1)
#if PHP_VERSION_ID >= 70400
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_obj_ref,"=&",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_static_prop, "=", 0);
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_static_prop_ref, "=&", 0);
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_inc_static_prop, "", 0);
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_dec_static_prop, "", 0);
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_inc_static_prop, "", 0);
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_dec_static_prop, "", 0);
#endif

void xdebug_init_tracing_globals(xdebug_tracing_globals_t *xg)
{
	xg->trace_handler = NULL;
	xg->trace_context = NULL;
}

void xdebug_tracing_minit(INIT_FUNC_ARGS)
{
	/* Override opcodes for variable assignments in traces */
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(include_or_eval, ZEND_INCLUDE_OR_EVAL);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign, ZEND_ASSIGN);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(qm_assign, ZEND_QM_ASSIGN);
#if PHP_VERSION_ID >= 70400
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_op, ZEND_ASSIGN_OP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_dim_op, ZEND_ASSIGN_DIM_OP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_obj_op, ZEND_ASSIGN_OBJ_OP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_static_prop_op, ZEND_ASSIGN_STATIC_PROP_OP);
#else
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_add, ZEND_ASSIGN_ADD);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_sub, ZEND_ASSIGN_SUB);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_mul, ZEND_ASSIGN_MUL);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_div, ZEND_ASSIGN_DIV);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_mod, ZEND_ASSIGN_MOD);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_pow, ZEND_ASSIGN_POW);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_sl, ZEND_ASSIGN_SL);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_sr, ZEND_ASSIGN_SR);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_concat, ZEND_ASSIGN_CONCAT);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_bw_or, ZEND_ASSIGN_BW_OR);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_bw_and, ZEND_ASSIGN_BW_AND);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_bw_xor, ZEND_ASSIGN_BW_XOR);
#endif
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_dim, ZEND_ASSIGN_DIM);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_obj, ZEND_ASSIGN_OBJ);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_ref, ZEND_ASSIGN_REF);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_inc, ZEND_PRE_INC);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_inc, ZEND_POST_INC);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_dec, ZEND_PRE_DEC);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_dec, ZEND_POST_DEC);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_inc_obj, ZEND_PRE_INC_OBJ);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_inc_obj, ZEND_POST_INC_OBJ);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_dec_obj, ZEND_PRE_DEC_OBJ);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_dec_obj, ZEND_POST_DEC_OBJ);
#if PHP_VERSION_ID >= 70400
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_obj_ref, ZEND_ASSIGN_OBJ_REF);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_static_prop, ZEND_ASSIGN_STATIC_PROP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_static_prop_ref, ZEND_ASSIGN_STATIC_PROP_REF);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_inc_static_prop, ZEND_PRE_INC_STATIC_PROP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_dec_static_prop, ZEND_PRE_DEC_STATIC_PROP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_inc_static_prop, ZEND_POST_INC_STATIC_PROP);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_dec_static_prop, ZEND_POST_DEC_STATIC_PROP);
#endif

	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_APPEND", XDEBUG_TRACE_OPTION_APPEND, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_COMPUTERIZED", XDEBUG_TRACE_OPTION_COMPUTERIZED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_HTML", XDEBUG_TRACE_OPTION_HTML, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_NAKED_FILENAME", XDEBUG_TRACE_OPTION_NAKED_FILENAME, CONST_CS | CONST_PERSISTENT);
}

void xdebug_tracing_rinit(void)
{
	XG_TRACE(trace_handler) = NULL;
	XG_TRACE(trace_context) = NULL;
}

void xdebug_tracing_post_deactivate(void)
{
	if (XG_TRACE(trace_context)) {
		xdebug_stop_trace();
	}

	XG_TRACE(trace_context) = NULL;
}

void xdebug_tracing_init_if_requested(zend_op_array *op_array)
{
	if (
		(XINI_TRACE(auto_trace) || xdebug_trigger_enabled(XINI_TRACE(trace_enable_trigger), "XDEBUG_TRACE", XINI_TRACE(trace_enable_trigger_value)))
		&& XINI_TRACE(trace_output_dir) && strlen(XINI_TRACE(trace_output_dir))
	) {
		/* In case we do an auto-trace we are not interested in the return
		 * value, but we still have to free it. */
		xdfree(xdebug_start_trace(NULL, STR_NAME_VAL(op_array->filename), XINI_TRACE(trace_options)));
	}
}

void xdebug_tracing_execute_ex(int function_nr, function_stack_entry *fse)
{
	if (fse->filtered_tracing || !XG_TRACE(trace_context)) {
		return;
	}

	if (XG_TRACE(trace_handler)->function_entry) {
		XG_TRACE(trace_handler)->function_entry(XG_TRACE(trace_context), fse, function_nr);
	}
}

void xdebug_tracing_execute_ex_end(int function_nr, function_stack_entry *fse, zend_execute_data *execute_data)
{
	if (fse->filtered_tracing || !XG_TRACE(trace_context)) {
		return;
	}

	if ((XG_TRACE(trace_handler)->function_exit)) {
		XG_TRACE(trace_handler)->function_exit(XG_TRACE(trace_context), fse, function_nr);
	}

	/* Store return value in the trace file */
	if (XINI_BASE(collect_return)) {
		zend_op_array *op_array = &(execute_data->func->op_array);

		if (execute_data && execute_data->return_value) {
			if (op_array->fn_flags & ZEND_ACC_GENERATOR) {
				if (XG_TRACE(trace_handler)->generator_return_value) {
					XG_TRACE(trace_handler)->generator_return_value(XG_TRACE(trace_context), fse, function_nr, (zend_generator*) execute_data->return_value);
				}
			} else {
				if (XG_TRACE(trace_handler)->return_value) {
					XG_TRACE(trace_handler)->return_value(XG_TRACE(trace_context), fse, function_nr, execute_data->return_value);
				}
			}
		}
	}
}

int xdebug_tracing_execute_internal(int function_nr, function_stack_entry *fse)
{
	if (fse->filtered_tracing || !XG_TRACE(trace_context)) {
		return 0;
	}

	if (fse->function.type != XFUNC_ZEND_PASS && (XG_TRACE(trace_handler)->function_entry)) {
		XG_TRACE(trace_handler)->function_entry(XG_TRACE(trace_context), fse, function_nr);
		return 1;
	}

	return 0;
}

void xdebug_tracing_execute_internal_end(int function_nr, function_stack_entry *fse, zval *return_value)
{
	if (fse->filtered_tracing || !XG_TRACE(trace_context)) {
		return;
	}

	if (fse->function.type != XFUNC_ZEND_PASS && (XG_TRACE(trace_handler)->function_exit)) {
		XG_TRACE(trace_handler)->function_exit(XG_TRACE(trace_context), fse, function_nr);
	}

	/* Store return value in the trace file */
	if (XINI_BASE(collect_return) && fse->function.type != XFUNC_ZEND_PASS && return_value && XG_TRACE(trace_handler)->return_value) {
		XG_TRACE(trace_handler)->return_value(XG_TRACE(trace_context), fse, function_nr, return_value);
	}
}

void xdebug_tracing_save_trace_context(void **original_trace_context)
{
	*original_trace_context = XG_TRACE(trace_context);
	XG_TRACE(trace_context) = NULL;
}

void xdebug_tracing_restore_trace_context(void *original_trace_context)
{
	XG_TRACE(trace_context) = original_trace_context;
}
