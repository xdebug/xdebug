/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2023 Derick Rethans                               |
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
#include "lib/php-header.h"
#include "ext/standard/php_string.h"

#include "php_xdebug.h"
#include "tracing_private.h"
#include "trace_textual.h"
#include "trace_flamegraph.h"
#include "trace_computerized.h"
#include "trace_html.h"

#include "lib/compat.h"
#include "lib/log.h"
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
		case 3: tmp = &xdebug_trace_handler_flamegraph_cost; break;
		case 4: tmp = &xdebug_trace_handler_flamegraph_mem; break;
		default:
			php_error(E_NOTICE, "A wrong value for xdebug.trace_format was selected (%d), defaulting to the textual format", (int) XINI_TRACE(trace_format));
			tmp = &xdebug_trace_handler_textual; break;
	}

	/* Override handler based on options */
	if (options & XDEBUG_TRACE_OPTION_FLAMEGRAPH_COST) {
		tmp = &xdebug_trace_handler_flamegraph_cost;
	}
	if (options & XDEBUG_TRACE_OPTION_FLAMEGRAPH_MEM) {
		tmp = &xdebug_trace_handler_flamegraph_mem;
	}
	if (options & XDEBUG_TRACE_OPTION_COMPUTERIZED) {
		tmp = &xdebug_trace_handler_computerized;
	}
	if (options & XDEBUG_TRACE_OPTION_HTML) {
		tmp = &xdebug_trace_handler_html;
	}

	if (!tmp->init || !tmp->deinit || !tmp->get_filename) {
		xdebug_log_ex(XLOG_CHAN_TRACE, XLOG_CRIT, "HNDLR", "Broken trace handler for format '%d', missing 'init', 'deinit', or 'get_filename'  handler", options);
	}

	return tmp;
}

xdebug_file *xdebug_trace_open_file(char *requested_filename, zend_string *script_filename, long options)
{
	xdebug_file *file = xdebug_file_ctor();
	char *filename_to_use;
	char *generated_filename = NULL;
	char *output_dir = xdebug_lib_get_output_dir(); /* not duplicated */

	if (requested_filename && strlen(requested_filename)) {
		filename_to_use = xdstrdup(requested_filename);
	} else {

		if (!strlen(XINI_TRACE(trace_output_name)) ||
			xdebug_format_output_filename(&generated_filename, XINI_TRACE(trace_output_name), ZSTR_VAL(script_filename)) <= 0
		) {
			/* Invalid or empty xdebug.trace_output_name */
			xdebug_file_dtor(file);
			return NULL;
		}

		/* Add a slash if none is present in the output_dir setting */
		output_dir = xdebug_lib_get_output_dir(); /* not duplicated */

		if (IS_SLASH(output_dir[strlen(output_dir) - 1])) {
			filename_to_use = xdebug_sprintf("%s%s", output_dir, generated_filename);
		} else {
			filename_to_use = xdebug_sprintf("%s%c%s", output_dir, DEFAULT_SLASH, generated_filename);
		}
	}

	if (!xdebug_file_open(
		file,
		filename_to_use,
		(options & XDEBUG_TRACE_OPTION_NAKED_FILENAME) ? NULL : "xt",
		(options & XDEBUG_TRACE_OPTION_APPEND) ? "ab" : "wb"
	)) {
		xdebug_log_diagnose_permissions(XLOG_CHAN_TRACE, output_dir, generated_filename);
	}

	if (generated_filename) {
		xdfree(generated_filename);
	}
	xdfree(filename_to_use);

	return file;
}

