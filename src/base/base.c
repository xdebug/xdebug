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

#include "php.h"
#include "TSRM.h"
#include "php_globals.h"
#include "zend_closures.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"

#if PHP_VERSION_ID >= 80100
# include "base_private.h"
# include "Zend/zend_fibers.h"
# include "Zend/zend_observer.h"
#endif

#include "php_xdebug.h"
#include "php_xdebug_arginfo.h"

#include "base.h"
#include "filter.h"
#include "develop/develop.h"
#include "develop/stack.h"
#include "gcstats/gc_stats.h"
#include "lib/lib_private.h"
#include "lib/log.h"
#include "lib/var_export_line.h"
#include "lib/var.h"
#include "profiler/profiler.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

#if PHP_VERSION_ID >= 80100
void (*xdebug_old_error_cb)(int type, zend_string *error_filename, const uint32_t error_lineno, zend_string *message);
void (*xdebug_new_error_cb)(int type, zend_string *error_filename, const uint32_t error_lineno, zend_string *message);
static void xdebug_error_cb(int orig_type, zend_string *error_filename, const uint32_t error_lineno, zend_string *message);
#elif PHP_VERSION_ID >= 80000
void (*xdebug_old_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, zend_string *message);
void (*xdebug_new_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, zend_string *message);
static void xdebug_error_cb(int orig_type, const char *error_filename, const uint32_t error_lineno, zend_string *message);
#else
void (*xdebug_old_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, const char *format, va_list args) ZEND_ATTRIBUTE_PTR_FORMAT(printf, 4, 0);
void (*xdebug_new_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, const char *format, va_list args) ZEND_ATTRIBUTE_PTR_FORMAT(printf, 4, 0);
static void xdebug_error_cb(int orig_type, const char *error_filename, const uint32_t error_lineno, const char *format, va_list args) ZEND_ATTRIBUTE_PTR_FORMAT(printf, 4, 0);
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

	if (!op_array) {
		return NULL;
	}

	xdebug_coverage_compile_file(op_array);
	xdebug_debugger_compile_file(op_array);

	return op_array;
}
/* }}} */

/* I don't like this API, but the function_stack_entry does not keep this as a
 * pointer, and hence we need two APIs for freeing :-S */
void xdebug_func_dtor_by_ref(xdebug_func *elem)
{
	if (elem->function) {
		xdfree(elem->function);
	}
	if (elem->object_class) {
		zend_string_release(elem->object_class);
	}
	if (elem->scope_class) {
		zend_string_release(elem->scope_class);
	}
}

void xdebug_func_dtor(xdebug_func *elem)
{
	xdebug_func_dtor_by_ref(elem);
	xdfree(elem);
}

