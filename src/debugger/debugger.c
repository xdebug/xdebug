/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2022 Derick Rethans                               |
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

#include "ext/standard/info.h"
#include "zend_exceptions.h"

#include "debugger_private.h"
#include "lib/log.h"
#include "lib/var.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

static size_t (*xdebug_orig_ub_write)(const char *string, size_t len);
static size_t xdebug_ub_write(const char *string, size_t length);

static void xdebug_line_list_dtor(xdebug_lines_list *line_list);

void xdebug_init_debugger_globals(xdebug_debugger_globals_t *xg)
{
	xg->breakpoint_count     = 0;
	xg->ide_key              = NULL;
	xg->stdout_mode          = 0;
	xg->no_exec              = 0;
	xg->context.program_name = NULL;
	xg->context.list.last_filename = NULL;
	xg->context.list.last_line = 0;
	xg->context.do_break     = 0;
	xg->context.pending_breakpoint = NULL;
	xg->context.do_step      = 0;
	xg->context.do_next      = 0;
	xg->context.do_finish    = 0;
	xg->context.do_connect_to_client = 0;

	xg->remote_connection_enabled  = 0;
	xg->remote_connection_pid      = 0;
	xg->breakpoints_allowed        = 0;
	xg->suppress_return_value_step = 0;

	/* Capturing output */
	if (sapi_module.ub_write != xdebug_ub_write) {
		xdebug_orig_ub_write = sapi_module.ub_write;
		sapi_module.ub_write = xdebug_ub_write;
	}

	/* Statistics and diagnostics */
	xg->context.connected_hostname = NULL;
	xg->context.connected_port = 0;
	xg->context.detached_message = NULL;
}

static char *xdebug_debugger_get_ide_key(void)
{
	char *ide_key;

	ide_key = XINI_DBG(ide_key_setting);
	if (ide_key && *ide_key) {
		return ide_key;
	}

	ide_key = getenv("DBGP_IDEKEY");
	if (ide_key && *ide_key) {
		return ide_key;
	}

	return NULL;
}

void xdebug_debugger_reset_ide_key(char *envval)
{
	if (XG_DBG(ide_key)) {
		xdfree(XG_DBG(ide_key));
	}
	XG_DBG(ide_key) = xdstrdup(envval);
}

int xdebug_debugger_bailout_if_no_exec_requested(void)
{
	/* We need to do this first before the executable clauses are called */
	if (XG_DBG(no_exec) == 1) {
		php_printf("DEBUG SESSION ENDED");
		return 1;
	}
	return 0;
}

static void register_compiled_variables(void)
{
	function_stack_entry *loop_fse;
	int                   i;

	if (!XG_BASE(stack)) {
		return;
	}

	loop_fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));

	for (i = 0; i < XDEBUG_VECTOR_COUNT(XG_BASE(stack)); i++, loop_fse--) {
		if (loop_fse->declared_vars) {
			continue;
		}

		if (loop_fse->user_defined == XDEBUG_BUILT_IN) {
			continue;
		}

		if (loop_fse->is_trampoline) {
			continue;
		}

		xdebug_lib_register_compiled_variables(loop_fse, loop_fse->op_array);
	}
}


void xdebug_debugger_set_program_name(zend_string *filename)
{
	if (!XG_DBG(context).program_name) {
		XG_DBG(context).program_name = zend_string_copy(filename);
	}
}

/* Remote debugger helper functions */
static int xdebug_handle_hit_value(xdebug_brk_info *brk_info)
{
	/* If this is a temporary breakpoint, disable the breakpoint */
	if (brk_info->temporary) {
		brk_info->disabled = 1;
	}

	/* Increase hit counter */
	brk_info->hit_count++;

	/* If the hit_value is 0, the condition check is disabled */
	if (!brk_info->hit_value) {
		return 1;
	}

	switch (brk_info->hit_condition) {
		case XDEBUG_HIT_GREATER_EQUAL:
			if (brk_info->hit_count >= brk_info->hit_value) {
				return 1;
			}
			break;
		case XDEBUG_HIT_EQUAL:
			if (brk_info->hit_count == brk_info->hit_value) {
				return 1;
			}
			break;
		case XDEBUG_HIT_MOD:
			if (brk_info->hit_count % brk_info->hit_value == 0) {
				return 1;
			}
			break;
		case XDEBUG_HIT_DISABLED:
			return 1;
			break;
	}
	return 0;
}

