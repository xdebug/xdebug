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
#include "php_xdebug_arginfo.h"

#include "base.h"
#include "filter.h"
#include "develop/develop.h"
#include "gcstats/gc_stats.h"
#include "lib/lib_private.h"
#include "lib/var.h"
#include "profiler/profiler.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

#if PHP_VERSION_ID >= 80000
void (*xdebug_old_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, zend_string *message);
void (*xdebug_new_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, zend_string *message);
void xdebug_error_cb(int orig_type, const char *error_filename, const uint32_t error_lineno, zend_string *message);
#else
void (*xdebug_old_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, const char *format, va_list args) ZEND_ATTRIBUTE_PTR_FORMAT(printf, 4, 0);
void (*xdebug_new_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, const char *format, va_list args);
void xdebug_error_cb(int orig_type, const char *error_filename, const uint32_t error_lineno, const char *format, va_list args);
#endif

/* execution redirection functions */
zend_op_array* (*old_compile_file)(zend_file_handle* file_handle, int type);
static void (*xdebug_old_execute_ex)(zend_execute_data *execute_data);
static void (*xdebug_old_execute_internal)(zend_execute_data *current_execute_data, zval *return_value);

/* error_cb and execption hook overrides */
void xdebug_base_use_original_error_cb(void);
void xdebug_base_use_xdebug_error_cb(void);
void xdebug_base_use_xdebug_throw_exception_hook(void);

/* Forward declarations for function overides */
PHP_FUNCTION(xdebug_set_time_limit);
PHP_FUNCTION(xdebug_error_reporting);
PHP_FUNCTION(xdebug_pcntl_exec);
PHP_FUNCTION(xdebug_pcntl_fork);


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

/* I don't like this API, but the function_stack_entry does not keep this as a
 * pointer, and hence we need two APIs for freeing :-S */
void xdebug_func_dtor_by_ref(xdebug_func *elem)
{
	if (elem->function) {
		xdfree(elem->function);
	}
	if (elem->class) {
		xdfree(elem->class);
	}
}

void xdebug_func_dtor(xdebug_func *elem)
{
	xdebug_func_dtor_by_ref(elem);
	xdfree(elem);
}