static void function_stack_entry_dtor(void *elem)
{
	unsigned int          i;
	function_stack_entry *e = elem;

	xdebug_func_dtor_by_ref(&e->function);

	if (e->filename) {
		zend_string_release(e->filename);
	}

	if (e->var) {
		for (i = 0; i < e->varc; i++) {
			if (e->var[i].name) {
				zend_string_release(e->var[i].name);
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
				char *tmp_object_class = xdebug_sprintf(
					"{anonymous-class:%s:%d-%d}",
					edata->func->common.scope->info.user.filename->val,
					edata->func->common.scope->info.user.line_start,
					edata->func->common.scope->info.user.line_end
				);
				tmp->object_class = zend_string_init(tmp_object_class, strlen(tmp_object_class), 0);
				xdfree(tmp_object_class);
			} else {
				if (edata->func->common.scope) {
					tmp->scope_class = zend_string_copy(edata->func->common.scope->name);
				}
				tmp->object_class = zend_string_copy(edata->This.value.obj->ce->name);
			}
		} else {
			if (edata->func->common.scope) {
				tmp->type = XFUNC_STATIC_MEMBER;
				tmp->object_class = zend_string_copy(edata->func->common.scope->name);
			}
		}
		if (edata->func->common.function_name) {
			if (edata->func->common.fn_flags & ZEND_ACC_CLOSURE) {
				tmp->function = xdebug_wrap_closure_location_around_function_name(&edata->func->op_array, STR_NAME_VAL(edata->func->common.function_name));
			} else if (strncmp(edata->func->common.function_name->val, "call_user_func", 14) == 0) {
				zend_string *fname = NULL;
				int          lineno = 0;

				if (edata->prev_execute_data && edata->prev_execute_data->func && edata->prev_execute_data->func->type == ZEND_USER_FUNCTION) {
					fname = edata->prev_execute_data->func->op_array.filename;
				}

				if (!fname) {
					function_stack_entry *tmp_fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));

					if (tmp_fse->filename) {
						fname = tmp_fse->filename;
					}
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
			edata->prev_execute_data->func &&
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

#define NO_VARIADIC    INT_MAX
#define DEBUG          0

static void collect_params_internal(function_stack_entry *fse, zend_execute_data *zdata, zend_op_array *op_array)
{
	int i;
	int is_variadic       = !!(zdata->func->common.fn_flags & ZEND_ACC_VARIADIC);
	int is_trampoline     = !!(zdata->func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE);
	int variadic_at_pos   = NO_VARIADIC;
	int names_expected    = 0;
	int arguments_sent    = 0;
	int arguments_storage = 0;

	arguments_sent = ZEND_CALL_NUM_ARGS(zdata);
	if (arguments_sent > USHRT_MAX) {
		return;
	}

	names_expected = zdata->func->internal_function.num_args;
	if (names_expected > arguments_sent) {
		names_expected = arguments_sent;
	}
#if DEBUG
	fprintf(stderr, "\nF: %s\n - CALL_NUM_ARGS: %d, op_array->num_args: %d, is_variadic: %d, trampoline: %d\n", fse->function.function, ZEND_CALL_NUM_ARGS(zdata), op_array->num_args, is_variadic, is_trampoline);
#endif

	/* If this function is variadic, we have an extra name field in arg_info, and also an extra
	 * argument sent to the function. */
	if (is_variadic && !is_trampoline) {
		names_expected++;
	}

	/* Pick the highest of "expected arguments" and "arguments given" (also
	 * taking into account the extra one for variadics */
	if (names_expected > arguments_sent) {
		arguments_storage = names_expected;
	} else {
		arguments_storage = arguments_sent;
	}

	fse->varc = arguments_storage;
	fse->var = xdmalloc(fse->varc * sizeof(xdebug_var_name));

#if DEBUG
	fprintf(stderr, " - names_expected: %d, arguments_sent: %d, arguments_storage: %d, fse->varc: %d\n", names_expected, arguments_sent, arguments_storage, fse->varc);
#endif

	/* Initialise everything in storage */
	for (i = 0; i < fse->varc; i++) {
		fse->var[i].name = NULL;
		ZVAL_UNDEF(&fse->var[i].data);
		fse->var[i].is_variadic = 0;
	}

	/* Collect Names */
	for (i = 0; i < names_expected; i++) {
		if (op_array->arg_info[i].name) {
			fse->var[i].name = zend_string_init(
				zdata->func->internal_function.arg_info[i].name,
				strlen(zdata->func->internal_function.arg_info[i].name),
				0
			);

			/* If an argument is a variadic, then we mark that on this 'name',
			 * and also remember which position the variadic started */
			if (ZEND_ARG_IS_VARIADIC(&op_array->arg_info[i]) && variadic_at_pos == NO_VARIADIC) {
				fse->var[i].is_variadic = 1;
				variadic_at_pos = i;
			}
		}
	}

	/* Collect Arguments */
	for (i = 0; i < arguments_sent; i++) {
		/* The index in ZEND_CALL_ARG is 1-based */
#if DEBUG
		fprintf(stderr, "Copying argument %d: ", i);
		fprintf(stderr, "ARG ");
#endif
		ZVAL_COPY(&(fse->var[i].data), ZEND_CALL_ARG(zdata, i + 1));
#if DEBUG
		fprintf(stderr, "OK\n");
#endif
	}

#if PHP_VERSION_ID >= 80000
	if (ZEND_CALL_INFO(zdata) & ZEND_CALL_HAS_EXTRA_NAMED_PARAMS) {
		zend_string *name;
		zval        *param;
		int          i = fse->varc;

		fse->varc += zend_hash_num_elements(zdata->extra_named_params);
		fse->var = xdrealloc(fse->var, fse->varc * sizeof(xdebug_var_name));

		ZEND_HASH_FOREACH_STR_KEY_VAL(zdata->extra_named_params, name, param) {
			fse->var[i].name = zend_string_copy(name);
			ZVAL_COPY(&(fse->var[i].data), param);
			fse->var[i].is_variadic = 0;
			i++;
		} ZEND_HASH_FOREACH_END();
	}
#endif

#if DEBUG
	for (i = 0; i < fse->varc; i++) {
		fprintf(stderr, "%2d %-20s %c %s\n", i, fse->var[i].name ? ZSTR_VAL(fse->var[i].name) : "---", fse->var[i].is_variadic ? 'V' : ' ', xdebug_get_zval_value_line(&fse->var[i].data, 0, NULL)->d);
	}
#endif
}

static void collect_params(function_stack_entry *fse, zend_execute_data *zdata, zend_op_array *op_array)
{
	int i;
	int is_variadic       = !!(zdata->func->common.fn_flags & ZEND_ACC_VARIADIC);
	int is_trampoline     = !!(zdata->func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE);
	int variadic_at_pos   = NO_VARIADIC;
	int names_expected    = 0;
	int arguments_sent    = 0;
	int arguments_storage = 0;

	arguments_sent = ZEND_CALL_NUM_ARGS(zdata);

	/* The op_array contains the number of * (named) arguments. */
	names_expected = op_array->num_args;

#if DEBUG
		fprintf(stderr, "\nF: %s\n - CALL_NUM_ARGS: %d, op_array->num_args: %d, is_variadic: %d, trampoline: %d\n", fse->function.function, ZEND_CALL_NUM_ARGS(zdata), op_array->num_args, is_variadic, is_trampoline);
#endif

	/* If this function is variadic, we have an extra name field in arg_info, and also an extra
	 * argument sent to the function. */
	if (is_variadic && !is_trampoline) {
		names_expected++;
		arguments_sent++;
	}

	/* Pick the highest of "expected arguments" and "arguments given" (also
	 * taking into account the extra one for variadics */
	if (names_expected > arguments_sent) {
		arguments_storage = names_expected;
	} else {
		arguments_storage = arguments_sent;
	}

	fse->varc = arguments_storage;
	fse->var = xdmalloc(fse->varc * sizeof(xdebug_var_name));

#if DEBUG
	fprintf(stderr, " - names_expected: %d, arguments_sent: %d, arguments_storage: %d, fse->varc: %d\n", names_expected, arguments_sent, arguments_storage, fse->varc);
#endif

	/* Initialise everything in storage */
	for (i = 0; i < fse->varc; i++) {
		fse->var[i].name = NULL;
		ZVAL_UNDEF(&fse->var[i].data);
		fse->var[i].is_variadic = 0;
	}

	/* Collect Names */
	for (i = 0; i < names_expected; i++) {
		if (op_array->arg_info[i].name) {
			fse->var[i].name = zend_string_copy(op_array->arg_info[i].name);
		}

		/* If an argument is a variadic, then we mark that on this 'name',
		 * and also remember which position the variadic started */
		if (ZEND_ARG_IS_VARIADIC(&op_array->arg_info[i]) && variadic_at_pos == NO_VARIADIC) {
			fse->var[i].is_variadic = 1;
			variadic_at_pos = i;
		}
	}

	/* Collect Arguments */
	for (i = 0; i < fse->varc; i++) {
		/* The index in ZEND_CALL_ARG is 1-based */
#if DEBUG
		fprintf(stderr, "Copying argument %d: ", i);
#endif
		if (i < names_expected || is_trampoline) {
#if DEBUG
			fprintf(stderr, "ARG ");
#endif
			ZVAL_COPY(&(fse->var[i].data), ZEND_CALL_ARG(zdata, i + 1));
		} else {
#if DEBUG
			fprintf(stderr, "VAR_NUM ");
#endif
			ZVAL_COPY(&(fse->var[i].data), ZEND_CALL_VAR_NUM(zdata, zdata->func->op_array.last_var + zdata->func->op_array.T + i - names_expected));
		}
#if DEBUG
		fprintf(stderr, "OK\n");
#endif
	}

#if PHP_VERSION_ID >= 80000
	if (ZEND_CALL_INFO(zdata) & ZEND_CALL_HAS_EXTRA_NAMED_PARAMS) {
		zend_string *name;
		zval        *param;
		int          i = fse->varc;

		fse->varc += zend_hash_num_elements(zdata->extra_named_params);
		fse->var = xdrealloc(fse->var, fse->varc * sizeof(xdebug_var_name));

		ZEND_HASH_FOREACH_STR_KEY_VAL(zdata->extra_named_params, name, param) {
			fse->var[i].name = zend_string_copy(name);
			ZVAL_COPY(&(fse->var[i].data), param);
			fse->var[i].is_variadic = 0;
			i++;
		} ZEND_HASH_FOREACH_END();
	}
#endif

#if DEBUG
	for (i = 0; i < fse->varc; i++) {
		fprintf(stderr, "%2d %-20s %c %s\n", i, fse->var[i].name ? ZSTR_VAL(fse->var[i].name) : "---", fse->var[i].is_variadic ? 'V' : ' ', xdebug_get_zval_value_line(&fse->var[i].data, 0, NULL)->d);
	}
#endif
}

function_stack_entry *xdebug_add_stack_frame(zend_execute_data *zdata, zend_op_array *op_array, int type)
{
	zend_execute_data    *edata;
	zend_op             **opline_ptr = NULL;
	function_stack_entry *tmp;
	zend_op              *cur_opcode;

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

	tmp = (function_stack_entry*) xdebug_vector_push(XG_BASE(stack));
	tmp->level         = XDEBUG_VECTOR_COUNT(XG_BASE(stack));
	tmp->user_defined  = type;
	tmp->op_array      = op_array;

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
	if (!tmp->filename && XG_BASE(stack)) {
		function_stack_entry *tail_fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));
		if (tail_fse->filename) {
			tmp->filename = zend_string_copy(tail_fse->filename);
		}
	}

	if (!tmp->filename) {
		tmp->filename = zend_string_init("Unknown", sizeof("Unknown") - 1, 0);
	}
	tmp->lineno = 0;

	tmp->prev_memory = XG_BASE(prev_memory);
	tmp->memory = zend_memory_usage(0);
	XG_BASE(prev_memory) = tmp->memory;

	/* Only get the time when it is actually going to be used. Profiling is not
	 * included, because it has its own points when it reads the current time.
	 * */
	if (XDEBUG_MODE_IS(XDEBUG_MODE_TRACING) || XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		tmp->nanotime = xdebug_get_nanotime();
	} else {
		tmp->nanotime = 0;
	}

	xdebug_build_fname(&(tmp->function), zdata);
	if (!tmp->function.type) {
		tmp->function.function     = xdstrdup("{main}");
		tmp->function.object_class = NULL;
		tmp->function.scope_class  = NULL;
		tmp->function.type         = XFUNC_MAIN;

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
	} else {
		tmp->lineno = find_line_number_for_current_execute_point(edata);
		tmp->is_variadic = !!(zdata->func->common.fn_flags & ZEND_ACC_VARIADIC);

		if (XDEBUG_MODE_IS(XDEBUG_MODE_TRACING) || XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
			if (ZEND_USER_CODE(zdata->func->type)) {
				collect_params(tmp, zdata, op_array);
			} else {
				collect_params_internal(tmp, zdata, op_array);
			}
		}
	}

	/* Now we have location and name, we can run the filter (for stack and tracing)*/
	xdebug_filter_run(tmp);

	/* Count code coverage line for call */
	xdebug_coverage_count_line_if_branch_check_active(op_array, tmp->filename, tmp->lineno);

	return tmp;
}

static void xdebug_execute_ex(zend_execute_data *execute_data)
{
	zend_op_array        *op_array = &(execute_data->func->op_array);
	zend_execute_data    *edata = execute_data->prev_execute_data;
	function_stack_entry *fse;
	int                   function_nr = 0;
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

	/* If the stack vector hasn't been initialised yet, we should abort immediately */
	if (!XG_BASE(stack)) {
		xdebug_old_execute_ex(execute_data);
		return;
	}

	if (XG_BASE(in_execution) && XDEBUG_VECTOR_COUNT(XG_BASE(stack)) == 0) {
		if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
			xdebug_debugger_set_program_name(op_array->filename);
			xdebug_debug_init_if_requested_at_startup();
		}

		if (XDEBUG_MODE_IS(XDEBUG_MODE_GCSTATS)) {
			xdebug_gcstats_init_if_requested(op_array);
		}

		if (XDEBUG_MODE_IS(XDEBUG_MODE_PROFILING)) {
			xdebug_profiler_init_if_requested(op_array);
		}

		if (XDEBUG_MODE_IS(XDEBUG_MODE_TRACING)) {
			xdebug_tracing_init_if_requested(op_array);
		}
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP) && (signed long) XDEBUG_VECTOR_COUNT(XG_BASE(stack)) >= XINI_BASE(max_nesting_level) && (XINI_BASE(max_nesting_level) != -1)) {
		zend_throw_exception_ex(zend_ce_error, 0, "Xdebug has detected a possible infinite loop, and aborted your script with a stack depth of '" ZEND_LONG_FMT "' frames", XINI_BASE(max_nesting_level));
	}

	fse = xdebug_add_stack_frame(edata, op_array, XDEBUG_USER_DEFINED);
	fse->function.internal = 0;

	/* A hack to make __call work with profiles. The function *is* user defined after all. */
	if (fse && xdebug_vector_element_is_valid(XG_BASE(stack), fse - 1) && fse->function.function && (strcmp(fse->function.function, "__call") == 0)) {
		(fse - 1)->user_defined = XDEBUG_USER_DEFINED;
	}

	function_nr = XG_BASE(function_count);

	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		xdebug_monitor_handler(fse);
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_TRACING)) {
		xdebug_tracing_execute_ex(function_nr, fse);
	}

	fse->execute_data = EG(current_execute_data)->prev_execute_data;
	if (ZEND_CALL_INFO(EG(current_execute_data)) & ZEND_CALL_HAS_SYMBOL_TABLE) {
		fse->symbol_table = EG(current_execute_data)->symbol_table;
	}

	if (XG_BASE(stack) && (XINI_DEV(show_local_vars) || xdebug_is_debug_connection_active())) {
		/* Because include/require is treated as a stack level, we have to add used
		 * variables in include/required files to all the stack levels above, until
		 * we hit a function or the top level stack.  This is so that the variables
		 * show up correctly where they should be.  We always call
		 * xdebug_lib_register_compiled_variables on the current stack level,
		 * otherwise vars in include files do not show up in the locals list. */
		function_stack_entry *loop_fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));
		int                   i;

		for (i = 0; i < XDEBUG_VECTOR_COUNT(XG_BASE(stack)); i++, loop_fse--) {
			xdebug_lib_register_compiled_variables(loop_fse, op_array);
			if (XDEBUG_IS_NORMAL_FUNCTION(&loop_fse->function)) {
				break;
			}
		}
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_COVERAGE)) {
		code_coverage_init = xdebug_coverage_execute_ex(fse, op_array, &code_coverage_filename, &code_coverage_function_name);
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		/* If we're in an eval, we need to create an ID for it. */
		if (fse->function.type == XFUNC_EVAL) {
			xdebug_debugger_register_eval(fse);
		}

		/* Check for entry breakpoints */
		xdebug_debugger_handle_breakpoints(fse, XDEBUG_BREAKPOINT_TYPE_CALL);
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_PROFILING)) {
		xdebug_profiler_execute_ex(fse, op_array);
	}

	xdebug_old_execute_ex(execute_data);

	/* Re-acquire the tail as nested calls through xdebug_old_execute_ex()
	 * might have reallocated the vector */
	fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));

	if (XDEBUG_MODE_IS(XDEBUG_MODE_PROFILING)) {
		xdebug_profiler_execute_ex_end(fse);
	}

	if (code_coverage_init) {
		xdebug_coverage_execute_ex_end(fse, op_array, code_coverage_filename, code_coverage_function_name);
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_TRACING)) {
		xdebug_tracing_execute_ex_end(function_nr, fse, execute_data);
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		/* Check for return breakpoints */
		xdebug_debugger_handle_breakpoints(fse, XDEBUG_BREAKPOINT_TYPE_RETURN);
	}

	fse->symbol_table = NULL;
	fse->execute_data = NULL;

	if (XG_BASE(stack)) {
		xdebug_vector_pop(XG_BASE(stack));
	}
}