int xdebug_do_eval(char *eval_string, zval *ret_zval)
{
	volatile int       res = 1;
	zend_execute_data *original_execute_data = EG(current_execute_data);
	int                original_no_extensions = EG(no_extensions);
	zend_object       *original_exception = EG(exception);
	JMP_BUF           *original_bailout = EG(bailout);

	/* Remember error reporting level and track errors */
	XG_BASE(error_reporting_override) = EG(error_reporting);
	XG_BASE(error_reporting_overridden) = 1;
	EG(error_reporting) = 0;

	XG_DBG(context).inhibit_notifications = 1;
	XG_DBG(breakpoints_allowed) = 0;

	/* Reset exception in case we're triggered while being in xdebug_throw_exception_hook */
	EG(exception) = NULL;

	/* Do evaluation */
	zend_first_try {
		res = (zend_eval_string(eval_string, ret_zval, (char*) "xdebug://debug-eval") == SUCCESS);
	} zend_end_try();

	/* FIXME: Bubble up exception message to DBGp return packet */
	if (EG(exception)) {
		if (!res) {
			zend_clear_exception();
		}
		res = 0;
	}

	/* Clean up */
	EG(error_reporting) = XG_BASE(error_reporting_override);
	XG_BASE(error_reporting_overridden) = 0;
	XG_DBG(breakpoints_allowed) = 1;
	XG_DBG(suppress_return_value_step) = 0;
	XG_DBG(context).inhibit_notifications = 0;

	EG(current_execute_data) = original_execute_data;
	EG(no_extensions) = original_no_extensions;
	EG(exception) = original_exception;
	EG(bailout) = original_bailout;

	return res;
}

int next_condition_met(void)
{
	function_stack_entry *fse;
	int                   level = 0;

	if (!XG_DBG(context).do_next) {
		return 0;
	}

	/* Get latest stack level and function number */
	if (XG_BASE(stack) && XDEBUG_VECTOR_TAIL(XG_BASE(stack))) {
		fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));
		level = fse->level;
	}

	if (XG_DBG(context).next_level >= level) {
		return 1;
	}

	return 0;
}

int finish_condition_met(int break_at_return_scope)
{
	function_stack_entry *fse;
	int                   level = 0;
	int                   func_nr = 0;

	if (!XG_DBG(context).do_finish) {
		return 0;
	}

	/* Get latest stack level and function number */
	if (XG_BASE(stack) && XDEBUG_VECTOR_TAIL(XG_BASE(stack))) {
		fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));
		level = fse->level;
		func_nr = fse->function_nr;
	}

	if (!break_at_return_scope && level < XG_DBG(context).finish_level) {
		return 1;
	}

	if (break_at_return_scope && level <= XG_DBG(context).finish_level) {
		return 1;
	}

	if (
		(level == XG_DBG(context).finish_level) &&
		(func_nr > XG_DBG(context).finish_func_nr)
	) {
		return 1;
	}

	return 0;
}

