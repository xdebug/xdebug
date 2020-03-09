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
#include "TSRM.h"
#include "php_globals.h"
#include "zend_closures.h"
#include "zend_exceptions.h"

#include "php_xdebug.h"

#include "base.h"
#include "filter.h"
#include "monitor.h"
#include "stack.h"
#include "gcstats/gc_stats.h"
#include "profiler/profiler.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

void (*xdebug_old_error_cb)(int type, const char *error_filename, const XDEBUG_ERROR_LINENO_TYPE error_lineno, const char *format, va_list args) ZEND_ATTRIBUTE_PTR_FORMAT(printf, 4, 0);
void (*xdebug_new_error_cb)(int type, const char *error_filename, const XDEBUG_ERROR_LINENO_TYPE error_lineno, const char *format, va_list args);
void xdebug_error_cb(int type, const char *error_filename, const XDEBUG_ERROR_LINENO_TYPE error_lineno, const char *format, va_list args);

/* execution redirection functions */
zend_op_array* (*old_compile_file)(zend_file_handle* file_handle, int type);
static void (*xdebug_old_execute_ex)(zend_execute_data *execute_data);
static void (*xdebug_old_execute_internal)(zend_execute_data *current_execute_data, zval *return_value);

static int xdebug_silence_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	zend_op_array *op_array = &execute_data->func->op_array;
	const zend_op *cur_opcode = execute_data->opline;

	xdebug_coverage_record_silence_if_active(execute_data, op_array);

	if (XINI_BASE(do_scream)) {
		execute_data->opline++;
		if (cur_opcode->opcode == ZEND_BEGIN_SILENCE) {
			XG_BASE(in_at) = 1;
		} else {
			XG_BASE(in_at) = 0;
		}
		return ZEND_USER_OPCODE_CONTINUE;
	}

	return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}

static void xdebug_throw_exception_hook(zval *exception)
{
	zval *code, *message, *file, *line;
	zval *xdebug_message_trace, *previous_exception;
	zend_class_entry *default_ce, *exception_ce;
	char *code_str = NULL;
	char *exception_trace;
	xdebug_str tmp_str = XDEBUG_STR_INITIALIZER;
	zval dummy;

	if (!exception) {
		return;
	}

	default_ce = Z_OBJCE_P(exception);
	exception_ce = Z_OBJCE_P(exception);

	code =    zend_read_property(default_ce, exception, "code",    sizeof("code")-1,    0, &dummy);
	message = zend_read_property(default_ce, exception, "message", sizeof("message")-1, 0, &dummy);
	file =    zend_read_property(default_ce, exception, "file",    sizeof("file")-1,    0, &dummy);
	line =    zend_read_property(default_ce, exception, "line",    sizeof("line")-1,    0, &dummy);

	if (Z_TYPE_P(code) == IS_LONG) {
		if (Z_LVAL_P(code) != 0) {
			code_str = xdebug_sprintf("%lu", Z_LVAL_P(code));
		}
	} else if (Z_TYPE_P(code) != IS_STRING) {
		code_str = xdstrdup("");
	}

	convert_to_string_ex(message);
	convert_to_string_ex(file);
	convert_to_long_ex(line);

	previous_exception = zend_read_property(default_ce, exception, "previous", sizeof("previous")-1, 1, &dummy);
	if (previous_exception && Z_TYPE_P(previous_exception) == IS_OBJECT) {
		xdebug_message_trace = zend_read_property(default_ce, previous_exception, "xdebug_message", sizeof("xdebug_message")-1, 1, &dummy);
		if (xdebug_message_trace && Z_TYPE_P(xdebug_message_trace) != IS_NULL) {
			xdebug_str_add(&tmp_str, Z_STRVAL_P(xdebug_message_trace), 0);
		}
	}

	if (!PG(html_errors)) {
		xdebug_str_addl(&tmp_str, "\n", 1, 0);
	}
	xdebug_append_error_description(&tmp_str, PG(html_errors), STR_NAME_VAL(exception_ce->name), Z_STRVAL_P(message), Z_STRVAL_P(file), Z_LVAL_P(line));
	xdebug_append_printable_stack(&tmp_str, PG(html_errors));
	exception_trace = tmp_str.d;
	zend_update_property_string(default_ce, exception, "xdebug_message", sizeof("xdebug_message")-1, exception_trace);

	if (XG_BASE(last_exception_trace)) {
		xdfree(XG_BASE(last_exception_trace));
	}
	XG_BASE(last_exception_trace) = exception_trace;

	if (XINI_BASE(show_ex_trace) || (instanceof_function(exception_ce, zend_ce_error) && XINI_BASE(show_error_trace))) {
		if (PG(log_errors)) {
			xdebug_log_stack(STR_NAME_VAL(exception_ce->name), Z_STRVAL_P(message), Z_STRVAL_P(file), Z_LVAL_P(line));
		}
		if (PG(display_errors)) {
			xdebug_str displ_tmp_str = XDEBUG_STR_INITIALIZER;
			xdebug_append_error_head(&displ_tmp_str, PG(html_errors), "exception");
			xdebug_str_add(&displ_tmp_str, exception_trace, 0);
			xdebug_append_error_footer(&displ_tmp_str, PG(html_errors));

			php_printf("%s", displ_tmp_str.d);
			xdebug_str_dtor(displ_tmp_str);
		}
	}

	xdebug_debugger_throw_exception_hook(exception_ce, file, line, code, code_str, message);

	/* Free code_str if necessary */
	if (code_str) {
		xdfree(code_str);
	}
}