static char* xdebug_start_trace(char* fname, zend_string *script_filename, long options)
{
	if (XG_TRACE(trace_context)) {
		return NULL;
	}

	XG_TRACE(trace_handler) = xdebug_select_trace_handler(options);
	if (!XG_TRACE(trace_handler)) {
		return NULL;
	}

	XG_TRACE(trace_context) = (void*) XG_TRACE(trace_handler)->init(fname, script_filename, options);
	if (!XG_TRACE(trace_context)) {
		return NULL;
	}

	if (XG_TRACE(trace_handler)->write_header) {
		XG_TRACE(trace_handler)->write_header(XG_TRACE(trace_context));
	}

	return xdstrdup(XG_TRACE(trace_handler)->get_filename(XG_TRACE(trace_context)));
}

static void xdebug_stop_trace(void)
{
	if (!XG_TRACE(trace_context)) {
		return;
	}

	if (XG_TRACE(trace_handler)->write_footer) {
		XG_TRACE(trace_handler)->write_footer(XG_TRACE(trace_context));
	}
	XG_TRACE(trace_handler)->deinit(XG_TRACE(trace_context));
	XG_TRACE(trace_context) = NULL;
}

char *xdebug_get_trace_filename(void)
{
	if (!(XG_TRACE(trace_context) && XG_TRACE(trace_handler) && XG_TRACE(trace_handler)->get_filename)) {
		return NULL;
	}

	return XG_TRACE(trace_handler)->get_filename(XG_TRACE(trace_context));
}

PHP_FUNCTION(xdebug_start_trace)
{
	char *fname = NULL;
	size_t fname_len = 0;
	char *trace_fname;
	zend_long options = XINI_TRACE(trace_options);
	function_stack_entry *fse;

	WARN_AND_RETURN_IF_MODE_IS_NOT(XDEBUG_MODE_TRACING);

	if (XG_TRACE(trace_context)) {
		php_error(E_NOTICE, "Function trace already started");
		RETURN_FALSE;
	}

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
}

PHP_FUNCTION(xdebug_stop_trace)
{
	WARN_AND_RETURN_IF_MODE_IS_NOT(XDEBUG_MODE_TRACING);

	if (!XG_TRACE(trace_context)) {
		php_error(E_NOTICE, "Function trace was not started");
		RETURN_FALSE;
	}

	RETVAL_STRING(XG_TRACE(trace_handler)->get_filename(XG_TRACE(trace_context)));
	xdebug_stop_trace();
}

PHP_FUNCTION(xdebug_get_tracefile_name)
{
	char *filename;

	WARN_AND_RETURN_IF_MODE_IS_NOT(XDEBUG_MODE_TRACING);

	filename = xdebug_get_trace_filename();
	if (!filename) {
		RETURN_FALSE;
	}

	RETVAL_STRING(filename);
}

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

static int xdebug_is_static_call(const zend_op *first_opcode, const zend_op *cur_opcode, const zend_op *prev_opcode, const zend_op **found_opcode)
{
	const zend_op *opcode_ptr;

	opcode_ptr = cur_opcode;

	if (
		(opcode_ptr->opcode == ZEND_ASSIGN_STATIC_PROP) || (opcode_ptr->opcode == ZEND_ASSIGN_STATIC_PROP_REF) ||
		(opcode_ptr->opcode == ZEND_PRE_INC_STATIC_PROP) || (opcode_ptr->opcode == ZEND_PRE_DEC_STATIC_PROP) ||
		(opcode_ptr->opcode == ZEND_POST_INC_STATIC_PROP) || (opcode_ptr->opcode == ZEND_POST_DEC_STATIC_PROP)
	) {
		*found_opcode = opcode_ptr;
		return 1;
	}

	while (!(opcode_ptr->opcode == ZEND_EXT_STMT) && !((opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_W) || (opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_RW))) {
		opcode_ptr = opcode_ptr - 1;
		if (opcode_ptr < first_opcode) {
			return 0;
		}
	}
	if ((opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_W) || (opcode_ptr->opcode == ZEND_FETCH_STATIC_PROP_RW)) {
		*found_opcode = opcode_ptr;
		return 1;
	}
	return 0;
}