void xdebug_debugger_statement_call(zend_string *filename, int lineno)
{
	xdebug_llist_element *le;
	xdebug_brk_info      *extra_brk_info;

	if (XG_DBG(context).do_connect_to_client) {
		XG_DBG(context).do_connect_to_client = 0;

		if (!xdebug_is_debug_connection_active()) {
			xdebug_debug_init_if_requested_on_xdebug_break();
			register_compiled_variables();
		}
	}

	if (xdebug_is_debug_connection_active()) {
		XG_DBG(suppress_return_value_step) = 0;

		if (XG_DBG(context).do_break) {
			xdebug_brk_info *brk_info = XG_DBG(context).pending_breakpoint;

			XG_DBG(context).do_break = 0;
			XG_DBG(context).pending_breakpoint = NULL;

			if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), filename, lineno, XDEBUG_BREAK, NULL, 0, NULL, brk_info, NULL)) {
				xdebug_mark_debug_connection_not_active();
				return;
			}
			return;
		}

		/* Check for "finish" */
		if (finish_condition_met(0)) {
			XG_DBG(context).do_finish = 0;

			if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), filename, lineno, XDEBUG_STEP, NULL, 0, NULL, NULL, NULL)) {
				xdebug_mark_debug_connection_not_active();
				return;
			}
			return;
		}

		/* Check for "next" */
		if (next_condition_met()) {
			XG_DBG(context).do_next = 0;

			if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), filename, lineno, XDEBUG_STEP, NULL, 0, NULL, NULL, NULL)) {
				xdebug_mark_debug_connection_not_active();
				return;
			}
			return;
		}

		/* Check for "step" */
		if (XG_DBG(context).do_step) {
			XG_DBG(context).do_step = 0;

			if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), filename, lineno, XDEBUG_STEP, NULL, 0, NULL, NULL, NULL)) {
				xdebug_mark_debug_connection_not_active();
				return;
			}
			return;
		}

		if (XG_DBG(context).line_breakpoints) {
			int   break_ok, res;
			zval  retval;

			for (le = XDEBUG_LLIST_HEAD(XG_DBG(context).line_breakpoints); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
				extra_brk_info = XDEBUG_LLIST_VALP(le);

				if (XG_DBG(context).handler->break_on_line(&(XG_DBG(context)), extra_brk_info, filename, lineno)) {
					break_ok = 1; /* Breaking is allowed by default */

					/* Check if we have a condition set for it */
					if (extra_brk_info->condition) {
						/* If there is a condition, we disable breaking by
						 * default and only enabled it when the code evaluates
						 * to TRUE */
						break_ok = 0;

						/* Remember error reporting level */
						res = xdebug_do_eval(extra_brk_info->condition, &retval);
						if (res) {
							break_ok = Z_TYPE(retval) == IS_TRUE;
							zval_dtor(&retval);
						}
					}
					if (break_ok && xdebug_handle_hit_value(extra_brk_info)) {
						if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), filename, lineno, XDEBUG_BREAK, NULL, 0, NULL, extra_brk_info, NULL)) {
							xdebug_mark_debug_connection_not_active();
							break;
						}
						return;
					}
				}
			}
		}
	}
}

void xdebug_debugger_throw_exception_hook(zend_object *exception, zval *file, zval *line, zval *code, char *code_str, zval *message)
{
	zend_class_entry *exception_ce = exception->ce;
	xdebug_brk_info *extra_brk_info;

	register_compiled_variables();

	/* Start JIT if requested and not yet enabled */
	xdebug_debug_init_if_requested_on_error();

	if (!xdebug_is_debug_connection_active() || !XG_DBG(breakpoints_allowed)) {
		return;
	}

	{
		int exception_breakpoint_found = 0;

		XG_DBG(suppress_return_value_step) = 1;

		/* Check if we have a wild card exception breakpoint */
		if (xdebug_hash_find(XG_DBG(context).exception_breakpoints, "*", 1, (void *) &extra_brk_info)) {
			exception_breakpoint_found = 1;
		} else {
			/* Check if we have a breakpoint on this exception or its parent classes */
			zend_class_entry *ce_ptr = exception_ce;

			/* Check if we have a breakpoint on this exception or its parent classes */
			do {
				if (xdebug_hash_find(XG_DBG(context).exception_breakpoints, (char *) STR_NAME_VAL(ce_ptr->name), STR_NAME_LEN(ce_ptr->name), (void *) &extra_brk_info)) {
					exception_breakpoint_found = 1;
				}
				ce_ptr = ce_ptr->parent;
			} while (!exception_breakpoint_found && ce_ptr);
		}
#if 0
		if (XG_DBG(context).resolved_breakpoints && exception_breakpoint_found) {
			XG_DBG(context).handler->resolve_breakpoints(&(XG_DBG(context)), extra_brk_info);
		}
#endif
		if (exception_breakpoint_found && xdebug_handle_hit_value(extra_brk_info)) {
			if (
				!XG_DBG(context).handler->remote_breakpoint(
					&(XG_DBG(context)), XG_BASE(stack),
					zend_get_executed_filename_ex(), zend_get_executed_lineno(),
					XDEBUG_BREAK,
					(char*) STR_NAME_VAL(exception_ce->name),
					code_str ? code_str : ((code && Z_TYPE_P(code) == IS_STRING) ? Z_STRVAL_P(code) : NULL),
					message ? Z_STRVAL_P(message) : "",
					extra_brk_info,
					NULL
				)
			) {
				xdebug_mark_debug_connection_not_active();
			}
		}
	}
}