/* {{{ zend_op_array xdebug_compile_file (file_handle, type)
 *    This function provides a hook for the execution of bananas */
static zend_op_array *xdebug_compile_file(zend_file_handle *file_handle, int type)
{
	zend_op_array *op_array;

	op_array = old_compile_file(file_handle, type);

	if (op_array) {
		xdebug_coverage_compile_file(op_array);
		xdebug_debugger_compile_file(op_array);
	}
	return op_array;
}
/* }}} */

static void xdebug_declared_var_dtor(void *dummy, void *elem)
{
	xdebug_str *s = (xdebug_str*) elem;

	xdebug_str_free(s);
}

static void function_stack_entry_dtor(void *dummy, void *elem)
{
	unsigned int          i;
	function_stack_entry *e = elem;

	e->refcount--;

	if (e->refcount == 0) {
		xdebug_func_dtor_by_ref(&e->function);

		if (e->filename) {
			xdfree(e->filename);
		}

		if (e->var) {
			for (i = 0; i < e->varc; i++) {
				if (e->var[i].name) {
					xdfree(e->var[i].name);
				}
				zval_ptr_dtor(&(e->var[i].data));
			}
			xdfree(e->var);
		}

		if (e->include_filename) {
			xdfree(e->include_filename);
		}

		if (e->declared_vars) {
			xdebug_llist_destroy(e->declared_vars, NULL);
			e->declared_vars = NULL;
		}

		if (e->profile.call_list) {
			xdebug_llist_destroy(e->profile.call_list, NULL);
			e->profile.call_list = NULL;
		}

		xdfree(e);
	}
}

static void xdebug_llist_string_dtor(void *dummy, void *elem)
{
	char *s = elem;

	if (s) {
		xdfree(s);
	}
}

static void add_used_variables(function_stack_entry *fse, zend_op_array *op_array)
{
	unsigned int i = 0;

	if (!fse->declared_vars) {
		fse->declared_vars = xdebug_llist_alloc(xdebug_declared_var_dtor);
	}

	/* Check parameters */
	for (i = 0; i < fse->varc; i++) {
		if (fse->var[i].name) {
			xdebug_llist_insert_next(fse->declared_vars, XDEBUG_LLIST_TAIL(fse->declared_vars), xdebug_str_create(fse->var[i].name, fse->var[i].length));
		}
	}

	/* gather used variables from compiled vars information */
	while (i < (unsigned int) op_array->last_var) {
		xdebug_llist_insert_next(fse->declared_vars, XDEBUG_LLIST_TAIL(fse->declared_vars), xdebug_str_create(STR_NAME_VAL(op_array->vars[i]), STR_NAME_LEN(op_array->vars[i])));
		i++;
	}

	/* opcode scanning time */
	while (i < op_array->last) {
		char *cv = NULL;
		int cv_len;

		if (op_array->opcodes[i].op1_type == IS_CV) {
			cv = (char *) xdebug_get_compiled_variable_name(op_array, op_array->opcodes[i].op1.var, &cv_len);
			xdebug_llist_insert_next(fse->declared_vars, XDEBUG_LLIST_TAIL(fse->declared_vars), xdebug_str_create(cv, cv_len));
		}
		if (op_array->opcodes[i].op2_type == IS_CV) {
			cv = (char *) xdebug_get_compiled_variable_name(op_array, op_array->opcodes[i].op2.var, &cv_len);
			xdebug_llist_insert_next(fse->declared_vars, XDEBUG_LLIST_TAIL(fse->declared_vars), xdebug_str_create(cv, cv_len));
		}
		i++;
	}
}