static void function_stack_entry_dtor(void *dummy, void *elem)
{
	unsigned int          i;
	function_stack_entry *e = elem;

	e->refcount--;

	if (e->refcount == 0) {
		xdebug_func_dtor_by_ref(&e->function);

		if (e->filename) {
			zend_string_release(e->filename);
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
			zend_string_release(e->include_filename);
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


int xdebug_include_or_eval_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	const zend_op *opline = execute_data->opline;
	zval *inc_filename;
	zval tmp_inc_filename;
	int  is_var;

	if (opline->extended_value != ZEND_EVAL) {
		return xdebug_call_original_opcode_handler_if_set(opline->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
	}

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
		zend_string_release(XG_BASE(last_eval_statement));
	}
	XG_BASE(last_eval_statement) = zend_string_init(Z_STRVAL_P(inc_filename), Z_STRLEN_P(inc_filename), 0);

	if (inc_filename == &tmp_inc_filename) {
		zval_dtor(&tmp_inc_filename);
	}

	return xdebug_call_original_opcode_handler_if_set(opline->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}

static int find_line_number_for_current_execute_point(zend_execute_data *edata)
{
	zend_execute_data *ptr = edata;

	while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
		ptr = ptr->prev_execute_data;
	}

	if (ptr && ptr->opline) {
		return ptr->opline->lineno;
	}

	return 0;
}


void xdebug_build_fname(xdebug_func *tmp, zend_execute_data *edata)
{
	memset(tmp, 0, sizeof(xdebug_func));

	if (edata && edata->func && edata->func == (zend_function*) &zend_pass_function) {
		tmp->type     = XFUNC_ZEND_PASS;
		tmp->function = xdstrdup("{zend_pass}");
	} else if (edata && edata->func) {
		tmp->type = XFUNC_NORMAL;
		if ((Z_TYPE(edata->This)) == IS_OBJECT) {
			tmp->type = XFUNC_MEMBER;
			if (edata->func->common.scope && strcmp(edata->func->common.scope->name->val, "class@anonymous") == 0) {
				tmp->class = xdebug_sprintf(
					"{anonymous-class:%s:%d-%d}",
					edata->func->common.scope->info.user.filename->val,
					edata->func->common.scope->info.user.line_start,
					edata->func->common.scope->info.user.line_end
				);
			} else {
				tmp->class = xdstrdup(edata->This.value.obj->ce->name->val);
			}
		} else {
			if (edata->func->common.scope) {
				tmp->type = XFUNC_STATIC_MEMBER;
				tmp->class = xdstrdup(edata->func->common.scope->name->val);
			}
		}
		if (edata->func->common.function_name) {
			if (edata->func->common.fn_flags & ZEND_ACC_CLOSURE) {
				tmp->function = xdebug_wrap_closure_location_around_function_name(&edata->func->op_array, edata->func->common.function_name->val);
			} else if (strncmp(edata->func->common.function_name->val, "call_user_func", 14) == 0) {
				zend_string *fname = NULL;
				int          lineno = 0;

				if (edata->prev_execute_data && edata->prev_execute_data->func && edata->prev_execute_data->func->type == ZEND_USER_FUNCTION) {
					fname = edata->prev_execute_data->func->op_array.filename;
				}

				if (
					!fname &&
					XDEBUG_LLIST_TAIL(XG_BASE(stack)) &&
					XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack))) &&
					((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack))))->filename
				) {
					fname = ((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack))))->filename;
				}

				if (!fname) {
					/* It wasn't a special call_user_func after all */
					goto normal_after_all;
				}

				lineno = find_line_number_for_current_execute_point(edata);

				tmp->function = xdebug_sprintf(
					"%s:{%s:%d}",
					edata->func->common.function_name->val,
					ZSTR_VAL(fname),
					lineno
				);
			} else {
normal_after_all:
				tmp->function = xdstrdup(edata->func->common.function_name->val);
			}
		} else if (
			edata &&
			edata->func &&
			edata->func->type == ZEND_EVAL_CODE &&
			edata->prev_execute_data &&
			edata->prev_execute_data->func &&
			edata->prev_execute_data->func->common.function_name &&
			(
				(strncmp(edata->prev_execute_data->func->common.function_name->val, "assert", 6) == 0) ||
				(strncmp(edata->prev_execute_data->func->common.function_name->val, "create_function", 15) == 0)
			)
		) {
			tmp->type = XFUNC_NORMAL;
			tmp->function = xdstrdup("{internal eval}");
		} else if (
			edata &&
			edata->prev_execute_data &&
			edata->prev_execute_data->func->type == ZEND_USER_FUNCTION &&
			edata->prev_execute_data->opline &&
			edata->prev_execute_data->opline->opcode == ZEND_INCLUDE_OR_EVAL
		) {
			switch (edata->prev_execute_data->opline->extended_value) {
				case ZEND_EVAL:
					tmp->type = XFUNC_EVAL;
					break;
				case ZEND_INCLUDE:
					tmp->type = XFUNC_INCLUDE;
					break;
				case ZEND_REQUIRE:
					tmp->type = XFUNC_REQUIRE;
					break;
				case ZEND_INCLUDE_ONCE:
					tmp->type = XFUNC_INCLUDE_ONCE;
					break;
				case ZEND_REQUIRE_ONCE:
					tmp->type = XFUNC_REQUIRE_ONCE;
					break;
				default:
					tmp->type = XFUNC_UNKNOWN;
					break;
			}
		} else if (
			edata &&
			edata->prev_execute_data
		) {
			xdebug_build_fname(tmp, edata->prev_execute_data);
		} else {
			tmp->type = XFUNC_UNKNOWN;
		}
	}
}

// TODO: Remove
#define XINI_DEV(v)    (XG(settings.develop.v))