void xdebug_debugger_error_cb(zend_string *error_filename, int error_lineno, int type, char *error_type_str, char *buffer)
{
	xdebug_brk_info *extra_brk_info = NULL;

	register_compiled_variables();

	/* Start JIT if requested and not yet enabled */
	xdebug_debug_init_if_requested_on_error();

	if (!xdebug_is_debug_connection_active() || !XG_DBG(breakpoints_allowed)) {
		return;
	}

	/* Send notification with warning/notice/error information */
	if (XG_DBG(context).send_notifications && !XG_DBG(context).inhibit_notifications) {
		if (!XG_DBG(context).handler->remote_notification(&(XG_DBG(context)), error_filename, error_lineno, type, error_type_str, buffer)) {
			xdebug_mark_debug_connection_not_active();
		}
	}

	/* Check for the pseudo exceptions to allow breakpoints on PHP error statuses */
	if (
		xdebug_hash_find(XG_DBG(context).exception_breakpoints, error_type_str, strlen(error_type_str), (void *) &extra_brk_info) ||
		xdebug_hash_find(XG_DBG(context).exception_breakpoints, "*", 1, (void *) &extra_brk_info)
	) {
		if (xdebug_handle_hit_value(extra_brk_info)) {
			char *type_str = xdebug_sprintf("%ld", type);

			if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), error_filename, error_lineno, XDEBUG_BREAK, error_type_str, type_str, buffer, extra_brk_info, NULL)) {
				xdebug_mark_debug_connection_not_active();
			}

			xdfree(type_str);
		}
	}
}