/* Opcode handler for exit, to be able to clean up the profiler */
int xdebug_exit_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	const zend_op *cur_opcode = execute_data->opline;

	xdebug_profiler_exit_handler();

	return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}


static void xdebug_execute_ex(zend_execute_data *execute_data)
{
	zend_op_array        *op_array = &(execute_data->func->op_array);
	zend_execute_data    *edata = execute_data->prev_execute_data;
	function_stack_entry *fse, *xfse;
	int                   function_nr = 0;
	xdebug_llist_element *le;
	char                 *code_coverage_function_name = NULL;
	char                 *code_coverage_file_name = NULL;
	int                   code_coverage_init = 0;

	/* For PHP 7, we need to reset the opline to the start, so that all opcode
	 * handlers are being hit. But not for generators, as that would make an
	 * endless loop. TODO: Fix RECV handling with generators. */
	if (!(EX(func)->op_array.fn_flags & ZEND_ACC_GENERATOR)) {
		EX(opline) = EX(func)->op_array.opcodes;
	}

	if (xdebug_debugger_bailout_if_no_exec_requested()) {
		return;
	}

	/* If we're evaluating for the debugger's eval capability, just bail out */
	if (op_array && op_array->filename && strcmp("xdebug://debug-eval", STR_NAME_VAL(op_array->filename)) == 0) {
		xdebug_old_execute_ex(execute_data);
		return;
	}

	/* if we're in a ZEND_EXT_STMT, we ignore this function call as it's likely
	   that it's just being called to check for breakpoints with conditions */
	if (edata && edata->func && ZEND_USER_CODE(edata->func->type) && edata->opline && edata->opline->opcode == ZEND_EXT_STMT) {
		xdebug_old_execute_ex(execute_data);
		return;
	}

	xdebug_debugger_set_program_name(op_array->filename);

	if (XG_BASE(in_execution)) {
		/* Start debugger if this is the first main script, or previously a
		 * connection was established and this process no longer has the same
		 * PID */
		if (
			XG_BASE(level) == 0 ||
			(xdebug_is_debug_connection_active() && !xdebug_is_debug_connection_active_for_current_pid())
		) {
			/* Start remote context if requested */
			xdebug_do_req();
		}

		if (XG_BASE(level) == 0) {
			xdebug_gcstats_init_if_requested(op_array);
			xdebug_profiler_init_if_requested(op_array);
			xdebug_tracing_init_if_requested(op_array);
		}
	}

	XG_BASE(level)++;
	if ((signed long) XG_BASE(level) > XINI_BASE(max_nesting_level) && (XINI_BASE(max_nesting_level) != -1)) {
		zend_throw_exception_ex(zend_ce_error, 0, "Maximum function nesting level of '" ZEND_LONG_FMT "' reached, aborting!", XINI_BASE(max_nesting_level));
	}

	fse = xdebug_add_stack_frame(edata, op_array, XDEBUG_USER_DEFINED);
	fse->function.internal = 0;

	/* A hack to make __call work with profiles. The function *is* user defined after all. */
	if (fse && fse->prev && fse->function.function && (strcmp(fse->function.function, "__call") == 0)) {
		fse->prev->user_defined = XDEBUG_USER_DEFINED;
	}

	function_nr = XG_BASE(function_count);

	xdebug_tracing_execute_ex(function_nr, fse);

	fse->execute_data = EG(current_execute_data)->prev_execute_data;
	if (ZEND_CALL_INFO(EG(current_execute_data)) & ZEND_CALL_HAS_SYMBOL_TABLE) {
		fse->symbol_table = EG(current_execute_data)->symbol_table;
	}
	if (Z_OBJ(EG(current_execute_data)->This)) {
		fse->This = &EG(current_execute_data)->This;
	} else {
		fse->This = NULL;
	}

	if (XG_BASE(stack) && (XINI_BASE(collect_vars) || XINI_BASE(show_local_vars) || xdebug_is_debug_connection_active_for_current_pid())) {
		/* Because include/require is treated as a stack level, we have to add used
		 * variables in include/required files to all the stack levels above, until
		 * we hit a function or the top level stack.  This is so that the variables
		 * show up correctly where they should be.  We always call
		 * add_used_variables on the current stack level, otherwise vars in include
		 * files do not show up in the locals list.  */
		for (le = XDEBUG_LLIST_TAIL(XG_BASE(stack)); le != NULL; le = XDEBUG_LLIST_PREV(le)) {
			xfse = XDEBUG_LLIST_VALP(le);
			add_used_variables(xfse, op_array);
			if (XDEBUG_IS_NORMAL_FUNCTION(&xfse->function)) {
				break;
			}
		}
	}

	code_coverage_init = xdebug_coverage_execute_ex(fse, op_array, &code_coverage_file_name, &code_coverage_function_name);

	/* If we're in an eval, we need to create an ID for it. This ID however
	 * depends on the debugger mechanism in use so we need to call a function
	 * in the handler for it */
	if (fse->function.type == XFUNC_EVAL) {
		xdebug_debugger_register_eval(fse);
	}

	/* Check for entry breakpoints */
	xdebug_debugger_handle_breakpoints(fse, XDEBUG_BREAKPOINT_TYPE_CALL);

	xdebug_profiler_execute_ex(fse, op_array);

	xdebug_old_execute_ex(execute_data);

	xdebug_profiler_execute_ex_end(fse);

	if (code_coverage_init) {
		xdebug_coverage_execute_ex_end(fse, op_array, code_coverage_file_name, code_coverage_function_name);
	}


	xdebug_tracing_execute_ex_end(function_nr, fse, execute_data);

	/* Check for return breakpoints */
	xdebug_debugger_handle_breakpoints(fse, XDEBUG_BREAKPOINT_TYPE_RETURN);

	fse->symbol_table = NULL;
	fse->execute_data = NULL;

	if (XG_BASE(stack)) {
		xdebug_llist_remove(XG_BASE(stack), XDEBUG_LLIST_TAIL(XG_BASE(stack)), function_stack_entry_dtor);
	}
	XG_BASE(level)--;
}