static int check_soap_call(function_stack_entry *fse, zend_execute_data *execute_data)
{
	if (
		fse->function.object_class &&
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
#if PHP_VERSION_ID >= 80100
	void                (*tmp_error_cb)(int type, zend_string *error_filename, const uint32_t error_lineno, zend_string *message) = NULL;
#elif PHP_VERSION_ID >= 80000
	void                (*tmp_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, zend_string *message) = NULL;
#else
	void                (*tmp_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, const char *format, va_list args) ZEND_ATTRIBUTE_PTR_FORMAT(printf, 4, 0) = NULL;
#endif

	/* If the stack vector hasn't been initialised yet, we should abort immediately */
	if (!XG_BASE(stack)) {
		if (xdebug_old_execute_internal) {
			xdebug_old_execute_internal(current_execute_data, return_value);
		} else {
			execute_internal(current_execute_data, return_value);
		}
		return;
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP) && (signed long) XDEBUG_VECTOR_COUNT(XG_BASE(stack)) >= XINI_BASE(max_nesting_level) && (XINI_BASE(max_nesting_level) != -1)) {
		zend_throw_exception_ex(zend_ce_error, 0, "Xdebug has detected a possible infinite loop, and aborted your script with a stack depth of '" ZEND_LONG_FMT "' frames", XINI_BASE(max_nesting_level));
	}

	fse = xdebug_add_stack_frame(edata, &edata->func->op_array, XDEBUG_BUILT_IN);
	fse->function.internal = 1;

	function_nr = XG_BASE(function_count);

	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		xdebug_monitor_handler(fse);
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_TRACING)) {
		function_call_traced = xdebug_tracing_execute_internal(function_nr, fse);
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		/* Check for entry breakpoints */
		xdebug_debugger_handle_breakpoints(fse, XDEBUG_BREAKPOINT_TYPE_CALL);
	}

	/* Check for SOAP */
	if (check_soap_call(fse, current_execute_data)) {
		restore_error_handler_situation = 1;
		tmp_error_cb = zend_error_cb;
		xdebug_base_use_original_error_cb();
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_PROFILING)) {
		xdebug_profiler_execute_internal(fse);
	}

	if (xdebug_old_execute_internal) {
		xdebug_old_execute_internal(current_execute_data, return_value);
	} else {
		execute_internal(current_execute_data, return_value);
	}

	/* Re-acquire the tail as nested calls through
	 * xdebug_old_execute_internal() might have reallocated the vector */
	fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));

	if (XDEBUG_MODE_IS(XDEBUG_MODE_PROFILING)) {
		xdebug_profiler_execute_internal_end(fse);
	}

	/* Restore SOAP situation if needed */
	if (restore_error_handler_situation) {
		zend_error_cb = tmp_error_cb;
	}

	/* We only call the function_exit handler and return value handler if the
	 * function call was also traced. Otherwise we end up with return trace
	 * lines without a corresponding function call line. */
	if (XDEBUG_MODE_IS(XDEBUG_MODE_TRACING) && function_call_traced) {
		xdebug_tracing_execute_internal_end(function_nr, fse, return_value);
	}

	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		/* Check for return breakpoints */
		xdebug_debugger_handle_breakpoints(fse, XDEBUG_BREAKPOINT_TYPE_RETURN);
	}

	if (XG_BASE(stack)) {
		xdebug_vector_pop(XG_BASE(stack));
	}
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