function_stack_entry *xdebug_add_stack_frame(zend_execute_data *zdata, zend_op_array *op_array, int type)
{
	zend_execute_data    *edata;
	zend_op             **opline_ptr = NULL;
	function_stack_entry *tmp;
	zend_op              *cur_opcode;
	int                   i = 0;
	int                   hit_variadic = 0;

	if (type == XDEBUG_USER_DEFINED) {
		edata = EG(current_execute_data)->prev_execute_data;
		if (edata) {
			opline_ptr = (zend_op**) &edata->opline;
		}
	} else {
		edata = EG(current_execute_data);
		opline_ptr = (zend_op**) &EG(current_execute_data)->opline;
	}
	zdata = EG(current_execute_data);

	tmp = xdmalloc (sizeof (function_stack_entry));
	tmp->var           = NULL;
	tmp->varc          = 0;
	tmp->refcount      = 1;
	tmp->level         = XG_BASE(level);
	tmp->arg_done      = 0;
	tmp->declared_vars = NULL;
	tmp->user_defined  = type;
	tmp->filename      = NULL;
	tmp->include_filename  = NULL;
	tmp->profile.call_list = NULL;
	tmp->op_array      = op_array;
	tmp->symbol_table  = NULL;
	tmp->execute_data  = NULL;
	tmp->is_variadic   = 0;
	tmp->filtered_tracing       = 0;
	tmp->filtered_code_coverage = 0;

	XG_BASE(function_count)++;
	tmp->function_nr = XG_BASE(function_count);
	{
		zend_execute_data *ptr = edata;
		while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
			ptr = ptr->prev_execute_data;
		}
		if (ptr) {
			tmp->filename = zend_string_copy(ptr->func->op_array.filename);
		}
	}

	if (!tmp->filename) {
		/* Includes/main script etc */
		tmp->filename  = (type == XDEBUG_USER_DEFINED && op_array && op_array->filename) ? zend_string_copy(op_array->filename) : NULL;
	}
	/* Call user function locations */
	if (
		!tmp->filename &&
		XG_BASE(stack) &&
		XDEBUG_LLIST_TAIL(XG_BASE(stack)) &&
		XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack))) &&
		((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack))))->filename
	) {
		tmp->filename = zend_string_copy(((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack))))->filename);
	}

	if (!tmp->filename) {
		tmp->filename = zend_string_init("UNKNOWN?", sizeof("UNKNOWN?") - 1, 0);
	}
	tmp->prev_memory = XG_BASE(prev_memory);
	tmp->memory = zend_memory_usage(0);
	XG_BASE(prev_memory) = tmp->memory;
	tmp->time   = xdebug_get_utime();
	tmp->lineno = 0;
	tmp->prev   = 0;

	xdebug_build_fname(&(tmp->function), zdata);
	if (!tmp->function.type) {
		tmp->function.function = xdstrdup("{main}");
		tmp->function.class    = NULL;
		tmp->function.type     = XFUNC_MAIN;

	} else if (tmp->function.type & XFUNC_INCLUDES) {
		tmp->lineno = 0;
		if (opline_ptr) {
			cur_opcode = *opline_ptr;
			if (cur_opcode) {
				tmp->lineno = cur_opcode->lineno;
			}
		}

		if (tmp->function.type == XFUNC_EVAL && XG_BASE(last_eval_statement)) {
			tmp->include_filename = zend_string_copy(XG_BASE(last_eval_statement));
		} else {
			tmp->include_filename = zend_string_copy(zend_get_executed_filename_ex());
		}
	} else  {
		tmp->lineno = find_line_number_for_current_execute_point(edata);
		tmp->is_variadic = !!(zdata->func->common.fn_flags & ZEND_ACC_VARIADIC);

		if (XINI_LIB(collect_params) || XINI_DEV(collect_vars) || xdebug_is_debug_connection_active()) {
			int    arguments_sent = 0, arguments_wanted = 0, arguments_storage = 0;

			/* This calculates how many arguments where sent to a function. It
			 * works for both internal and user defined functions.
			 * op_array->num_args works only for user defined functions so
			 * we're not using that here. */
			arguments_sent = ZEND_CALL_NUM_ARGS(zdata);
			arguments_wanted = arguments_sent;

			if (ZEND_USER_CODE(zdata->func->type)) {
				arguments_wanted = op_array->num_args;
			}

			if (ZEND_USER_CODE(zdata->func->type) && zdata->func->common.fn_flags & ZEND_ACC_VARIADIC) {
				arguments_wanted++;
				arguments_sent++;
			}

			if (arguments_wanted > arguments_sent) {
				arguments_storage = arguments_wanted;
			} else {
				arguments_storage = arguments_sent;
			}
			tmp->var = xdmalloc(arguments_storage * sizeof (xdebug_var_name));

			for (i = 0; i < arguments_sent; i++) {
				tmp->var[tmp->varc].name = NULL;
				ZVAL_UNDEF(&tmp->var[tmp->varc].data);
				tmp->var[tmp->varc].length = 0;
				tmp->var[tmp->varc].is_variadic = 0;

				/* Because it is possible that more parameters are sent, then
				 * actually wanted  we can only access the name in case there
				 * is an associated variable to receive the variable here. */
				if (tmp->user_defined == XDEBUG_USER_DEFINED && i < arguments_wanted) {
					if (op_array->arg_info[i].name) {
						tmp->var[tmp->varc].name = xdstrdup(STR_NAME_VAL(op_array->arg_info[i].name));
						tmp->var[tmp->varc].length = STR_NAME_LEN(op_array->arg_info[i].name);
					}
					if (ZEND_ARG_IS_VARIADIC(&op_array->arg_info[i])) {
						tmp->var[tmp->varc].is_variadic = 1;
					}
					if (ZEND_ARG_IS_VARIADIC(&op_array->arg_info[i]) && !hit_variadic) {
						tmp->var[tmp->varc].is_variadic = 1;
						hit_variadic = 1;
					}
				}

				if (XINI_LIB(collect_params)) {
					if ((i < arguments_wanted) || ((zdata->func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE) && (i < arguments_sent))) {
						if (ZEND_CALL_ARG(zdata, tmp->varc+1)) {
							ZVAL_COPY(&(tmp->var[tmp->varc].data), ZEND_CALL_ARG(zdata, tmp->varc+1));
						}
					} else {
						ZVAL_COPY(&(tmp->var[tmp->varc].data), ZEND_CALL_VAR_NUM(zdata, zdata->func->op_array.last_var + zdata->func->op_array.T + i - arguments_wanted));
					}
				}
				tmp->varc++;
			}

			/* Sometimes not enough arguments are send to a user defined
			 * function, so we have to gather only the name for those extra. */
			if (tmp->user_defined == XDEBUG_USER_DEFINED && arguments_sent < arguments_wanted) {
				for (i = arguments_sent; i < arguments_wanted; i++) {
					if (op_array->arg_info[i].name) {
						tmp->var[tmp->varc].name = xdstrdup(STR_NAME_VAL(op_array->arg_info[i].name));
						tmp->var[tmp->varc].length = STR_NAME_LEN(op_array->arg_info[i].name);
					}
					ZVAL_UNDEF(&tmp->var[tmp->varc].data);
					tmp->var[tmp->varc].is_variadic = 0;
					tmp->varc++;
				}
			}
		}
	}

	/* Now we have location and name, we can run the filter */
	xdebug_filter_run_tracing(tmp);

	/* Count code coverage line for call */
	xdebug_coverage_count_line_if_branch_check_active(op_array, tmp->filename, tmp->lineno);

	if (XG_BASE(stack)) {
		if (XDEBUG_LLIST_TAIL(XG_BASE(stack))) {
			function_stack_entry *prev = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG_BASE(stack)));
			tmp->prev = prev;
		}
		xdebug_llist_insert_next(XG_BASE(stack), XDEBUG_LLIST_TAIL(XG_BASE(stack)), tmp);
	}

	return tmp;
}