static const zend_op *xdebug_find_referenced_opline(zend_execute_data *execute_data, const zend_op *cur_opcode, int op1_or_op2)
{
	size_t variable_number;
	const zend_op *scan_opcode;
	int found;
	int op_type = (op1_or_op2 == 1) ? cur_opcode->op1_type : cur_opcode->op2_type;

	if (op_type != IS_VAR) {
		return NULL;
	}

	variable_number = (op1_or_op2 == 1) ? cur_opcode->op1.var : cur_opcode->op2.var;
	scan_opcode = cur_opcode;
	found = 0;

	/* Scroll up until we find a RES of IS_VAR with the right value */
	do {
		scan_opcode--;
		if (scan_opcode->result_type == IS_VAR && scan_opcode->result.var == variable_number) {
			found = 1;
		}
	} while (!found);

	return scan_opcode;
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
		xdebug_str_addc(&name, '$');
		xdebug_str_add(&name, zend_get_compiled_variable_name(op_array, cur_opcode->result.var)->val, 0);

		return name.d;
	}

	is_static = xdebug_is_static_call(op_array->opcodes, cur_opcode, prev_opcode, &static_opcode_ptr);
	options = xdebug_var_export_options_from_ini();
	options->no_decoration = 1;

	if (cur_opcode->op1_type == IS_CV) {
		if (!lower_bound) {
			xdebug_str_addc(&name, '$');
			xdebug_str_add(&name, zend_get_compiled_variable_name(op_array, cur_opcode->op1.var)->val, 0);
		}
	} else if (cur_opcode->op1_type == IS_VAR && cur_opcode->opcode == ZEND_ASSIGN && (prev_opcode->opcode == ZEND_FETCH_W || prev_opcode->opcode == ZEND_FETCH_RW)) {
		if (is_static) {
			xdebug_str_add_literal(&name, "self::");
		} else {
			zval_value = xdebug_get_zval_value_line(xdebug_get_zval_with_opline(execute_data, prev_opcode, prev_opcode->op1_type, &prev_opcode->op1, &is_var), 0, options);
			xdebug_str_addc(&name, '$');
			xdebug_str_add_str(&name, zval_value);
			xdebug_str_free(zval_value);
		}
	} else if (is_static) { /* todo : see if you can change this and the previous cases around */
		xdebug_str_add_literal(&name, "self::");
	}
	if (cur_opcode->opcode >= ZEND_PRE_INC_OBJ && cur_opcode->opcode <= ZEND_POST_DEC_OBJ) {
		zval_value = xdebug_get_zval_value_line(xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var), 0, options);
		xdebug_str_add_literal(&name, "$this->");
		xdebug_str_add_str(&name, zval_value);
		xdebug_str_free(zval_value);
	}

	if (cur_opcode->opcode >= ZEND_PRE_INC_STATIC_PROP && cur_opcode->opcode <= ZEND_POST_DEC_STATIC_PROP) {
		zval_value = xdebug_get_zval_value_line(xdebug_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1, &is_var), 0, options);
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
				xdebug_str_add_literal(&name, "$this");
			}
			if (opcode_ptr->op1_type == IS_CV) {
				xdebug_str_addc(&name, '$');
				xdebug_str_add(&name, zend_get_compiled_variable_name(op_array, opcode_ptr->op1.var)->val, 0);
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
				xdebug_str_add_literal(&name, "->");
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
		(cur_opcode->opcode == ZEND_ASSIGN_OBJ) ||
		(cur_opcode->opcode == ZEND_ASSIGN_OBJ_REF)
	) {
		if (cur_opcode->op1_type == IS_UNUSED) {
			xdebug_str_add_literal(&name, "$this");
		}
		dimval = xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var);
		xdebug_str_add_literal(&name, "->");
		xdebug_str_add(&name, Z_STRVAL_P(dimval), 0);
	}

	if (cur_opcode->opcode == ZEND_ASSIGN_STATIC_PROP_REF) {
		dimval = xdebug_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1, &is_var);
		xdebug_str_add(&name, Z_STRVAL_P(dimval), 0);
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
			xdebug_str_add_literal(&name, "$this->");
		} else {
			xdebug_str_add_literal(&name, "->");
		}
		xdebug_str_add_str(&name, zval_value);
		xdebug_str_free(zval_value);
	}
	if (cur_opcode->opcode == ZEND_ASSIGN_STATIC_PROP_OP) {
		zval_value = xdebug_get_zval_value_line(xdebug_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1, &is_var), 0, options);
		xdebug_str_add_literal(&name, "self::");
		xdebug_str_add_str(&name, zval_value);
		xdebug_str_free(zval_value);
	}

	if (cur_opcode->opcode == ZEND_ASSIGN_DIM) {
		if (next_opcode->opcode == ZEND_OP_DATA && cur_opcode->op2_type == IS_UNUSED) {
			xdebug_str_add_literal(&name, "[]");
		} else {
			zval_value = xdebug_get_zval_value_line(xdebug_get_zval_with_opline(execute_data, opcode_ptr, opcode_ptr->op2_type, &opcode_ptr->op2, &is_var), 0, NULL);
			xdebug_str_addc(&name, '[');
			xdebug_str_add_str(&name, zval_value);
			xdebug_str_addc(&name, ']');
			xdebug_str_free(zval_value);
		}
	}

	if (cur_opcode->opcode == ZEND_ASSIGN_STATIC_PROP) {
		dimval = xdebug_get_zval(execute_data, cur_opcode->op1_type, &cur_opcode->op1, &is_var);
		xdebug_str_add(&name, Z_STRVAL_P(dimval), 0);
	}

	xdfree(options->runtime);
	xdfree(options);

	return name.d;
}