static int check_soap_call(function_stack_entry *fse, zend_execute_data *execute_data)
{
	if (
		fse->function.class &&
		Z_OBJ(EX(This)) &&
		Z_TYPE(EX(This)) == IS_OBJECT &&
		(zend_hash_str_find_ptr(&module_registry, "soap", sizeof("soap") - 1) != NULL)
	) {
		zend_class_entry *soap_server_ce, *soap_client_ce;

		soap_server_ce = zend_hash_str_find_ptr(CG(class_table), "soapserver", 10);
		soap_client_ce = zend_hash_str_find_ptr(CG(class_table), "soapclient", 10);

		if (!soap_server_ce || !soap_client_ce) {
			return 0;
		}

		if (
			(instanceof_function(Z_OBJCE(EX(This)), soap_server_ce)) ||
			(instanceof_function(Z_OBJCE(EX(This)), soap_client_ce))
		) {
			return 1;
		}
	}
	return 0;
}

static void xdebug_execute_internal(zend_execute_data *current_execute_data, zval *return_value)
{
	zend_execute_data    *edata = EG(current_execute_data);
	function_stack_entry *fse;
	int                   function_nr = 0;
	int                   function_call_traced = 0;
	int                   restore_error_handler_situation = 0;
	void                (*tmp_error_cb)(int type, const char *error_filename, const XDEBUG_ERROR_LINENO_TYPE error_lineno, const char *format, va_list args) ZEND_ATTRIBUTE_PTR_FORMAT(printf, 4, 0) = NULL;

	XG_BASE(level)++;
	if ((signed long) XG_BASE(level) > XINI_BASE(max_nesting_level) && (XINI_BASE(max_nesting_level) != -1)) {
		zend_throw_exception_ex(zend_ce_error, 0, "Maximum function nesting level of '" ZEND_LONG_FMT "' reached, aborting!", XINI_BASE(max_nesting_level));
	}

	fse = xdebug_add_stack_frame(edata, &edata->func->op_array, XDEBUG_BUILT_IN);
	fse->function.internal = 1;

	function_nr = XG_BASE(function_count);

	function_call_traced = xdebug_tracing_execute_internal(function_nr, fse);

	/* Check for entry breakpoints */
	xdebug_debugger_handle_breakpoints(fse, XDEBUG_BREAKPOINT_TYPE_CALL);

	/* Check for SOAP */
	if (check_soap_call(fse, current_execute_data)) {
		restore_error_handler_situation = 1;
		tmp_error_cb = zend_error_cb;
		zend_error_cb = xdebug_old_error_cb;
	}

	xdebug_profiler_execute_internal(fse);

	if (xdebug_old_execute_internal) {
		xdebug_old_execute_internal(current_execute_data, return_value);
	} else {
		execute_internal(current_execute_data, return_value);
	}

	xdebug_profiler_execute_internal_end(fse);

	/* Restore SOAP situation if needed */
	if (restore_error_handler_situation) {
		zend_error_cb = tmp_error_cb;
	}

	/* We only call the function_exit handler and return value handler if the
	 * function call was also traced. Otherwise we end up with return trace
	 * lines without a corresponding function call line. */
	if (function_call_traced) {
		xdebug_tracing_execute_internal_end(function_nr, fse, return_value);
	}

	/* Check for return breakpoints */
	xdebug_debugger_handle_breakpoints(fse, XDEBUG_BREAKPOINT_TYPE_RETURN);

	if (XG_BASE(stack)) {
		xdebug_llist_remove(XG_BASE(stack), XDEBUG_LLIST_TAIL(XG_BASE(stack)), function_stack_entry_dtor);
	}
	XG_BASE(level)--;
}