static int handle_breakpoints(function_stack_entry *fse, int breakpoint_type, zval *return_value)
{
	char            *tmp_name = NULL;
	size_t           tmp_len = 0;
	xdebug_brk_info *extra_brk_info = NULL;

	/* Function breakpoints */
	if (fse->function.type == XFUNC_NORMAL) {
		tmp_len = 2 + strlen(fse->function.function) + 1;
		tmp_name = xdmalloc(tmp_len);
		/* We intentionally do not use xdebug_sprintf because it can create a bottleneck in large
		 * codebases due to setlocale calls. We don't care about the locale here. */
		snprintf(
			tmp_name, tmp_len,
			"%c/%s",
			(breakpoint_type & XDEBUG_BREAKPOINT_TYPE_CALL) ? 'C' : 'R',
			fse->function.function
		);
	}
	/* class->function breakpoints */
	else if (fse->function.type == XFUNC_MEMBER || fse->function.type == XFUNC_STATIC_MEMBER) {
		/* Using strlen(ZSTR_VAL(...)) here to cut of the string at the first \0, which is needed
		 * for anonymous classes, in combination with the snprintf() below */
		tmp_len = 2 + strlen(ZSTR_VAL(fse->function.object_class)) + 2 + strlen(fse->function.function) + 1;
		tmp_name = xdmalloc(tmp_len);
		/* We intentionally do not use xdebug_sprintf because it can create a bottleneck in large
		 * codebases due to setlocale calls. We don't care about the locale here. */
		snprintf(
			tmp_name, tmp_len,
			"%c/%s::%s",
			(breakpoint_type & XDEBUG_BREAKPOINT_TYPE_CALL) ? 'C' : 'R',
			ZSTR_VAL(fse->function.object_class), fse->function.function
		);
	}
	/* Unknown */
	else {
		return 1;
	}

	if (xdebug_hash_find(XG_DBG(context).function_breakpoints, tmp_name, tmp_len - 1, (void *) &extra_brk_info)) {
		/* Yup, breakpoint found, call handler if the breakpoint is not
		 * disabled AND handle_hit_value is happy */
		if (!extra_brk_info->disabled && (extra_brk_info->function_break_type == (breakpoint_type & XDEBUG_BREAKPOINT_TYPES_MASK))) {
			if (xdebug_handle_hit_value(extra_brk_info)) {
				if (fse->user_defined == XDEBUG_BUILT_IN || (breakpoint_type & XDEBUG_BREAKPOINT_TYPE_RETURN)) {
					if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), fse->filename, fse->lineno, XDEBUG_BREAK, NULL, 0, NULL, extra_brk_info, return_value)) {
						xdfree(tmp_name);
						return 0;
					}
				} else {
					XG_DBG(context).do_break = 1;
					XG_DBG(context).pending_breakpoint = extra_brk_info;
				}
			}
		}
	}
	xdfree(tmp_name);

	if (
		(XG_DBG(context).breakpoint_include_return_value) &&
		(breakpoint_type & XDEBUG_BREAKPOINT_TYPE_RETURN) &&
		!(XG_DBG(suppress_return_value_step)) &&
		return_value
	) {
		if (XG_DBG(context).do_step) {
			XG_DBG(context).do_step = 0;
		} else if (finish_condition_met(1)) {
			XG_DBG(context).do_finish = 0;
		} else {
			return 1;
		}

		if (!XG_DBG(context).handler->remote_breakpoint(&(XG_DBG(context)), XG_BASE(stack), fse->filename, fse->lineno, XDEBUG_BREAK, NULL, 0, NULL, extra_brk_info, return_value)) {
			return 0;
		}
	}

	return 1;
}

void xdebug_debugger_handle_breakpoints(function_stack_entry *fse, int breakpoint_type, zval *return_value)
{
	if (xdebug_is_debug_connection_active() && XG_DBG(breakpoints_allowed)) {
		if (!handle_breakpoints(fse, breakpoint_type, return_value)) {
			xdebug_mark_debug_connection_not_active();
		}
	}
}

static size_t xdebug_ub_write(const char *string, size_t length)
{
	if (xdebug_is_debug_connection_active()) {
		if (-1 == XG_DBG(context).handler->remote_stream_output(string, length)) {
			return 0;
		}
	}
	return xdebug_orig_ub_write(string, length);
}

static void xdebug_hook_output_handlers()
{
	/* Override output handler for capturing output */
	if (xdebug_orig_ub_write == NULL) {
		xdebug_orig_ub_write = sapi_module.ub_write;
		sapi_module.ub_write = xdebug_ub_write;
	}
}

static void xdebug_unhook_output_handlers()
{
	/* Restore original output handler */
	sapi_module.ub_write = xdebug_orig_ub_write;
	xdebug_orig_ub_write = NULL;
}

void xdebug_debugger_zend_startup(void)
{
	/* Hook output handlers (header and output writer) */
	xdebug_hook_output_handlers();
}

void xdebug_debugger_zend_shutdown(void)
{
	/* Remove our hooks to output handlers (header and output writer) */
	xdebug_unhook_output_handlers();
}

void xdebug_debugger_minit(void)
{
	XG_DBG(breakpoint_count) = 0;
}

void xdebug_debugger_minfo(void)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Debugger", "enabled");
	php_info_print_table_row(2, "IDE Key", XG_DBG(ide_key));
	php_info_print_table_end();
}