#if PHP_VERSION_ID >= 80100
static struct xdebug_fiber_entry* xdebug_fiber_entry_ctor(xdebug_vector *stack)
{
	struct xdebug_fiber_entry *tmp = xdmalloc(sizeof(struct xdebug_fiber_entry));

	tmp->stack = stack;

	return tmp;
}

static void xdebug_fiber_entry_dtor(struct xdebug_fiber_entry *entry)
{
	xdebug_vector_destroy(entry->stack);
	xdfree(entry);
}

static xdebug_str *create_key_for_fiber(zend_fiber_context *fiber)
{
	xdebug_str *tmp = xdebug_str_new();

	xdebug_str_add_fmt(tmp, "{fiber:%0" PRIXPTR "}", ((uintptr_t) fiber));

	return tmp;
}

static void add_fiber_main(zend_fiber_context *fiber)
{
	xdebug_str           *key = create_key_for_fiber(fiber);
	function_stack_entry *tmp = (function_stack_entry*) xdebug_vector_push(XG_BASE(stack));

	tmp->level = 0;
	tmp->user_defined = XDEBUG_BUILT_IN;
	tmp->function.type = XFUNC_FIBER;
	tmp->function.object_class = NULL;
	tmp->function.scope_class = NULL;
	tmp->function.function = xdstrdup(key->d);
}