static int xdebug_common_assign_dim_handler(const char *op, XDEBUG_OPCODE_HANDLER_ARGS)
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

	if (XG_TRACE(trace_context) && XINI_TRACE(collect_assignments)) {
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
		} else if (
			(next_opcode->opcode == ZEND_OP_DATA) &&
			(cur_opcode->opcode != ZEND_ASSIGN_OBJ_REF) &&
			(cur_opcode->opcode != ZEND_ASSIGN_STATIC_PROP_REF)
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
		} else if (cur_opcode->opcode == ZEND_ASSIGN_OBJ_REF) {
			if (next_opcode->op1_type == IS_CV) {
				right_full_varname = xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, next_opcode->op1.var)->val);
			} else {
				const zend_op *referenced_opline = xdebug_find_referenced_opline(execute_data, next_opcode, 1);
				right_full_varname = xdebug_find_var_name(execute_data, referenced_opline, NULL);
			}
		} else if (cur_opcode->opcode == ZEND_ASSIGN_STATIC_PROP_REF) {
			if (next_opcode->op1_type == IS_CV) {
				right_full_varname = xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, next_opcode->op1.var)->val);
			} else {
				const zend_op *referenced_opline = xdebug_find_referenced_opline(execute_data, next_opcode, 1);
				right_full_varname = xdebug_find_var_name(execute_data, referenced_opline, NULL);
			}
		} else {
			val = xdebug_get_zval(execute_data, cur_opcode->op2_type, &cur_opcode->op2, &is_var);
		}

		fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));
		if (XG_TRACE(trace_context) && XINI_TRACE(collect_assignments) && XG_TRACE(trace_handler)->assignment) {
			XG_TRACE(trace_handler)->assignment(XG_TRACE(trace_context), fse, full_varname, val, right_full_varname, op, file, lineno);
		}
		xdfree(full_varname);
		xdfree(right_full_varname);
	}

	return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}

XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign, "=");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(qm_assign, "=");
XDEBUG_OPCODE_OVERRIDE_ASSIGN_OP(assign_op);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_OP(assign_dim_op);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_OP(assign_obj_op);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_OP(assign_static_prop_op);
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_inc, "");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_inc, "");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_dec, "");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_dec, "");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_inc_obj, "");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_inc_obj, "");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_dec_obj, "");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_dec_obj, "");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_concat, ".=");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_dim, "=");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_obj, "=");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_ref, "=&");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_obj_ref, "=&");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_static_prop, "=");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_static_prop_ref, "=&");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_inc_static_prop, "");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_dec_static_prop, "");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_inc_static_prop, "");
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_dec_static_prop, "");

void xdebug_init_tracing_globals(xdebug_tracing_globals_t *xg)
{
	xg->trace_handler = NULL;
	xg->trace_context = NULL;
}

void xdebug_tracing_minit(INIT_FUNC_ARGS)
{
	/* Override opcodes for variable assignments in traces */
	xdebug_register_with_opcode_multi_handler(ZEND_ASSIGN, xdebug_assign_handler);
	xdebug_register_with_opcode_multi_handler(ZEND_QM_ASSIGN, xdebug_qm_assign_handler);

	xdebug_set_opcode_handler(ZEND_ASSIGN_OP, xdebug_assign_op_handler);
	xdebug_set_opcode_handler(ZEND_ASSIGN_DIM_OP, xdebug_assign_dim_op_handler);
	xdebug_set_opcode_handler(ZEND_ASSIGN_OBJ_OP, xdebug_assign_obj_op_handler);
	xdebug_set_opcode_handler(ZEND_ASSIGN_STATIC_PROP_OP, xdebug_assign_static_prop_op_handler);
	xdebug_register_with_opcode_multi_handler(ZEND_ASSIGN_DIM, xdebug_assign_dim_handler);
	xdebug_register_with_opcode_multi_handler(ZEND_ASSIGN_OBJ, xdebug_assign_obj_handler);
	xdebug_set_opcode_handler(ZEND_ASSIGN_REF, xdebug_assign_ref_handler);
	xdebug_set_opcode_handler(ZEND_PRE_INC, xdebug_pre_inc_handler);
	xdebug_set_opcode_handler(ZEND_POST_INC, xdebug_post_inc_handler);
	xdebug_set_opcode_handler(ZEND_PRE_DEC, xdebug_pre_dec_handler);
	xdebug_set_opcode_handler(ZEND_POST_DEC, xdebug_post_dec_handler);
	xdebug_set_opcode_handler(ZEND_PRE_INC_OBJ, xdebug_pre_inc_obj_handler);
	xdebug_set_opcode_handler(ZEND_POST_INC_OBJ, xdebug_post_inc_obj_handler);
	xdebug_set_opcode_handler(ZEND_PRE_DEC_OBJ, xdebug_pre_dec_obj_handler);
	xdebug_set_opcode_handler(ZEND_POST_DEC_OBJ, xdebug_post_dec_obj_handler);
	xdebug_set_opcode_handler(ZEND_ASSIGN_OBJ_REF, xdebug_assign_obj_ref_handler);
	xdebug_register_with_opcode_multi_handler(ZEND_ASSIGN_STATIC_PROP, xdebug_assign_static_prop_handler);
	xdebug_set_opcode_handler(ZEND_ASSIGN_STATIC_PROP_REF, xdebug_assign_static_prop_ref_handler);
	xdebug_set_opcode_handler(ZEND_PRE_INC_STATIC_PROP, xdebug_pre_inc_static_prop_handler);
	xdebug_set_opcode_handler(ZEND_PRE_DEC_STATIC_PROP, xdebug_pre_dec_static_prop_handler);
	xdebug_set_opcode_handler(ZEND_POST_INC_STATIC_PROP, xdebug_post_inc_static_prop_handler);
	xdebug_set_opcode_handler(ZEND_POST_DEC_STATIC_PROP, xdebug_post_dec_static_prop_handler);
}