void xdebug_debugger_rinit(void)
{
	char *idekey;

	xdebug_disable_opcache_optimizer();

	/* Get the ide key for this session */
	XG_DBG(ide_key) = NULL;
	idekey = xdebug_debugger_get_ide_key();
	if (idekey && *idekey) {
		if (XG_DBG(ide_key)) {
			xdfree(XG_DBG(ide_key));
		}
		XG_DBG(ide_key) = xdstrdup(idekey);
	}

	XG_DBG(no_exec) = 0;
	xdebug_lib_set_active_symbol_table(NULL);

	/* Check if we have this special get variable that stops a debugging
	 * request without executing any code */
	{
		zend_string *stop_no_exec = zend_string_init(ZEND_STRL("XDEBUG_SESSION_STOP_NO_EXEC"), 0);
		if (
			(
				(
					zend_hash_find(Z_ARR(PG(http_globals)[TRACK_VARS_GET]), stop_no_exec) != NULL
				) || (
					zend_hash_find(Z_ARR(PG(http_globals)[TRACK_VARS_POST]), stop_no_exec) != NULL
				)
			)
			&& !SG(headers_sent)
		) {
			xdebug_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION") - 1, (char*) "", 0, 0, "/", 1, NULL, 0, 0, 1, 0);
			XG_DBG(no_exec) = 1;
		}
		zend_string_release(stop_no_exec);
	}

	xdebug_mark_debug_connection_not_active();

	XG_DBG(breakpoints_allowed) = 1;
	XG_DBG(suppress_return_value_step) = 0;
	XG_DBG(detached) = 0;
	XG_DBG(breakable_lines_map) = xdebug_hash_alloc(2048, (xdebug_hash_dtor_t) xdebug_line_list_dtor);
	XG_DBG(function_count) = 0;
	XG_DBG(class_count) = 0;

	/* Initialize some debugger context properties */
	XG_DBG(context).program_name   = NULL;
	XG_DBG(context).list.last_filename = NULL;
	XG_DBG(context).list.last_line = 0;
	XG_DBG(context).do_break       = 0;
	XG_DBG(context).pending_breakpoint = NULL;
	XG_DBG(context).do_step        = 0;
	XG_DBG(context).do_next        = 0;
	XG_DBG(context).do_finish      = 0;
	XG_DBG(context).do_connect_to_client = 0;

	/* Statistics and diagnostics */
	XG_DBG(context).connected_hostname = NULL;
	XG_DBG(context).connected_port = 0;
	XG_DBG(context).detached_message = NULL;
}

void xdebug_debugger_post_deactivate(void)
{
	if (XG_DBG(remote_connection_enabled)) {
		XG_DBG(context).handler->remote_deinit(&(XG_DBG(context)));
		xdebug_close_socket(XG_DBG(context).socket);
	}
	if (XG_DBG(context).program_name) {
		zend_string_release(XG_DBG(context).program_name);
	}

	if (XG_DBG(ide_key)) {
		xdfree(XG_DBG(ide_key));
		XG_DBG(ide_key) = NULL;
	}

	if (XG_DBG(context.list.last_filename)) {
		zend_string_release(XG_DBG(context).list.last_filename);
		XG_DBG(context).list.last_filename = NULL;
	}

	xdebug_hash_destroy(XG_DBG(breakable_lines_map));
	XG_DBG(breakable_lines_map) = NULL;

	if (XG_DBG(context).connected_hostname) {
		xdfree(XG_DBG(context).connected_hostname);
		XG_DBG(context).connected_hostname = NULL;
	}

	if (XG_DBG(context).detached_message) {
		xdfree(XG_DBG(context).detached_message);
		XG_DBG(context).detached_message = NULL;
	}
}

xdebug_set *xdebug_debugger_get_breakable_lines_from_oparray(zend_op_array *opa)
{
	int         i;
	xdebug_set *tmp;

	tmp = xdebug_set_create(opa->line_end);

	for (i = 0; i < opa->last; i++ ) {
		if (opa->opcodes[i].opcode == ZEND_EXT_STMT ) {
			xdebug_set_add(tmp, opa->opcodes[i].lineno);
		}
	}

	return tmp;
}


/* {{{ function/lines map collection helpers */
static void xdebug_function_lines_map_dtor(xdebug_function_lines_map_item *lines_map)
{
	xdebug_set_free(lines_map->lines_breakable);
	xdfree(lines_map);
}