static xdebug_vector* create_stack_for_fiber(zend_fiber_context *fiber)
{
	xdebug_vector             *tmp_stack = xdebug_vector_alloc(sizeof(function_stack_entry), function_stack_entry_dtor);
	xdebug_str                *key       = create_key_for_fiber(fiber);
	struct xdebug_fiber_entry *entry     = xdebug_fiber_entry_ctor(tmp_stack);

	xdebug_hash_add(XG_BASE(fiber_stacks), key->d, key->l, entry);

	xdebug_str_free(key);

	return tmp_stack;
}

static void remove_stack_for_fiber(zend_fiber_context *fiber)
{
	xdebug_str *key = create_key_for_fiber(fiber);

	xdebug_hash_delete(XG_BASE(fiber_stacks), key->d, key->l);

	xdebug_str_free(key);
}

static xdebug_vector *find_stack_for_fiber(zend_fiber_context *fiber)
{
	struct xdebug_fiber_entry *entry = NULL;
	xdebug_str                *key = create_key_for_fiber(fiber);

	xdebug_hash_find(XG_BASE(fiber_stacks), key->d, key->l, (void*) &entry);

	xdebug_str_free(key);

	return entry->stack;
}

static void xdebug_fiber_switch_observer(zend_fiber_context *from, zend_fiber_context *to)
{
	xdebug_vector *current_stack;

	if (from->status == ZEND_FIBER_STATUS_DEAD) {
		remove_stack_for_fiber(from);
	}
	if (to->status == ZEND_FIBER_STATUS_INIT) {
		current_stack = create_stack_for_fiber(to);
	} else {
		current_stack = find_stack_for_fiber(to);
	}
	XG_BASE(stack) = current_stack;

	if (to->status == ZEND_FIBER_STATUS_INIT) {
		add_fiber_main(to);
	}
}
#endif

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

	xdebug_nanotime_init();