void xdebug_tracing_register_constants(INIT_FUNC_ARGS)
{
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_APPEND", XDEBUG_TRACE_OPTION_APPEND, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_COMPUTERIZED", XDEBUG_TRACE_OPTION_COMPUTERIZED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_FLAMEGRAPH_COST", XDEBUG_TRACE_OPTION_FLAMEGRAPH_COST, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_FLAMEGRAPH_MEM", XDEBUG_TRACE_OPTION_FLAMEGRAPH_MEM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_HTML", XDEBUG_TRACE_OPTION_HTML, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_NAKED_FILENAME", XDEBUG_TRACE_OPTION_NAKED_FILENAME, CONST_CS | CONST_PERSISTENT);
}

void xdebug_tracing_rinit(void)
{
	XG_TRACE(trace_handler) = NULL;
	XG_TRACE(trace_context) = NULL;

	xdebug_disable_opcache_optimizer();
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
	if (xdebug_lib_start_with_request(XDEBUG_MODE_TRACING) || xdebug_lib_start_with_trigger(XDEBUG_MODE_TRACING, NULL)) {
		/* In case we do an auto-trace we are not interested in the return
		 * value, but we still have to free it. */
		xdfree(xdebug_start_trace(NULL, op_array->filename, XINI_TRACE(trace_options)));
	}
}

void xdebug_tracing_execute_ex(function_stack_entry *fse)
{
	if (fse->filtered_tracing || !XG_TRACE(trace_context)) {
		return;
	}

	if (XG_TRACE(trace_handler)->function_entry) {
		XG_TRACE(trace_handler)->function_entry(XG_TRACE(trace_context), fse);
	}
}

void xdebug_tracing_execute_ex_end(function_stack_entry *fse, zend_execute_data *execute_data)
{
	zend_op_array *op_array;

	if (fse->filtered_tracing || !XG_TRACE(trace_context)) {
		return;
	}

	if ((XG_TRACE(trace_handler)->function_exit)) {
		XG_TRACE(trace_handler)->function_exit(XG_TRACE(trace_context), fse);
	}

	/* Store return value in the trace file */
	if (!XINI_TRACE(collect_return)) {
		return;
	}

	op_array = &(execute_data->func->op_array);

	if (!execute_data || !execute_data->return_value) {
		return;
	}

	if (op_array->fn_flags & ZEND_ACC_GENERATOR) {
		if (XG_TRACE(trace_handler)->generator_return_value) {
			XG_TRACE(trace_handler)->generator_return_value(XG_TRACE(trace_context), fse, (zend_generator*) execute_data->return_value);
		}
	} else {
		if (XG_TRACE(trace_handler)->return_value) {
			XG_TRACE(trace_handler)->return_value(XG_TRACE(trace_context), fse, execute_data->return_value);
		}
	}
}

int xdebug_tracing_execute_internal(function_stack_entry *fse)
{
	if (fse->filtered_tracing || !XG_TRACE(trace_context)) {
		return 0;
	}

	if (fse->function.type != XFUNC_ZEND_PASS && (XG_TRACE(trace_handler)->function_entry)) {
		XG_TRACE(trace_handler)->function_entry(XG_TRACE(trace_context), fse);
		return 1;
	}

	return 0;
}

void xdebug_tracing_execute_internal_end(function_stack_entry *fse, zval *return_value)
{
	if (fse->filtered_tracing || !XG_TRACE(trace_context)) {
		return;
	}

	if (fse->function.type != XFUNC_ZEND_PASS && (XG_TRACE(trace_handler)->function_exit)) {
		XG_TRACE(trace_handler)->function_exit(XG_TRACE(trace_context), fse);
	}

	/* Store return value in the trace file */
	if (XINI_TRACE(collect_return) && fse->function.type != XFUNC_ZEND_PASS && return_value && XG_TRACE(trace_handler)->return_value) {
		XG_TRACE(trace_handler)->return_value(XG_TRACE(trace_context), fse, return_value);
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