static void xdebug_line_list_dtor(xdebug_lines_list *line_list)
{
	size_t i;

	for (i  = 0; i < line_list->count; i++) {
		xdebug_function_lines_map_dtor(line_list->functions[i]);
	}
	xdfree(line_list->functions);
	xdfree(line_list);
}

static xdebug_lines_list *get_file_function_line_list(zend_string *filename)
{
	xdebug_lines_list *lines_list;

	if (xdebug_hash_find(XG_DBG(breakable_lines_map), ZSTR_VAL(filename), ZSTR_LEN(filename), (void *) &lines_list)) {
		return lines_list;
	}

	lines_list = xdmalloc(sizeof(xdebug_lines_list));
	lines_list->count = 0;
	lines_list->size  = 0;
	lines_list->functions = NULL;

	xdebug_hash_add(XG_DBG(breakable_lines_map), ZSTR_VAL(filename), ZSTR_LEN(filename), (void *) lines_list);

	return lines_list;
}

static void add_function_to_lines_list(xdebug_lines_list *lines_list, zend_op_array *opa)
{
	xdebug_function_lines_map_item *map_item = xdmalloc(sizeof(xdebug_function_lines_map_item));

	map_item->line_start = opa->line_start;
	map_item->line_end   = opa->line_end;
	map_item->line_span  = opa->line_end - opa->line_start;
	map_item->lines_breakable = xdebug_debugger_get_breakable_lines_from_oparray(opa);

	if (lines_list->count >= lines_list->size) {
		lines_list->size = lines_list->size == 0 ? 16 : lines_list->size * 2;
		lines_list->functions = xdrealloc(lines_list->functions, sizeof(xdebug_function_lines_map_item *) * lines_list->size);
	}
	lines_list->functions[lines_list->count] = map_item;
	lines_list->count++;

#if PHP_VERSION_ID >= 80100
	if (opa->num_dynamic_func_defs) {
		uint32_t i;

		for (i = 0; i < opa->num_dynamic_func_defs; i++) {
			add_function_to_lines_list(lines_list, opa->dynamic_func_defs[i]);
		}
	}
#endif
}
/* }}} */

static void resolve_breakpoints_for_function(xdebug_lines_list *lines_list, zend_op_array *opa)
{
	add_function_to_lines_list(lines_list, opa);
}

static void resolve_breakpoints_for_class(xdebug_lines_list *file_function_lines_list, zend_class_entry *ce, zend_string *filename)
{
	zend_op_array    *function_op_array;

	ZEND_HASH_FOREACH_PTR(&ce->function_table, function_op_array) {
		if (!ZEND_USER_CODE(function_op_array->type)) {
			continue;
		}
		/* Only resolve if the file names are the same. This is needed in case
		 * of inheritance or traits where op arrays from other files might get introduced */
		if (ZSTR_LEN(filename) != ZSTR_LEN(function_op_array->filename)) {
			continue;
		}
		if (strcmp(ZSTR_VAL(filename), ZSTR_VAL(function_op_array->filename)) != 0) {
			continue;
		}
		resolve_breakpoints_for_function(file_function_lines_list, function_op_array);
	} ZEND_HASH_FOREACH_END();
}