#if PHP_VERSION_ID >= 80100
	zend_observer_fiber_switch_register(xdebug_fiber_switch_observer);
#endif
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
		(XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP) || XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG))
		&&
		(zend_hash_str_find(Z_ARR(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_SOAPACTION", sizeof("HTTP_SOAPACTION") - 1) == NULL)
	) {
		xdebug_base_use_xdebug_error_cb();
		xdebug_base_use_xdebug_throw_exception_hook();
	}

#if PHP_VERSION_ID >= 80100
	XG_BASE(fiber_stacks) = xdebug_hash_alloc(64, (xdebug_hash_dtor_t) xdebug_fiber_entry_dtor);
	XG_BASE(stack) = create_stack_for_fiber(EG(main_fiber_context));
#else
	XG_BASE(stack) = xdebug_vector_alloc(sizeof(function_stack_entry), function_stack_entry_dtor);
#endif
	XG_BASE(in_debug_info) = 0;
	XG_BASE(prev_memory)   = 0;
	XG_BASE(function_count) = -1;
	XG_BASE(last_eval_statement) = NULL;
	XG_BASE(last_exception_trace) = NULL;

	/* Initialize start time */
	if (XDEBUG_MODE_IS(XDEBUG_MODE_TRACING) || XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		XG_BASE(start_nanotime) = xdebug_get_nanotime();
	} else {
		XG_BASE(start_nanotime) = 0;
	}

	XG_BASE(in_var_serialisation) = 0;
	zend_ce_closure->serialize = xdebug_closure_serialize_deny_wrapper;

	/* Signal that we're in a request now */
	XG_BASE(in_execution) = 1;

	/* filters */
	XG_BASE(filter_type_code_coverage) = XDEBUG_FILTER_NONE;
	XG_BASE(filter_type_stack)         = XDEBUG_FILTER_NONE;
	XG_BASE(filter_type_tracing)       = XDEBUG_FILTER_NONE;
	XG_BASE(filters_code_coverage)     = xdebug_llist_alloc(xdebug_llist_string_dtor);
	XG_BASE(filters_stack)             = xdebug_llist_alloc(xdebug_llist_string_dtor);
	XG_BASE(filters_tracing)           = xdebug_llist_alloc(xdebug_llist_string_dtor);

	xdebug_base_overloaded_functions_setup();
}

void xdebug_base_post_deactivate()
{
#if PHP_VERSION_ID >= 80100
	xdebug_hash_destroy(XG_BASE(fiber_stacks));
	XG_BASE(fiber_stacks) = NULL;
#else
	xdebug_vector_destroy(XG_BASE(stack));
#endif
	XG_BASE(stack) = NULL;

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
	xdebug_llist_destroy(XG_BASE(filters_code_coverage), NULL);
	xdebug_llist_destroy(XG_BASE(filters_stack), NULL);
	xdebug_llist_destroy(XG_BASE(filters_tracing), NULL);
	XG_BASE(filters_tracing) = NULL;
	XG_BASE(filters_code_coverage) = NULL;

	xdebug_base_overloaded_functions_restore();
}

void xdebug_base_rshutdown()
{
	/* Signal that we're no longer in a request */
	XG_BASE(in_execution) = 0;
}

/* Error callback for formatting stack traces */
#if PHP_VERSION_ID >= 80100
static void xdebug_error_cb(int orig_type, zend_string *error_filename, const unsigned int error_lineno, zend_string *message)
{
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		int type                        = orig_type & E_ALL;
		char *error_type_str            = xdebug_error_type(type);

		xdebug_debugger_error_cb(error_filename, error_lineno, type, error_type_str, ZSTR_VAL(message));

		xdfree(error_type_str);
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		xdebug_develop_error_cb(orig_type, error_filename, error_lineno, message);
	} else {
		xdebug_old_error_cb(orig_type, error_filename, error_lineno, message);
	}
}
#elif PHP_VERSION_ID >= 80000
static void xdebug_error_cb(int orig_type, const char *error_filename, const unsigned int error_lineno, zend_string *message)
{
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		int type                        = orig_type & E_ALL;
		char *error_type_str            = xdebug_error_type(type);
		zend_string *tmp_error_filename = zend_string_init(error_filename, strlen(error_filename), 0);

		xdebug_debugger_error_cb(tmp_error_filename, error_lineno, type, error_type_str, ZSTR_VAL(message));

		zend_string_release(tmp_error_filename);
		xdfree(error_type_str);
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		xdebug_develop_error_cb(orig_type, error_filename, error_lineno, message);
	} else {
		xdebug_old_error_cb(orig_type, error_filename, error_lineno, message);
	}
}
#else
static void xdebug_error_cb(int orig_type, const char *error_filename, const unsigned int error_lineno, const char *format, va_list args)
{
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		int          type               = orig_type & E_ALL;
		char        *error_type_str     = xdebug_error_type(type);
		zend_string *tmp_error_filename = zend_string_init(error_filename, strlen(error_filename), 0);
		char        *buffer;
		va_list      new_args;

		va_copy(new_args, args);
		vspprintf(&buffer, PG(log_errors_max_len), format, new_args);
		va_end(new_args);

		xdebug_debugger_error_cb(tmp_error_filename, error_lineno, type, error_type_str, buffer);

		efree(buffer);
		zend_string_release(tmp_error_filename);
		xdfree(error_type_str);
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		xdebug_develop_error_cb(orig_type, error_filename, error_lineno, format, args);
	} else {
		xdebug_old_error_cb(orig_type, error_filename, error_lineno, format, args);
	}
}
#endif

void xdebug_base_use_original_error_cb(void)
{
	zend_error_cb = xdebug_old_error_cb;
}

void xdebug_base_use_xdebug_error_cb(void)
{
	zend_error_cb = xdebug_new_error_cb;
}

#if PHP_VERSION_ID >= 80000
static void xdebug_throw_exception_hook(zend_object *exception)
{
#else
static void xdebug_throw_exception_hook(zval *exception)
{
#endif
	zval *code, *message, *file, *line;
	zend_class_entry *exception_ce;
	char *code_str = NULL;
	zval dummy;

	if (!XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP) && !XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
		return;
	}

	if (!exception) {
		return;
	}

#if PHP_VERSION_ID >= 80000
	exception_ce = exception->ce;
#else
	exception_ce = Z_OBJCE_P(exception);
#endif

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

	if (Z_TYPE_P(message) != IS_STRING) {
		message = NULL;
	}

	convert_to_string_ex(file);
	convert_to_long_ex(line);

	if (XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
		xdebug_develop_throw_exception_hook(exception, file, line, code, code_str, message);
	}
	if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG)) {
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

	RETURN_FALSE;
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