static void xdebug_execute_ex(zend_execute_data *execute_data)
{
	zend_op_array        *op_array = &(execute_data->func->op_array);
	zend_execute_data    *edata = execute_data->prev_execute_data;
	function_stack_entry *fse, *xfse;
	int                   function_nr = 0;
	xdebug_llist_element *le;
	char                 *code_coverage_function_name = NULL;
	zend_string          *code_coverage_filename = NULL;
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

	if (XG_BASE(in_execution) && XG_BASE(level) == 0) {
		if (xdebug_lib_mode_is(XDEBUG_MODE_STEP_DEBUG)) {
			xdebug_debugger_set_program_name(op_array->filename);
			xdebug_debug_init_if_requested_at_startup();
		}

		if (xdebug_lib_mode_is(XDEBUG_MODE_GCSTATS)) {
			xdebug_gcstats_init_if_requested(op_array);
		}

		if (xdebug_lib_mode_is(XDEBUG_MODE_PROFILING)) {
			xdebug_profiler_init_if_requested(op_array);
		}

		if (xdebug_lib_mode_is(XDEBUG_MODE_TRACING)) {
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

	if (xdebug_lib_mode_is(XDEBUG_MODE_DEVELOP)) {
		xdebug_monitor_handler(fse);
	}
	if (xdebug_lib_mode_is(XDEBUG_MODE_TRACING)) {
		xdebug_tracing_execute_ex(function_nr, fse);
	}

	fse->execute_data = EG(current_execute_data)->prev_execute_data;
	if (ZEND_CALL_INFO(EG(current_execute_data)) & ZEND_CALL_HAS_SYMBOL_TABLE) {
		fse->symbol_table = EG(current_execute_data)->symbol_table;
	}
	if (Z_OBJ(EG(current_execute_data)->This)) {
		fse->This = &EG(current_execute_data)->This;
	} else {
		fse->This = NULL;
	}

	if (XG_BASE(stack) && (XINI_DEV(collect_vars) || XINI_DEV(show_local_vars) || xdebug_is_debug_connection_active())) {
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

	if (xdebug_lib_mode_is(XDEBUG_MODE_COVERAGE)) {
		code_coverage_init = xdebug_coverage_execute_ex(fse, op_array, &code_coverage_filename, &code_coverage_function_name);
	}

	if (xdebug_lib_mode_is(XDEBUG_MODE_STEP_DEBUG)) {
		/* If we're in an eval, we need to create an ID for it. */
		if (fse->function.type == XFUNC_EVAL) {
			xdebug_debugger_register_eval(fse);
		}

		/* Check for entry breakpoints */
		xdebug_debugger_handle_breakpoints(fse, XDEBUG_BREAKPOINT_TYPE_CALL);
	}

	if (xdebug_lib_mode_is(XDEBUG_MODE_PROFILING)) {
		xdebug_profiler_execute_ex(fse, op_array);
	}

	xdebug_old_execute_ex(execute_data);

	if (xdebug_lib_mode_is(XDEBUG_MODE_PROFILING)) {
		xdebug_profiler_execute_ex_end(fse);
	}

	if (code_coverage_init) {
		xdebug_coverage_execute_ex_end(fse, op_array, code_coverage_filename, code_coverage_function_name);
	}

	if (xdebug_lib_mode_is(XDEBUG_MODE_TRACING)) {
		xdebug_tracing_execute_ex_end(function_nr, fse, execute_data);
	}

	if (xdebug_lib_mode_is(XDEBUG_MODE_STEP_DEBUG)) {
		/* Check for return breakpoints */
		xdebug_debugger_handle_breakpoints(fse, XDEBUG_BREAKPOINT_TYPE_RETURN);
	}

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
#if PHP_VERSION_ID >= 80000
	void                (*tmp_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, zend_string *message) = NULL;
#else
	void                (*tmp_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, const char *format, va_list args) ZEND_ATTRIBUTE_PTR_FORMAT(printf, 4, 0) = NULL;
#endif

	XG_BASE(level)++;
	if ((signed long) XG_BASE(level) > XINI_BASE(max_nesting_level) && (XINI_BASE(max_nesting_level) != -1)) {
		zend_throw_exception_ex(zend_ce_error, 0, "Maximum function nesting level of '" ZEND_LONG_FMT "' reached, aborting!", XINI_BASE(max_nesting_level));
	}

	fse = xdebug_add_stack_frame(edata, &edata->func->op_array, XDEBUG_BUILT_IN);
	fse->function.internal = 1;

	function_nr = XG_BASE(function_count);

	if (xdebug_lib_mode_is(XDEBUG_MODE_DEVELOP)) {
		xdebug_monitor_handler(fse);
	}
	if (xdebug_lib_mode_is(XDEBUG_MODE_TRACING)) {
		function_call_traced = xdebug_tracing_execute_internal(function_nr, fse);
	}

	if (xdebug_lib_mode_is(XDEBUG_MODE_STEP_DEBUG)) {
		/* Check for entry breakpoints */
		xdebug_debugger_handle_breakpoints(fse, XDEBUG_BREAKPOINT_TYPE_CALL);
	}

	/* Check for SOAP */
	if (check_soap_call(fse, current_execute_data)) {
		restore_error_handler_situation = 1;
		tmp_error_cb = zend_error_cb;
		xdebug_base_use_original_error_cb();
	}

	if (xdebug_lib_mode_is(XDEBUG_MODE_PROFILING)) {
		xdebug_profiler_execute_internal(fse);
	}

	if (xdebug_old_execute_internal) {
		xdebug_old_execute_internal(current_execute_data, return_value);
	} else {
		execute_internal(current_execute_data, return_value);
	}

	if (xdebug_lib_mode_is(XDEBUG_MODE_PROFILING)) {
		xdebug_profiler_execute_internal_end(fse);
	}

	/* Restore SOAP situation if needed */
	if (restore_error_handler_situation) {
		zend_error_cb = tmp_error_cb;
	}

	/* We only call the function_exit handler and return value handler if the
	 * function call was also traced. Otherwise we end up with return trace
	 * lines without a corresponding function call line. */
	if (xdebug_lib_mode_is(XDEBUG_MODE_TRACING) && function_call_traced) {
		xdebug_tracing_execute_internal_end(function_nr, fse, return_value);
	}

	if (xdebug_lib_mode_is(XDEBUG_MODE_STEP_DEBUG)) {
		/* Check for return breakpoints */
		xdebug_debugger_handle_breakpoints(fse, XDEBUG_BREAKPOINT_TYPE_RETURN);
	}

	if (XG_BASE(stack)) {
		xdebug_llist_remove(XG_BASE(stack), XDEBUG_LLIST_TAIL(XG_BASE(stack)), function_stack_entry_dtor);
	}
	XG_BASE(level)--;
}

static void xdebug_base_overloaded_functions_setup(void)
{
	zend_function *orig;

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

	/* Override pcntl_fork with our own function to be able
	 * to start the debugger for the forked process */
	orig = zend_hash_str_find_ptr(EG(function_table), "pcntl_fork", sizeof("pcntl_fork") - 1);
	if (orig) {
		XG_BASE(orig_pcntl_fork_func) = orig->internal_function.handler;
		orig->internal_function.handler = zif_xdebug_pcntl_fork;
	} else {
		XG_BASE(orig_pcntl_fork_func) = NULL;
	}
}

static void xdebug_base_overloaded_functions_restore(void)
{
	zend_function *orig;

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

	if (XG_BASE(orig_pcntl_fork_func)) {
		orig = zend_hash_str_find_ptr(EG(function_table), "pcntl_fork", sizeof("pcntl_fork") - 1);
		if (orig) {
			orig->internal_function.handler = XG_BASE(orig_pcntl_fork_func);
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
	/* Record Zend and Xdebug error callbacks, the actual setting is done in
	 * base on RINIT */
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
	if (
		(xdebug_lib_mode_is(XDEBUG_MODE_DEVELOP) || xdebug_lib_mode_is(XDEBUG_MODE_STEP_DEBUG))
		&&
		(zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_SOAPACTION", sizeof("HTTP_SOAPACTION") - 1) == NULL)
	) {
		xdebug_base_use_xdebug_error_cb();
		xdebug_base_use_xdebug_throw_exception_hook();
	}

	XG_BASE(stack) = xdebug_llist_alloc(function_stack_entry_dtor);
	XG_BASE(level)         = 0;
	XG_BASE(in_debug_info) = 0;
	XG_BASE(prev_memory)   = 0;
	XG_BASE(function_count) = -1;
	XG_BASE(last_eval_statement) = NULL;
	XG_BASE(last_exception_trace) = NULL;

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

	xdebug_base_overloaded_functions_setup();
}

void xdebug_base_post_deactivate()
{
	xdebug_llist_destroy(XG_BASE(stack), NULL);
	XG_BASE(stack) = NULL;

	XG_BASE(level)            = 0;
	XG_BASE(in_debug_info)    = 0;

	if (XG_BASE(last_eval_statement)) {
		zend_string_release(XG_BASE(last_eval_statement));
		XG_BASE(last_eval_statement) = NULL;
	}
	if (XG_BASE(last_exception_trace)) {
		xdfree(XG_BASE(last_exception_trace));
		XG_BASE(last_exception_trace) = NULL;
	}

	/* filters */
	xdebug_llist_destroy(XG_BASE(filters_tracing), NULL);
	xdebug_llist_destroy(XG_BASE(filters_code_coverage), NULL);
	XG_BASE(filters_tracing) = NULL;
	XG_BASE(filters_code_coverage) = NULL;

	xdebug_base_overloaded_functions_restore();
}

void xdebug_base_rshutdown()
{
	/* Signal that we're no longer in a request */
	XG_BASE(in_execution) = 0;
}


void xdebug_base_use_original_error_cb(void)
{
	zend_error_cb = xdebug_old_error_cb;
}

void xdebug_base_use_xdebug_error_cb(void)
{
	zend_error_cb = xdebug_new_error_cb;
}

static void xdebug_throw_exception_hook(zval *exception)
{
	zval *code, *message, *file, *line;
	zend_class_entry *exception_ce;
	char *code_str = NULL;
	zval dummy;

	if (!xdebug_lib_mode_is(XDEBUG_MODE_DEVELOP) && !xdebug_lib_mode_is(XDEBUG_MODE_STEP_DEBUG)) {
		return;
	}
	if (!exception) {
		return;
	}

	exception_ce = Z_OBJCE_P(exception);

	code =    zend_read_property(exception_ce, exception, "code",    sizeof("code")-1,    0, &dummy);
	message = zend_read_property(exception_ce, exception, "message", sizeof("message")-1, 0, &dummy);
	file =    zend_read_property(exception_ce, exception, "file",    sizeof("file")-1,    0, &dummy);
	line =    zend_read_property(exception_ce, exception, "line",    sizeof("line")-1,    0, &dummy);

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

	if (xdebug_lib_mode_is(XDEBUG_MODE_DEVELOP)) {
		xdebug_develop_throw_exception_hook(exception, file, line, code, code_str, message);
	}
	if (xdebug_lib_mode_is(XDEBUG_MODE_STEP_DEBUG)) {
		xdebug_debugger_throw_exception_hook(exception, file, line, code, code_str, message);
	}

	/* Free code_str if necessary */
	if (code_str) {
		xdfree(code_str);
	}
}

void xdebug_base_use_xdebug_throw_exception_hook(void)
{
	zend_throw_exception_hook = xdebug_throw_exception_hook;
}

/* {{{ proto void xdebug_set_time_limit(void)
   Dummy function to prevent time limit from being set within the script */
PHP_FUNCTION(xdebug_set_time_limit)
{
	if (!xdebug_is_debug_connection_active()) {
		XG_BASE(orig_set_time_limit_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	}
}
/* }}} */

/* {{{ proto int xdebug_error_reporting(void)
   Dummy function to return original error reporting level when 'eval' has turned it into 0 */
PHP_FUNCTION(xdebug_error_reporting)
{
	if (ZEND_NUM_ARGS() == 0 && XG_BASE(error_reporting_overridden) && xdebug_is_debug_connection_active()) {
		RETURN_LONG(XG_BASE(error_reporting_override));
	}
	XG_BASE(orig_error_reporting_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto void xdebug_pcntl_exec(void)
   Dummy function to stop profiling when we run pcntl_exec */
PHP_FUNCTION(xdebug_pcntl_exec)
{
	/* We need to stop the profiler and trace files here */
	xdebug_profiler_pcntl_exec_handler();

	XG_BASE(orig_pcntl_exec_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto int xdebug_pcntl_fork(void)
   Dummy function to set a new connection when forking a process */
PHP_FUNCTION(xdebug_pcntl_fork)
{
	XG_BASE(orig_pcntl_fork_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

	xdebug_debugger_restart_if_pid_changed();
}
/* }}} */