void xdebug_debugger_compile_file(zend_op_array *op_array)
{
	zend_op_array    *function_op_array;
	zend_class_entry *class_entry;
	xdebug_lines_list *file_function_lines_list;

	RETURN_IF_MODE_IS_NOT(XDEBUG_MODE_STEP_DEBUG);

	/* The breakable_lines_map can not be set if another extension compiles
	 * scripts during RINIT */
	if (!XG_DBG(breakable_lines_map)) {
		return;
	}

	file_function_lines_list = get_file_function_line_list(op_array->filename);

	ZEND_HASH_REVERSE_FOREACH_PTR(CG(function_table), function_op_array) {
		if (_idx == XG_DBG(function_count)) {
			break;
		}
		if (!ZEND_USER_CODE(function_op_array->type)) {
			continue;
		}
		resolve_breakpoints_for_function(file_function_lines_list, function_op_array);
	} ZEND_HASH_FOREACH_END();
	XG_DBG(function_count) = CG(function_table)->nNumUsed;

	ZEND_HASH_REVERSE_FOREACH_PTR(CG(class_table), class_entry) {
		if (_idx == XG_DBG(class_count)) {
			break;
		}
		if (class_entry->type == ZEND_INTERNAL_CLASS) {
			continue;
		}
		resolve_breakpoints_for_class(file_function_lines_list, class_entry, op_array->filename);
	} ZEND_HASH_FOREACH_END();
	XG_DBG(class_count) = CG(class_table)->nNumUsed;

	add_function_to_lines_list(file_function_lines_list, op_array);

	if (!xdebug_is_debug_connection_active()) {
		return;
	}

	XG_DBG(context).handler->resolve_breakpoints(
		&(XG_DBG(context)),
		op_array->filename
	);
}

static void resolve_breakpoints_for_eval(int eval_id, zend_op_array *opa)
{
	xdebug_lines_list *lines_list;
	char *eval_filename = xdebug_sprintf("dbgp://%d", eval_id);
	zend_string *eval_string = zend_string_init(eval_filename, strlen(eval_filename), 0);

	lines_list = get_file_function_line_list(eval_string);
	add_function_to_lines_list(lines_list, opa);

	resolve_breakpoints_for_function(lines_list, opa);

	if (!xdebug_is_debug_connection_active()) {
		zend_string_release(eval_string);
		xdfree(eval_filename);
		return;
	}

	XG_DBG(context).handler->resolve_breakpoints(
		&(XG_DBG(context)),
		eval_string
	);

	zend_string_release(eval_string);
	xdfree(eval_filename);
}

void xdebug_debugger_register_eval(function_stack_entry *fse)
{
	if (xdebug_is_debug_connection_active() && XG_DBG(context).handler->register_eval_id) {
		int eval_id = XG_DBG(context).handler->register_eval_id(&(XG_DBG(context)), fse);

		resolve_breakpoints_for_eval(eval_id, fse->op_array);
	}
}

void xdebug_debugger_restart_if_pid_changed()
{
	zend_ulong pid;

	if (!xdebug_is_debug_connection_active()) {
		return;
	}

	pid = xdebug_get_pid();

	/* Start debugger if previously a connection was established and this
	 * process no longer has the same PID */
	if (XG_DBG(remote_connection_pid) != pid) {
		xdebug_restart_debugger();
	}
}

PHP_FUNCTION(xdebug_break)
{
	RETURN_FALSE_IF_MODE_IS_NOT(XDEBUG_MODE_STEP_DEBUG);

	xdebug_debug_init_if_requested_on_xdebug_break();

	if (!xdebug_is_debug_connection_active()) {
		RETURN_FALSE;
	}

	register_compiled_variables();

	XG_DBG(context).do_break = 1;
	XG_DBG(context).pending_breakpoint = NULL;

	RETURN_TRUE;
}

PHP_FUNCTION(xdebug_connect_to_client)
{
	RETURN_FALSE_IF_MODE_IS_NOT(XDEBUG_MODE_STEP_DEBUG);

	XG_DBG(context).do_connect_to_client = 1;

	RETURN_TRUE;
}

PHP_FUNCTION(xdebug_is_debugger_active)
{
	RETURN_BOOL(xdebug_is_debug_connection_active());
}

PHP_FUNCTION(xdebug_notify)
{
	function_stack_entry *fse;
	zval *data;

	RETURN_FALSE_IF_MODE_IS_NOT(XDEBUG_MODE_STEP_DEBUG);

	if (!xdebug_is_debug_connection_active()) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &data) == FAILURE) {
		return;
	}

	fse = xdebug_get_stack_frame(0);

	XG_DBG(context).handler->user_notification(
		&(XG_DBG(context)),
		fse->filename, fse->lineno,
		data
	);

	RETURN_TRUE;
}