static void xdebug_overloaded_functions_setup(void)
{
	zend_function *orig;

	/* Override var_dump with our own function */
	orig = zend_hash_str_find_ptr(EG(function_table), "var_dump", sizeof("var_dump") - 1);
	XG_BASE(orig_var_dump_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_xdebug_var_dump;

	/* Override set_time_limit with our own function to prevent timing out while debugging */
	orig = zend_hash_str_find_ptr(EG(function_table), "set_time_limit", sizeof("set_time_limit") - 1);
	XG_BASE(orig_set_time_limit_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_xdebug_set_time_limit;

	/* Override error_reporting with our own function, to be able to give right answer during DBGp's
	 * 'eval' commands */
	orig = zend_hash_str_find_ptr(EG(function_table), "error_reporting", sizeof("error_reporting") - 1);
	XG_BASE(orig_error_reporting_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_xdebug_error_reporting;

	/* Override pcntl_exec with our own function to be able to write profiling summary */
	orig = zend_hash_str_find_ptr(EG(function_table), "pcntl_exec", sizeof("pcntl_exec") - 1);
	if (orig) {
		XG_BASE(orig_pcntl_exec_func) = orig->internal_function.handler;
		orig->internal_function.handler = zif_xdebug_pcntl_exec;
	} else {
		XG_BASE(orig_pcntl_exec_func) = NULL;
	}
}

static void xdebug_overloaded_functions_restore(void)
{
	zend_function *orig;

	orig = zend_hash_str_find_ptr(EG(function_table), "var_dump", sizeof("var_dump") - 1);
	orig->internal_function.handler = XG_BASE(orig_var_dump_func);

	orig = zend_hash_str_find_ptr(EG(function_table), "set_time_limit", sizeof("set_time_limit") - 1);
	orig->internal_function.handler = XG_BASE(orig_set_time_limit_func);

	orig = zend_hash_str_find_ptr(EG(function_table), "error_reporting", sizeof("error_reporting") - 1);
	orig->internal_function.handler = XG_BASE(orig_error_reporting_func);

	if (XG_BASE(orig_pcntl_exec_func)) {
		orig = zend_hash_str_find_ptr(EG(function_table), "pcntl_exec", sizeof("pcntl_exec") - 1);
		if (orig) {
			orig->internal_function.handler = XG_BASE(orig_pcntl_exec_func);
		}
	}
}

static int xdebug_closure_serialize_deny_wrapper(zval *object, unsigned char **buffer, size_t *buf_len, zend_serialize_data *data)
{
	zend_class_entry *ce = Z_OBJCE_P(object);

	if (!XG_BASE(in_var_serialisation)) {
		zend_throw_exception_ex(NULL, 0, "Serialization of '%s' is not allowed", STR_NAME_VAL(ce->name));
	}
	return FAILURE;
}


void xdebug_base_minit(INIT_FUNC_ARGS)
{
	/* Replace error handler callback with our own */
	xdebug_old_error_cb = zend_error_cb;
	xdebug_new_error_cb = xdebug_error_cb;

	/* Redirect compile and execute functions to our own. For PHP 7.3 and
	 * later, we hook these in xdebug_post_startup instead */
#if PHP_VERSION_ID < 70300
	old_compile_file = zend_compile_file;
	zend_compile_file = xdebug_compile_file;
#endif

	xdebug_old_execute_ex = zend_execute_ex;
	zend_execute_ex = xdebug_execute_ex;

	xdebug_old_execute_internal = zend_execute_internal;
	zend_execute_internal = xdebug_execute_internal;


	xdebug_set_opcode_handler(ZEND_BEGIN_SILENCE, xdebug_silence_handler);
	xdebug_set_opcode_handler(ZEND_END_SILENCE, xdebug_silence_handler);

	REGISTER_LONG_CONSTANT("XDEBUG_STACK_NO_DESC", XDEBUG_STACK_NO_DESC, CONST_CS | CONST_PERSISTENT);

	XG_BASE(error_reporting_override) = 0;
	XG_BASE(error_reporting_overridden) = 0;
	XG_BASE(output_is_tty) = OUTPUT_NOT_CHECKED;
}

void xdebug_base_mshutdown()
{
	/* Reset compile, execute and error callbacks */
	zend_compile_file = old_compile_file;
	zend_execute_ex = xdebug_old_execute_ex;
	zend_execute_internal = xdebug_old_execute_internal;
	zend_error_cb = xdebug_old_error_cb;
}

void xdebug_base_post_startup()
{
	old_compile_file = zend_compile_file;
	zend_compile_file = xdebug_compile_file;
}

void xdebug_base_rinit()
{
	/* Hack: We check for a soap header here, if that's existing, we don't use
	 * Xdebug's error handler to keep soap fault from fucking up. */
	if (XINI_BASE(default_enable) && zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_SOAPACTION", sizeof("HTTP_SOAPACTION") - 1) == NULL) {
		zend_error_cb = xdebug_new_error_cb;
		zend_throw_exception_hook = xdebug_throw_exception_hook;
	}

	XG_BASE(stack) = xdebug_llist_alloc(function_stack_entry_dtor);
	XG_BASE(level)         = 0;
	XG_BASE(in_debug_info) = 0;
	XG_BASE(prev_memory)   = 0;
	XG_BASE(function_count) = -1;
	XG_BASE(last_exception_trace) = NULL;
	XG_BASE(last_eval_statement) = NULL;
	XG_BASE(do_collect_errors) = 0;
	XG_BASE(collected_errors)  = xdebug_llist_alloc(xdebug_llist_string_dtor);
	XG_BASE(do_monitor_functions) = 0;
	XG_BASE(functions_to_monitor) = NULL;
	XG_BASE(monitored_functions_found) = xdebug_llist_alloc(xdebug_monitored_function_dtor);
	XG_BASE(headers) = xdebug_llist_alloc(xdebug_llist_string_dtor);


	/* Initialize dump superglobals */
	XG_BASE(dumped) = 0;

	/* Initialize start time */
	XG_BASE(start_time) = xdebug_get_utime();

	XG_BASE(in_var_serialisation) = 0;
	zend_ce_closure->serialize = xdebug_closure_serialize_deny_wrapper;

	/* Signal that we're in a request now */
	XG_BASE(in_execution) = 1;

	/* filters */
	XG_BASE(filter_type_tracing)       = XDEBUG_FILTER_NONE;
	XG_BASE(filter_type_profiler)      = XDEBUG_FILTER_NONE;
	XG_BASE(filter_type_code_coverage) = XDEBUG_FILTER_NONE;
	XG_BASE(filters_tracing)           = xdebug_llist_alloc(xdebug_llist_string_dtor);
	XG_BASE(filters_code_coverage)     = xdebug_llist_alloc(xdebug_llist_string_dtor);

	/* Overload var_dump, set_time_limit, error_reporting, and pcntl_exec */
	xdebug_overloaded_functions_setup();
}

void xdebug_base_post_deactivate()
{
	xdebug_llist_destroy(XG_BASE(stack), NULL);
	XG_BASE(stack) = NULL;

	XG_BASE(level)            = 0;
	XG_BASE(in_debug_info)    = 0;

	if (XG_BASE(last_exception_trace)) {
		xdfree(XG_BASE(last_exception_trace));
		XG_BASE(last_exception_trace) = NULL;
	}

	if (XG_BASE(last_eval_statement)) {
		efree(XG_BASE(last_eval_statement));
		XG_BASE(last_eval_statement) = NULL;
	}

	xdebug_llist_destroy(XG_BASE(collected_errors), NULL);
	XG_BASE(collected_errors) = NULL;

	xdebug_llist_destroy(XG_BASE(monitored_functions_found), NULL);
	XG_BASE(monitored_functions_found) = NULL;

	if (XG_BASE(functions_to_monitor)) {
		xdebug_hash_destroy(XG_BASE(functions_to_monitor));
		XG_BASE(functions_to_monitor) = NULL;
	}

	/* Clean up collected headers */
	xdebug_llist_destroy(XG_BASE(headers), NULL);
	XG_BASE(headers) = NULL;

	/* filters */
	xdebug_llist_destroy(XG_BASE(filters_tracing), NULL);
	xdebug_llist_destroy(XG_BASE(filters_code_coverage), NULL);
	XG_BASE(filters_tracing) = NULL;
	XG_BASE(filters_code_coverage) = NULL;

	/* Restore original var_dump, set_time_limit, error_reporting, and pcntl_exec handlers */
	xdebug_overloaded_functions_restore();
}

void xdebug_base_rshutdown()
{
	/* Signal that we're no longer in a request */
	XG_BASE(in_execution) = 0;
}

PHP_FUNCTION(xdebug_enable)
{
	zend_error_cb = xdebug_new_error_cb;
	zend_throw_exception_hook = xdebug_throw_exception_hook;
}

PHP_FUNCTION(xdebug_disable)
{
	zend_error_cb = xdebug_old_error_cb;
	zend_throw_exception_hook = NULL;
}

PHP_FUNCTION(xdebug_is_enabled)
{
	RETURN_BOOL(zend_error_cb == xdebug_new_error_cb);
}

PHP_FUNCTION(xdebug_get_collected_errors)
{
	xdebug_llist_element *le;
	char                 *string;
	zend_bool             clear = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &clear) == FAILURE) {
		return;
	}

	array_init(return_value);
	for (le = XDEBUG_LLIST_HEAD(XG_BASE(collected_errors)); le != NULL; le = XDEBUG_LLIST_NEXT(le))	{
		string = XDEBUG_LLIST_VALP(le);
		add_next_index_string(return_value, string);
	}

	if (clear) {
		xdebug_llist_destroy(XG_BASE(collected_errors), NULL);
		XG_BASE(collected_errors) = xdebug_llist_alloc(xdebug_llist_string_dtor);
	}
}
