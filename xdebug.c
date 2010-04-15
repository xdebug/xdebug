/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2010 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   |           Ilia Alshanetsky <ilia@prohost.org>                        |
   |           Harald Radi <harald.radi@nme.at>                           |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "main/php_version.h"
#include "xdebug_compat.h"

#if HAVE_XDEBUG

#ifndef PHP_WIN32
#include <sys/time.h>
#include <unistd.h>
#else
#include "win32/time.h"
#include <process.h>
#endif

#include "TSRM.h"
#include "SAPI.h"
#include "main/php_ini.h"
#include "ext/standard/head.h"
#include "ext/standard/html.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "php_globals.h"
#include "ext/standard/php_var.h"


#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_code_coverage.h"
#include "xdebug_com.h"
#include "xdebug_llist.h"
#include "xdebug_mm.h"
#include "xdebug_var.h"
#include "xdebug_profiler.h"
#include "xdebug_stack.h"
#include "xdebug_superglobals.h"
#include "xdebug_tracing.h"
#include "usefulstuff.h"

/* execution redirection functions */
zend_op_array* (*old_compile_file)(zend_file_handle* file_handle, int type TSRMLS_DC);
zend_op_array* xdebug_compile_file(zend_file_handle*, int TSRMLS_DC);

void (*xdebug_old_execute)(zend_op_array *op_array TSRMLS_DC);
void xdebug_execute(zend_op_array *op_array TSRMLS_DC);

void (*xdebug_old_execute_internal)(zend_execute_data *current_execute_data, int return_value_used TSRMLS_DC);
void xdebug_execute_internal(zend_execute_data *current_execute_data, int return_value_used TSRMLS_DC);

/* error callback repalcement functions */
void (*old_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);
void (*new_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);
void xdebug_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);

static int xdebug_header_handler(sapi_header_struct *h XG_SAPI_HEADER_OP_DC, sapi_headers_struct *s TSRMLS_DC);

static void xdebug_throw_exception_hook(zval *exception TSRMLS_DC);
int xdebug_exit_handler(ZEND_OPCODE_HANDLER_ARGS);

int zend_xdebug_initialised = 0;
int zend_xdebug_global_offset = -1;

int (*xdebug_orig_header_handler)(sapi_header_struct *h XG_SAPI_HEADER_OP_DC, sapi_headers_struct *s TSRMLS_DC);

zend_function_entry xdebug_functions[] = {
	PHP_FE(xdebug_get_stack_depth,       NULL)
	PHP_FE(xdebug_get_function_stack,    NULL)
	PHP_FE(xdebug_get_formatted_function_stack,    NULL)
	PHP_FE(xdebug_print_function_stack,  NULL)
	PHP_FE(xdebug_get_declared_vars,     NULL)
	PHP_FE(xdebug_call_class,            NULL)
	PHP_FE(xdebug_call_function,         NULL)
	PHP_FE(xdebug_call_file,             NULL)
	PHP_FE(xdebug_call_line,             NULL)

	PHP_FE(xdebug_var_dump,              NULL)
	PHP_FE(xdebug_debug_zval,            NULL)
	PHP_FE(xdebug_debug_zval_stdout,     NULL)

	PHP_FE(xdebug_enable,                NULL)
	PHP_FE(xdebug_disable,               NULL)
	PHP_FE(xdebug_is_enabled,            NULL)
	PHP_FE(xdebug_break,                 NULL)

	PHP_FE(xdebug_start_trace,           NULL)
	PHP_FE(xdebug_stop_trace,            NULL)
	PHP_FE(xdebug_get_tracefile_name,    NULL)

	PHP_FE(xdebug_get_profiler_filename, NULL)
	PHP_FE(xdebug_dump_aggr_profiling_data, NULL)
	PHP_FE(xdebug_clear_aggr_profiling_data, NULL)

#if HAVE_PHP_MEMORY_USAGE
	PHP_FE(xdebug_memory_usage,          NULL)
	PHP_FE(xdebug_peak_memory_usage,     NULL)
#endif
	PHP_FE(xdebug_time_index,            NULL)

	PHP_FE(xdebug_start_error_collection, NULL)
	PHP_FE(xdebug_stop_error_collection, NULL)
	PHP_FE(xdebug_get_collected_errors,  NULL)

	PHP_FE(xdebug_start_code_coverage,   NULL)
	PHP_FE(xdebug_stop_code_coverage,    NULL)
	PHP_FE(xdebug_get_code_coverage,     NULL)
	PHP_FE(xdebug_get_function_count,    NULL)

	PHP_FE(xdebug_dump_superglobals,     NULL)
	PHP_FE(xdebug_get_headers,           NULL)
	{NULL, NULL, NULL}
};

zend_module_entry xdebug_module_entry = {
	STANDARD_MODULE_HEADER,
	"xdebug",
	xdebug_functions,
	PHP_MINIT(xdebug),
	PHP_MSHUTDOWN(xdebug),
	PHP_RINIT(xdebug),
	NULL,
	PHP_MINFO(xdebug),
	XDEBUG_VERSION,
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 2) || PHP_MAJOR_VERSION >= 6
	NO_MODULE_GLOBALS,
#endif
	ZEND_MODULE_POST_ZEND_DEACTIVATE_N(xdebug),
	STANDARD_MODULE_PROPERTIES_EX
};


ZEND_DECLARE_MODULE_GLOBALS(xdebug)

#if COMPILE_DL_XDEBUG
ZEND_GET_MODULE(xdebug)
#endif

static PHP_INI_MH(OnUpdateServer)
{
	DUMP_TOK(server);
}

static PHP_INI_MH(OnUpdateGet)
{
	DUMP_TOK(get);
}

static PHP_INI_MH(OnUpdatePost)
{
	DUMP_TOK(post);
}

static PHP_INI_MH(OnUpdateCookie)
{
	DUMP_TOK(cookie);
}

static PHP_INI_MH(OnUpdateFiles)
{
	DUMP_TOK(files);
}

static PHP_INI_MH(OnUpdateEnv)
{
	DUMP_TOK(env);
}

static PHP_INI_MH(OnUpdateRequest)
{
	DUMP_TOK(request);
}

static PHP_INI_MH(OnUpdateSession)
{
	DUMP_TOK(session);
}

static PHP_INI_MH(OnUpdateIDEKey)
{
	if (XG(ide_key)) {
		xdfree(XG(ide_key));
	}
	if (!new_value) {
		XG(ide_key) = NULL;
	} else {
		XG(ide_key) = xdstrdup(new_value);
	}
	return SUCCESS;
}

static PHP_INI_MH(OnUpdateDebugMode)
{
	if (!new_value) {
		XG(remote_mode) = XDEBUG_NONE;

	} else if (strcmp(new_value, "jit") == 0) {
		XG(remote_mode) = XDEBUG_JIT;

	} else if (strcmp(new_value, "req") == 0) {
		XG(remote_mode) = XDEBUG_REQ;

	} else {
		XG(remote_mode) = XDEBUG_NONE;
	}
	return SUCCESS;
}

#ifdef P_tmpdir
# define XDEBUG_TEMP_DIR P_tmpdir
#else
# define XDEBUG_TEMP_DIR "/tmp"
#endif

PHP_INI_BEGIN()
	/* Debugger settings */
	STD_PHP_INI_BOOLEAN("xdebug.auto_trace",      "0",                  PHP_INI_ALL,    OnUpdateBool,   auto_trace,        zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_output_dir",  XDEBUG_TEMP_DIR,      PHP_INI_ALL,    OnUpdateString, trace_output_dir,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_output_name", "trace.%c",           PHP_INI_ALL,    OnUpdateString, trace_output_name, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_format",      "0",                  PHP_INI_ALL,    OnUpdateLong,   trace_format,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_options",     "0",                  PHP_INI_ALL,    OnUpdateLong,   trace_options,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_includes","1",                  PHP_INI_ALL,    OnUpdateBool,   collect_includes,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.collect_params",  "0",                    PHP_INI_ALL,    OnUpdateLong,   collect_params,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_return",  "0",                  PHP_INI_ALL,    OnUpdateBool,   collect_return,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_vars",    "0",                  PHP_INI_ALL,    OnUpdateBool,   collect_vars,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_assignments", "0",              PHP_INI_ALL,    OnUpdateBool,   collect_assignments, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.default_enable",  "1",                  PHP_INI_ALL,    OnUpdateBool,   default_enable,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.extended_info",   "1",                  PHP_INI_SYSTEM, OnUpdateBool,   extended_info,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.file_link_format",  "",                   PHP_INI_ALL,    OnUpdateString, file_link_format,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.manual_url",        "http://www.php.net", PHP_INI_ALL,    OnUpdateString, manual_url,        zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.max_nesting_level", "100",                PHP_INI_ALL,    OnUpdateLong,   max_nesting_level, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.overload_var_dump", "1", PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   overload_var_dump, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_exception_trace",  "0",            PHP_INI_ALL,    OnUpdateBool,   show_ex_trace,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_local_vars", "0",                  PHP_INI_ALL,    OnUpdateBool,   show_local_vars,   zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.show_mem_delta",  "0",                  PHP_INI_ALL,    OnUpdateBool,   show_mem_delta,    zend_xdebug_globals, xdebug_globals)

	/* Dump superglobals settings */
	PHP_INI_ENTRY("xdebug.dump.COOKIE",           NULL,                 PHP_INI_ALL,    OnUpdateCookie)
	PHP_INI_ENTRY("xdebug.dump.ENV",              NULL,                 PHP_INI_ALL,    OnUpdateEnv)
	PHP_INI_ENTRY("xdebug.dump.FILES",            NULL,                 PHP_INI_ALL,    OnUpdateFiles)
	PHP_INI_ENTRY("xdebug.dump.GET",              NULL,                 PHP_INI_ALL,    OnUpdateGet)
	PHP_INI_ENTRY("xdebug.dump.POST",             NULL,                 PHP_INI_ALL,    OnUpdatePost)
	PHP_INI_ENTRY("xdebug.dump.REQUEST",          NULL,                 PHP_INI_ALL,    OnUpdateRequest)
	PHP_INI_ENTRY("xdebug.dump.SERVER",           NULL,                 PHP_INI_ALL,    OnUpdateServer)
	PHP_INI_ENTRY("xdebug.dump.SESSION",          NULL,                 PHP_INI_ALL,    OnUpdateSession)
	STD_PHP_INI_BOOLEAN("xdebug.dump_globals",    "1",                  PHP_INI_ALL,    OnUpdateBool,   dump_globals,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.dump_once",       "1",                  PHP_INI_ALL,    OnUpdateBool,   dump_once,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.dump_undefined",  "0",                  PHP_INI_ALL,    OnUpdateBool,   dump_undefined,    zend_xdebug_globals, xdebug_globals)

	/* Profiler settings */
	STD_PHP_INI_BOOLEAN("xdebug.profiler_enable",         "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler_enable,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.profiler_output_dir",       XDEBUG_TEMP_DIR,      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, profiler_output_dir,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.profiler_output_name",      "cachegrind.out.%p",  PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, profiler_output_name,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_enable_trigger", "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler_enable_trigger, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_append",         "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler_append,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_aggregate",      "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler_aggregate,      zend_xdebug_globals, xdebug_globals)

	/* Remote debugger settings */
	STD_PHP_INI_BOOLEAN("xdebug.remote_enable",   "0",   PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   remote_enable,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_handler",    "dbgp",               PHP_INI_ALL,    OnUpdateString, remote_handler,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_host",       "localhost",          PHP_INI_ALL,    OnUpdateString, remote_host,       zend_xdebug_globals, xdebug_globals)
	PHP_INI_ENTRY("xdebug.remote_mode",           "req",                PHP_INI_ALL,    OnUpdateDebugMode)
	STD_PHP_INI_ENTRY("xdebug.remote_port",       "9000",               PHP_INI_ALL,    OnUpdateLong,   remote_port,       zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.remote_autostart","0",                  PHP_INI_ALL,    OnUpdateBool,   remote_autostart,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.remote_connect_back","0",               PHP_INI_SYSTEM, OnUpdateBool,   remote_connect_back,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_log",        "",                   PHP_INI_ALL,    OnUpdateString, remote_log,        zend_xdebug_globals, xdebug_globals)
	PHP_INI_ENTRY("xdebug.idekey",                "",                   PHP_INI_ALL,    OnUpdateIDEKey)
	STD_PHP_INI_ENTRY("xdebug.remote_cookie_expire_time", "3600",       PHP_INI_ALL,    OnUpdateLong,   remote_cookie_expire_time, zend_xdebug_globals, xdebug_globals)

	/* Variable display settings */
	STD_PHP_INI_ENTRY("xdebug.var_display_max_children", "128",         PHP_INI_ALL,    OnUpdateLong,   display_max_children, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.var_display_max_data",     "512",         PHP_INI_ALL,    OnUpdateLong,   display_max_data,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.var_display_max_depth",    "3",           PHP_INI_ALL,    OnUpdateLong,   display_max_depth,    zend_xdebug_globals, xdebug_globals)

	/* Scream support */
	STD_PHP_INI_BOOLEAN("xdebug.scream",                 "0",           PHP_INI_ALL,    OnUpdateBool,   do_scream,            zend_xdebug_globals, xdebug_globals)
PHP_INI_END()

static void php_xdebug_init_globals (zend_xdebug_globals *xg TSRMLS_DC)
{
	zend_extension dummy_ext;

	xg->stack                = NULL;
	xg->level                = 0;
	xg->do_trace             = 0;
	xg->trace_file           = NULL;
	xg->do_code_coverage     = 0;
	xg->breakpoint_count     = 0;
	xg->ide_key              = NULL;

	xdebug_llist_init(&xg->server, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->get, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->post, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->cookie, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->files, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->env, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->request, xdebug_superglobals_dump_dtor);
	xdebug_llist_init(&xg->session, xdebug_superglobals_dump_dtor);

	/* Get reserved offset */
	xg->reserved_offset = zend_xdebug_global_offset;

	/* Override header generation in SAPI */
	if (sapi_module.header_handler != xdebug_header_handler) {
		xdebug_orig_header_handler = sapi_module.header_handler;
		sapi_module.header_handler = xdebug_header_handler;
	}
	xg->headers = NULL;
}

static void php_xdebug_shutdown_globals (zend_xdebug_globals *xg TSRMLS_DC)
{
	xdebug_llist_empty(&xg->server, NULL);
	xdebug_llist_empty(&xg->get, NULL);
	xdebug_llist_empty(&xg->post, NULL);
	xdebug_llist_empty(&xg->cookie, NULL);
	xdebug_llist_empty(&xg->files, NULL);
	xdebug_llist_empty(&xg->env, NULL);
	xdebug_llist_empty(&xg->request, NULL);
	xdebug_llist_empty(&xg->session, NULL);
}

void xdebug_env_key()
{
	char *ide_key = getenv("DBGP_IDEKEY");
	if (!ide_key || !*ide_key) {
		ide_key = getenv("USER");
		if (!ide_key || !*ide_key) {
			ide_key = getenv("USERNAME");
		}
	}
	if (ide_key && *ide_key) {
		zend_alter_ini_entry("xdebug.idekey", sizeof("xdebug.idekey"), ide_key, strlen(ide_key), PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
	}
}

void xdebug_env_config()
{
	char       *config = getenv("XDEBUG_CONFIG");
	xdebug_arg *parts;
	int			i;
	/*
		XDEBUG_CONFIG format:
		XDEBUG_CONFIG=var=val var=val
	*/
	xdebug_env_key();
	if (!config) {
		return;
	}

	parts = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));
	xdebug_arg_init(parts);
	xdebug_explode(" ", config, parts, -1);

	for (i = 0; i < parts->c; ++i) {
		char *name = NULL;
		char *envvar = parts->args[i];
		char *envval = NULL;
		char *eq = strchr(envvar, '=');
		if (!eq || !*eq) {
			continue;
		}
		*eq = 0;
		envval = eq + 1;
		if (!*envval) {
			continue;
		}

		if (strcasecmp(envvar, "remote_enable") == 0) {
			name = "xdebug.remote_enable";
		} else
		if (strcasecmp(envvar, "remote_port") == 0) {
			name = "xdebug.remote_port";
		} else
		if (strcasecmp(envvar, "remote_host") == 0) {
			name = "xdebug.remote_host";
		} else
		if (strcasecmp(envvar, "remote_handler") == 0) {
			name = "xdebug.remote_handler";
		} else
		if (strcasecmp(envvar, "remote_mode") == 0) {
			name = "xdebug.remote_mode";
		} else
		if (strcasecmp(envvar, "idekey") == 0) {
			name = "xdebug.idekey";
		} else
		if (strcasecmp(envvar, "profiler_enable") == 0) {
			name = "xdebug.profiler_enable";
		} else
		if (strcasecmp(envvar, "profiler_output_dir") == 0) {
			name = "xdebug.profiler_output_dir";
		} else
		if (strcasecmp(envvar, "profiler_output_name") == 0) {
			name = "xdebug.profiler_output_name";
		} else
		if (strcasecmp(envvar, "profiler_enable_trigger") == 0) {
			name = "xdebug.profiler_enable_trigger";
		} else
		if (strcasecmp(envvar, "remote_log") == 0) {
			name = "xdebug.remote_log";
		} else
		if (strcasecmp(envvar, "remote_cookie_expire_time") == 0) {
			name = "xdebug.remote_cookie_expire_time";
		}

		if (name) {
			zend_alter_ini_entry(name, strlen(name) + 1, envval, strlen(envval), PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
		}
	}

	xdebug_arg_dtor(parts);
}

static int xdebug_silence_handler(ZEND_OPCODE_HANDLER_ARGS)
{
    if (XG(do_scream)) {
        execute_data->opline++;
        return ZEND_USER_OPCODE_CONTINUE;
    }
    return ZEND_USER_OPCODE_DISPATCH;
}

static int xdebug_include_or_eval_handler(ZEND_OPCODE_HANDLER_ARGS)
{
	zend_op *opline = execute_data->opline;

	if (Z_LVAL(opline->op2.u.constant) == ZEND_EVAL) {
		zval *inc_filename;
		zval tmp_inc_filename;
		int  is_var;
		int  tmp_len;

		inc_filename = xdebug_get_zval(execute_data, &opline->op1, execute_data->Ts, &is_var);
		
		/* If there is no inc_filename, we're just bailing out instead */
		if (!inc_filename) {
			return ZEND_USER_OPCODE_DISPATCH;
		}

		if (inc_filename->type != IS_STRING) {
			tmp_inc_filename = *inc_filename;
			zval_copy_ctor(&tmp_inc_filename);
			convert_to_string(&tmp_inc_filename);
			inc_filename = &tmp_inc_filename;
		}

		/* Now let's store this info */
		if (XG(last_eval_statement)) {
			efree(XG(last_eval_statement));
		}
		XG(last_eval_statement) = php_addcslashes(Z_STRVAL_P(inc_filename), Z_STRLEN_P(inc_filename), &tmp_len, 0, "'\\\0..\37", 6 TSRMLS_CC);

		if (inc_filename == &tmp_inc_filename) {
			zval_dtor(&tmp_inc_filename);
		}
	}
	return ZEND_USER_OPCODE_DISPATCH;
}


PHP_MINIT_FUNCTION(xdebug)
{
	zend_extension dummy_ext;

	ZEND_INIT_MODULE_GLOBALS(xdebug, php_xdebug_init_globals, php_xdebug_shutdown_globals);
	REGISTER_INI_ENTRIES();

	/* initialize aggregate call information hash */
	zend_hash_init_ex(&XG(aggr_calls), 50, NULL, (dtor_func_t) xdebug_profile_aggr_call_entry_dtor, 1, 0);

	/* Redirect compile and execute functions to our own */
	old_compile_file = zend_compile_file;
	zend_compile_file = xdebug_compile_file;

	xdebug_old_execute = zend_execute;
	zend_execute = xdebug_execute;

	xdebug_old_execute_internal = zend_execute_internal;
	zend_execute_internal = xdebug_execute_internal;

	/* Replace error handler callback with our own */
	old_error_cb = zend_error_cb;
	new_error_cb = xdebug_error_cb;

	/* Get reserved offset */
	zend_xdebug_global_offset = zend_get_resource_handle(&dummy_ext);

	/* Overload the "exit" opcode */
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_EXIT);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_JMP);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_JMPZ);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_JMPNZ);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_IDENTICAL);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_NOT_IDENTICAL);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_EQUAL);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_NOT_EQUAL);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_SMALLER);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_IS_SMALLER_OR_EQUAL);

	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ADD_ARRAY_ELEMENT);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_RETURN);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_EXT_STMT);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_RAISE_ABSTRACT_ERROR);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAR);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAR_NO_REF);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SEND_VAL);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_NEW);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_EXT_FCALL_BEGIN);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_CATCH);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_BOOL);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ADD_STRING);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_INIT_ARRAY);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_R);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_R);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_W);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_FUNC_ARG);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_FUNC_ARG);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_DIM_UNSET);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_OBJ_UNSET);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_CLASS);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_FETCH_CONSTANT);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_CONCAT);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_ISSET_ISEMPTY_DIM_OBJ);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_PRE_INC_OBJ);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_SWITCH_FREE);
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_QM_ASSIGN);
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || PHP_MAJOR_VERSION >= 6
	XDEBUG_SET_OPCODE_OVERRIDE_COMMON(ZEND_DECLARE_LAMBDA_FUNCTION);
#endif
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(include_or_eval, ZEND_INCLUDE_OR_EVAL);

	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign, ZEND_ASSIGN);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_add, ZEND_ASSIGN_ADD);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_sub, ZEND_ASSIGN_SUB);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_mul, ZEND_ASSIGN_MUL);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_div, ZEND_ASSIGN_DIV);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_mod, ZEND_ASSIGN_MOD);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_sl, ZEND_ASSIGN_SL);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_sr, ZEND_ASSIGN_SR);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_concat, ZEND_ASSIGN_CONCAT);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_bw_or, ZEND_ASSIGN_BW_OR);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_bw_and, ZEND_ASSIGN_BW_AND);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_bw_xor, ZEND_ASSIGN_BW_XOR);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_dim, ZEND_ASSIGN_DIM);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(assign_obj, ZEND_ASSIGN_OBJ);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_inc, ZEND_PRE_INC);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_inc, ZEND_POST_INC);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(pre_dec, ZEND_PRE_DEC);
	XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(post_dec, ZEND_POST_DEC);

	zend_set_user_opcode_handler(ZEND_BEGIN_SILENCE, xdebug_silence_handler);
	zend_set_user_opcode_handler(ZEND_END_SILENCE, xdebug_silence_handler);

	if (zend_xdebug_initialised == 0) {
		zend_error(E_WARNING, "Xdebug MUST be loaded as a Zend extension");
	}

	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_APPEND", XDEBUG_TRACE_OPTION_APPEND, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_COMPUTERIZED", XDEBUG_TRACE_OPTION_COMPUTERIZED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_HTML", XDEBUG_TRACE_OPTION_HTML, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("XDEBUG_CC_UNUSED", XDEBUG_CC_OPTION_UNUSED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_CC_DEAD_CODE", XDEBUG_CC_OPTION_DEAD_CODE, CONST_CS | CONST_PERSISTENT);

	XG(breakpoint_count) = 0;
#ifndef ZTS
	if (sapi_module.header_handler != xdebug_header_handler) {
		xdebug_orig_header_handler = sapi_module.header_handler;
		sapi_module.header_handler = xdebug_header_handler;
	}
#endif

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(xdebug)
{
	if (XG(profiler_aggregate)) {
		xdebug_profiler_output_aggr_data(NULL TSRMLS_CC);
	}

	/* Reset compile, execute and error callbacks */
	zend_compile_file = old_compile_file;
	zend_execute = xdebug_old_execute;
	zend_execute_internal = xdebug_old_execute_internal;
	zend_error_cb = old_error_cb;

	zend_hash_destroy(&XG(aggr_calls));

#ifdef ZTS
	ts_free_id(xdebug_globals_id);
#else
	php_xdebug_shutdown_globals(&xdebug_globals TSRMLS_CC);
#endif

	return SUCCESS;
}

static void xdebug_llist_string_dtor(void *dummy, void *elem)
{
	char *s = elem;

	if (s) {
		xdfree(s);
	}
}

static void xdebug_used_var_dtor(void *dummy, void *elem)
{
	char *s = elem;

	if (s) {
		xdfree(s);
	}
}

static void xdebug_stack_element_dtor(void *dummy, void *elem)
{
	int                   i;
	function_stack_entry *e = elem;

	e->refcount--;

	if (e->refcount == 0) {
		if (e->function.function) {
			xdfree(e->function.function);
		}
		if (e->function.class) {
			xdfree(e->function.class);
		}
		if (e->filename) {
			xdfree(e->filename);
		}

		if (e->var) {
			for (i = 0; i < e->varc; i++) {
				if (e->var[i].name) {
					xdfree(e->var[i].name);
				}
			}
			xdfree(e->var);
		}

		if (e->include_filename) {
			xdfree(e->include_filename);
		}

		if (e->used_vars) {
			xdebug_llist_destroy(e->used_vars, NULL);
			e->used_vars = NULL;
		}

		if (e->profile.call_list) {
			xdebug_llist_destroy(e->profile.call_list, NULL);
			e->profile.call_list = NULL;
		}

		xdfree(e);
	}
}

#if PHP_VERSION_ID >= 50200
#define COOKIE_ENCODE , 1, 0
#elif PHP_API_VERSION >= 20030820
#define COOKIE_ENCODE , 1
#else
#define COOKIE_ENCODE
#endif

PHP_RINIT_FUNCTION(xdebug)
{
	zend_function *orig;
	char *idekey;
	zval **dummy;
	
	/* get xdebug ini entries from the environment also */
	XG(ide_key) = NULL;
	xdebug_env_config();
	idekey = zend_ini_string("xdebug.idekey", sizeof("xdebug.idekey"), 0);

	XG(no_exec)       = 0;
	XG(level)         = 0;
	XG(do_trace)      = 0;
	XG(do_code_coverage) = 0;
	XG(code_coverage) = xdebug_hash_alloc(32, xdebug_coverage_file_dtor);
	XG(stack)         = xdebug_llist_alloc(xdebug_stack_element_dtor);
	XG(trace_file)    = NULL;
	XG(tracefile_name) = NULL;
	XG(profile_file)  = NULL;
	XG(profile_filename) = NULL;
	XG(prev_memory)   = 0;
	XG(function_count) = -1;
	XG(active_symbol_table) = NULL;
	XG(This) = NULL;
	XG(last_exception_trace) = NULL;
	XG(last_eval_statement) = NULL;
	XG(do_collect_errors) = 0;
	XG(collected_errors)  = xdebug_llist_alloc(xdebug_llist_string_dtor);
	XG(reserved_offset) = zend_xdebug_global_offset;

	if (idekey && *idekey) {
		if (XG(ide_key)) {
			xdfree(XG(ide_key));
		}
		XG(ide_key) = xdstrdup(idekey);
	}

/* {{{ Initialize auto globals in Zend Engine 2 */
	zend_is_auto_global("_ENV",     sizeof("_ENV")-1     TSRMLS_CC);
	zend_is_auto_global("_GET",     sizeof("_GET")-1     TSRMLS_CC);
	zend_is_auto_global("_POST",    sizeof("_POST")-1    TSRMLS_CC);
	zend_is_auto_global("_COOKIE",  sizeof("_COOKIE")-1  TSRMLS_CC);
	zend_is_auto_global("_REQUEST", sizeof("_REQUEST")-1 TSRMLS_CC);
	zend_is_auto_global("_FILES",   sizeof("_FILES")-1   TSRMLS_CC);
	zend_is_auto_global("_SERVER",  sizeof("_SERVER")-1  TSRMLS_CC);
	zend_is_auto_global("_SESSION", sizeof("_SESSION")-1 TSRMLS_CC);
/* }}} */

	/* Check if we have this special get variable that stops a debugging
	 * request without executing any code */
	if (
		(
			(
				PG(http_globals)[TRACK_VARS_GET] &&
				zend_hash_find(PG(http_globals)[TRACK_VARS_GET]->value.ht, "XDEBUG_SESSION_STOP_NO_EXEC", sizeof("XDEBUG_SESSION_STOP_NO_EXEC"), (void **) &dummy) == SUCCESS
			) || (
				PG(http_globals)[TRACK_VARS_POST] &&
				zend_hash_find(PG(http_globals)[TRACK_VARS_POST]->value.ht, "XDEBUG_SESSION_STOP_NO_EXEC", sizeof("XDEBUG_SESSION_STOP_NO_EXEC"), (void **) &dummy) == SUCCESS
			)
		)
		&& !SG(headers_sent)
	) {
		php_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), "", 0, time(NULL) + XG(remote_cookie_expire_time), "/", 1, NULL, 0, 0 COOKIE_ENCODE TSRMLS_CC);
		XG(no_exec) = 1;
	}

	/* Only enabled extended info when it is not disabled */
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || PHP_MAJOR_VERSION >= 6
	CG(compiler_options) = CG(compiler_options) | (XG(extended_info) ? ZEND_COMPILE_EXTENDED_INFO : 0);
#else
	CG(extended_info) = XG(extended_info);
#endif

	/* Hack: We check for a soap header here, if that's existing, we don't use
	 * Xdebug's error handler to keep soap fault from fucking up. */
	if (XG(default_enable) && zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_SOAPACTION", 16, (void**)&dummy) == FAILURE) {
		zend_error_cb = new_error_cb;
		zend_throw_exception_hook = xdebug_throw_exception_hook;
	}
	XG(remote_enabled) = 0;
	XG(profiler_enabled) = 0;
	XG(breakpoints_allowed) = 1;
	if (XG(auto_trace) && XG(trace_output_dir) && strlen(XG(trace_output_dir))) {
		/* In case we do an auto-trace we are not interested in the return
		 * value, but we still have to free it. */
		xdfree(xdebug_start_trace(NULL, XG(trace_options) TSRMLS_CC));
	}

	/* Initialize some debugger context properties */
	XG(context).program_name   = NULL;
	XG(context).list.last_file = NULL;
	XG(context).list.last_line = 0;
	XG(context).do_break       = 0;
	XG(context).do_step        = 0;
	XG(context).do_next        = 0;
	XG(context).do_finish      = 0;

	/* Initialize dump superglobals */
	XG(dumped) = 0;

	/* Initialize start time */
	XG(start_time) = xdebug_get_utime();

	/* Override var_dump with our own function */
	XG(var_dump_overloaded) = 0;
	if (XG(overload_var_dump)) {
		zend_hash_find(EG(function_table), "var_dump", 9, (void **)&orig);
		XG(orig_var_dump_func) = orig->internal_function.handler;
		orig->internal_function.handler = zif_xdebug_var_dump;
		XG(var_dump_overloaded) = 1;
	}

	/* Override set_time_limit with our own function to prevent timing out while debugging */
	zend_hash_find(EG(function_table), "set_time_limit", 15, (void **)&orig);
	XG(orig_set_time_limit_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_xdebug_set_time_limit;

	XG(headers) = xdebug_llist_alloc(xdebug_llist_string_dtor);
	if (strcmp(sapi_module.name, "cli") == 0) {
		SG(request_info).no_headers = 1;
	}

	return SUCCESS;
}

ZEND_MODULE_POST_ZEND_DEACTIVATE_D(xdebug)
{
	zend_function *orig;
	TSRMLS_FETCH();

	if (XG(remote_enabled)) {
		XG(context).handler->remote_deinit(&(XG(context)));
		xdebug_close_socket(XG(context).socket); 
	}
	if (XG(context).program_name) {
		xdfree(XG(context).program_name);
	}

	xdebug_llist_destroy(XG(stack), NULL);
	XG(stack) = NULL;

	if (XG(do_trace) && XG(trace_file)) {
		xdebug_stop_trace(TSRMLS_C);
	}

	if (XG(profile_file)) {
		fclose(XG(profile_file));
	}

	if (XG(profile_filename)) {
		xdfree(XG(profile_filename));
	}

	if (XG(ide_key)) {
		xdfree(XG(ide_key));
		XG(ide_key) = NULL;
	}

	XG(level)            = 0;
	XG(do_trace)         = 0;
	XG(do_code_coverage) = 0;

	xdebug_hash_destroy(XG(code_coverage));
	XG(code_coverage) = NULL;

	if (XG(context.list.last_file)) {
		xdfree(XG(context).list.last_file);
	}

	if (XG(last_exception_trace)) {
		xdfree(XG(last_exception_trace));
	}

	if (XG(last_eval_statement)) {
		efree(XG(last_eval_statement));
	}

	xdebug_llist_destroy(XG(collected_errors), NULL);
	XG(collected_errors) = NULL;

	/* Reset var_dump and set_time_limit to the original function */
	if (XG(var_dump_overloaded)) {
		zend_hash_find(EG(function_table), "var_dump", 9, (void **)&orig);
		orig->internal_function.handler = XG(orig_var_dump_func);
	}
	zend_hash_find(EG(function_table), "set_time_limit", 15, (void **)&orig);
	orig->internal_function.handler = XG(orig_set_time_limit_func);

	/* Clean up collected headers */
	xdebug_llist_destroy(XG(headers), NULL);
	XG(headers) = NULL;

	return SUCCESS;
}


PHP_MINFO_FUNCTION(xdebug)
{
	xdebug_remote_handler_info *ptr = xdebug_handlers_get();

	php_info_print_table_start();
	php_info_print_table_header(2, "xdebug support", "enabled");
	php_info_print_table_row(2, "Version", XDEBUG_VERSION);
	php_info_print_table_end();
	
	if (zend_xdebug_initialised == 0) {
		php_info_print_table_start();
		php_info_print_table_header(1, "XDEBUG NOT LOADED AS ZEND EXTENSION");
		php_info_print_table_end();
	}

	php_info_print_table_start();
	php_info_print_table_header(2, "Supported protocols", "Revision");
	while (ptr->name) {
		php_info_print_table_row(2, ptr->description, ptr->handler.get_revision());
		ptr++;
	}
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

static void add_used_variables(function_stack_entry *fse, zend_op_array *op_array)
{
	int i = 0; 

	if (!fse->used_vars) {
		fse->used_vars = xdebug_llist_alloc(xdebug_used_var_dtor);
	}

	/* Check parameters */
	for (i = 0; i < fse->varc; i++) {
		if (fse->var[i].name) {
			xdebug_llist_insert_next(fse->used_vars, XDEBUG_LLIST_TAIL(fse->used_vars), xdstrdup(fse->var[i].name));
		}
	}

	/* gather used variables from compiled vars information */
	while (i < op_array->last_var) {
		xdebug_llist_insert_next(fse->used_vars, XDEBUG_LLIST_TAIL(fse->used_vars), xdstrdup(op_array->vars[i].name));
		i++;
	}

	/* opcode scanning time */
	while (i < op_array->size) {
		char *cv = NULL;
		int cv_len;

		if (op_array->opcodes[i].op1.op_type == IS_CV) {
			cv = zend_get_compiled_variable_name(op_array, op_array->opcodes[i].op1.u.var, &cv_len);
			xdebug_llist_insert_next(fse->used_vars, XDEBUG_LLIST_TAIL(fse->used_vars), xdstrdup(cv));
		}
		if (op_array->opcodes[i].op2.op_type == IS_CV) {
			cv = zend_get_compiled_variable_name(op_array, op_array->opcodes[i].op2.u.var, &cv_len);
			xdebug_llist_insert_next(fse->used_vars, XDEBUG_LLIST_TAIL(fse->used_vars), xdstrdup(cv));
		}
		i++;
	}
}

static void xdebug_throw_exception_hook(zval *exception TSRMLS_DC)
{
	zval *message, *file, *line, *xdebug_message_trace, *previous_exception;
	zend_class_entry *default_ce, *exception_ce;
	xdebug_brk_info *extra_brk_info;
	char *exception_trace;
	xdebug_str tmp_str = { 0, 0, NULL };

	if (!exception) {
		return;
	}

	default_ce = zend_exception_get_default(TSRMLS_C);
	exception_ce = zend_get_class_entry(exception TSRMLS_CC);

	message = zend_read_property(default_ce, exception, "message", sizeof("message")-1, 0 TSRMLS_CC);
	file =    zend_read_property(default_ce, exception, "file",    sizeof("file")-1,    0 TSRMLS_CC);
	line =    zend_read_property(default_ce, exception, "line",    sizeof("line")-1,    0 TSRMLS_CC);

	if (Z_TYPE_P(message) != IS_STRING || Z_TYPE_P(file) != IS_STRING || Z_TYPE_P(line) != IS_LONG) {
		php_error(E_ERROR, "Your exception class uses incorrect types for common properties: 'message' and 'file' need to be a string and 'line' needs to be an integer.");
	}

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 2) || PHP_MAJOR_VERSION >= 6
	previous_exception = zend_read_property(default_ce, exception, "previous", sizeof("previous")-1, 1 TSRMLS_CC);
	if (previous_exception && Z_TYPE_P(previous_exception) != IS_NULL) {
		xdebug_message_trace = zend_read_property(default_ce, previous_exception, "xdebug_message", sizeof("xdebug_message")-1, 1 TSRMLS_CC);
		if (xdebug_message_trace && Z_TYPE_P(xdebug_message_trace) != IS_NULL) {
			xdebug_str_add(&tmp_str, Z_STRVAL_P(xdebug_message_trace), 0);
		}
	}
#endif
	if (!PG(html_errors)) {
		xdebug_str_addl(&tmp_str, "\n", 1, 0);
	}
	xdebug_append_error_description(&tmp_str, PG(html_errors), exception_ce->name, Z_STRVAL_P(message), Z_STRVAL_P(file), Z_LVAL_P(line) TSRMLS_CC);
	xdebug_append_printable_stack(&tmp_str, PG(html_errors) TSRMLS_CC);
	exception_trace = tmp_str.d;
	zend_update_property_string(default_ce, exception, "xdebug_message", sizeof("xdebug_message")-1, exception_trace TSRMLS_CC);

	if (XG(last_exception_trace)) {
		xdfree(XG(last_exception_trace));
	}
	XG(last_exception_trace) = exception_trace;

	if (XG(show_ex_trace)) {
		if (PG(log_errors)) {
			xdebug_log_stack(exception_ce->name, Z_STRVAL_P(message), Z_STRVAL_P(file), Z_LVAL_P(line) TSRMLS_CC);
		}
		if (PG(display_errors)) {
			php_printf("%s", exception_trace);
		}
	}

	/* Start JIT if requested and not yet enabled */
	xdebug_do_jit(TSRMLS_C);

	if (XG(remote_enabled)) {
		/* Check if we have a breakpoint on this exception */
		if (xdebug_hash_find(XG(context).exception_breakpoints, exception_ce->name, strlen(exception_ce->name), (void *) &extra_brk_info)) {
			if (xdebug_handle_hit_value(extra_brk_info)) {
				if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), Z_STRVAL_P(file), Z_LVAL_P(line), XDEBUG_BREAK, exception_ce->name, Z_STRVAL_P(message))) {
					XG(remote_enabled) = 0;
				}
			}
		}
	}
}

static int handle_breakpoints(function_stack_entry *fse, int breakpoint_type)
{
	xdebug_brk_info *extra_brk_info = NULL;
	char            *tmp_name = NULL;
	TSRMLS_FETCH();

	/* Function breakpoints */
	if (fse->function.type == XFUNC_NORMAL) {
		if (xdebug_hash_find(XG(context).function_breakpoints, fse->function.function, strlen(fse->function.function), (void *) &extra_brk_info)) {
			/* Yup, breakpoint found, we call the handler when it's not
			 * disabled AND handle_hit_value is happy */
			if (!extra_brk_info->disabled && (extra_brk_info->function_break_type == breakpoint_type)) {
				if (xdebug_handle_hit_value(extra_brk_info)) {
					if (fse->user_defined == XDEBUG_INTERNAL || (breakpoint_type == XDEBUG_BRK_FUNC_RETURN)) {
						if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), fse->filename, fse->lineno, XDEBUG_BREAK, NULL, NULL)) {
							return 0;
						}
					} else {
						XG(context).do_break = 1;
					}
				}
			}
		}
	}
	/* class->function breakpoints */
	else if (fse->function.type == XFUNC_MEMBER || fse->function.type == XFUNC_STATIC_MEMBER) {
		if (fse->function.type == XFUNC_MEMBER) {
			tmp_name = xdebug_sprintf("%s->%s", fse->function.class, fse->function.function);
		} else if( fse->function.type == XFUNC_STATIC_MEMBER) {
			tmp_name = xdebug_sprintf("%s::%s", fse->function.class, fse->function.function);
		}

		if (xdebug_hash_find(XG(context).function_breakpoints, tmp_name, strlen(tmp_name), (void *) &extra_brk_info)) {
			/* Yup, breakpoint found, call handler if the breakpoint is not
			 * disabled AND handle_hit_value is happy */
			if (!extra_brk_info->disabled && (extra_brk_info->function_break_type == breakpoint_type)) {
				if (xdebug_handle_hit_value(extra_brk_info)) {
					XG(context).do_break = 1;
				}
			}
		}
		xdfree(tmp_name);
	}
	return 1;
}

void xdebug_execute(zend_op_array *op_array TSRMLS_DC)
{
	zval                **dummy;
	zend_execute_data    *edata = EG(current_execute_data);
	function_stack_entry *fse, *xfse;
	char                 *magic_cookie = NULL;
	int                   do_return = (XG(do_trace) && XG(trace_file));
	int                   function_nr = 0;
	xdebug_llist_element *le;
	int                   eval_id = 0;


	if (XG(no_exec) == 1) {
		php_printf("DEBUG SESSION ENDED");
		return;
	}

	if (!XG(context).program_name) {
		XG(context).program_name = xdstrdup(op_array->filename);
	}

	if (XG(level) == 0) {
		/* Set session cookie if requested */
		if (
			((
				PG(http_globals)[TRACK_VARS_GET] &&
				zend_hash_find(PG(http_globals)[TRACK_VARS_GET]->value.ht, "XDEBUG_SESSION_START", sizeof("XDEBUG_SESSION_START"), (void **) &dummy) == SUCCESS
			) || (
				PG(http_globals)[TRACK_VARS_POST] &&
				zend_hash_find(PG(http_globals)[TRACK_VARS_POST]->value.ht, "XDEBUG_SESSION_START", sizeof("XDEBUG_SESSION_START"), (void **) &dummy) == SUCCESS
			))
			&& !SG(headers_sent)
		) {
			convert_to_string_ex(dummy);
			magic_cookie = xdstrdup(Z_STRVAL_PP(dummy));
			if (XG(ide_key)) {
				xdfree(XG(ide_key));
			}
			XG(ide_key) = xdstrdup(Z_STRVAL_PP(dummy));
			php_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), Z_STRVAL_PP(dummy), Z_STRLEN_PP(dummy), time(NULL) + XG(remote_cookie_expire_time), "/", 1, NULL, 0, 0 COOKIE_ENCODE TSRMLS_CC);
		} else if (
			PG(http_globals)[TRACK_VARS_COOKIE] &&
			zend_hash_find(PG(http_globals)[TRACK_VARS_COOKIE]->value.ht, "XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), (void **) &dummy) == SUCCESS
		) {
			convert_to_string_ex(dummy);
			magic_cookie = xdstrdup(Z_STRVAL_PP(dummy));
			if (XG(ide_key)) {
				xdfree(XG(ide_key));
			}
			XG(ide_key) = xdstrdup(Z_STRVAL_PP(dummy));
		} else if (getenv("XDEBUG_CONFIG")) {
			magic_cookie = xdstrdup(getenv("XDEBUG_CONFIG"));
			if (XG(ide_key) && *XG(ide_key) && !SG(headers_sent)) {
				php_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), XG(ide_key), strlen(XG(ide_key)), time(NULL) + XG(remote_cookie_expire_time), "/", 1, NULL, 0, 0 COOKIE_ENCODE TSRMLS_CC);
			}
		}
			

		/* Remove session cookie if requested */
		if (
			(
				(
					PG(http_globals)[TRACK_VARS_GET] &&
					zend_hash_find(PG(http_globals)[TRACK_VARS_GET]->value.ht, "XDEBUG_SESSION_STOP", sizeof("XDEBUG_SESSION_STOP"), (void **) &dummy) == SUCCESS
				) || (
					PG(http_globals)[TRACK_VARS_POST] &&
					zend_hash_find(PG(http_globals)[TRACK_VARS_POST]->value.ht, "XDEBUG_SESSION_STOP", sizeof("XDEBUG_SESSION_STOP"), (void **) &dummy) == SUCCESS
				)
			)
			&& !SG(headers_sent)
		) {
			if (magic_cookie) {
				xdfree(magic_cookie);
				magic_cookie = NULL;
			}
			php_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), "", 0, time(NULL) + XG(remote_cookie_expire_time), "/", 1, NULL, 0, 0 COOKIE_ENCODE TSRMLS_CC);
		}

		/* Start remote context if requested */
		if (
			(magic_cookie || XG(remote_autostart)) &&
			!XG(remote_enabled) &&
			XG(remote_enable) &&
			(XG(remote_mode) == XDEBUG_REQ)
		) {
			/* Initialize debugging session */
			if (XG(remote_connect_back)) {
				zval **remote_addr = NULL;
				zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "REMOTE_ADDR", 12, (void**)&remote_addr);
				if (remote_addr) {
					XG(context).socket = xdebug_create_socket(Z_STRVAL_PP(remote_addr), XG(remote_port));
				} else {
					XG(context).socket = xdebug_create_socket(XG(remote_host), XG(remote_port));
				}
			} else {
				XG(context).socket = xdebug_create_socket(XG(remote_host), XG(remote_port));
			}
			if (XG(context).socket >= 0) {
				XG(remote_enabled) = 1;

				/* Get handler from mode */
				XG(context).handler = xdebug_handler_get(XG(remote_handler));
				if (!XG(context).handler) {
					XG(remote_enabled) = 0;
					zend_error(E_WARNING, "The remote debug handler '%s' is not supported.", XG(remote_handler));
				} else if (!XG(context).handler->remote_init(&(XG(context)), XDEBUG_REQ)) {
					/* The request could not be started, ignore it then */
					XG(remote_enabled) = 0;
				} else {
					/* All is well, turn off script time outs */
					zend_alter_ini_entry("max_execution_time", sizeof("max_execution_time"), "0", strlen("0"), PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
				}
			}
		}
		if (magic_cookie) {
			xdfree(magic_cookie);
			magic_cookie = NULL;
		}

		/* Check for special GET/POST parameter to start profiling */
		if (
			!XG(profiler_enabled) &&
			(
				XG(profiler_enable)
				|| 
				(
					XG(profiler_enable_trigger) &&
					(
						(
							PG(http_globals)[TRACK_VARS_GET] && 
							zend_hash_find(PG(http_globals)[TRACK_VARS_GET]->value.ht, "XDEBUG_PROFILE", sizeof("XDEBUG_PROFILE"), (void **) &dummy) == SUCCESS
						) || (
							PG(http_globals)[TRACK_VARS_POST] && 
							zend_hash_find(PG(http_globals)[TRACK_VARS_POST]->value.ht, "XDEBUG_PROFILE", sizeof("XDEBUG_PROFILE"), (void **) &dummy) == SUCCESS
						) || (
							PG(http_globals)[TRACK_VARS_COOKIE] && 
							zend_hash_find(PG(http_globals)[TRACK_VARS_COOKIE]->value.ht, "XDEBUG_PROFILE", sizeof("XDEBUG_PROFILE"), (void **) &dummy) == SUCCESS
						)
					)
				)
			)
		) {
			if (xdebug_profiler_init(op_array->filename TSRMLS_CC) == SUCCESS) {
				XG(profiler_enabled) = 1;
			}
		}
	}

	XG(level)++;
	if (XG(level) == XG(max_nesting_level)) {
		php_error(E_ERROR, "Maximum function nesting level of '%ld' reached, aborting!", XG(max_nesting_level));
	}

	fse = xdebug_add_stack_frame(edata, op_array, XDEBUG_EXTERNAL TSRMLS_CC);

	function_nr = XG(function_count);
	xdebug_trace_function_begin(fse, function_nr TSRMLS_CC);

	fse->symbol_table = EG(active_symbol_table);
	fse->execute_data = EG(current_execute_data);
	fse->This = EG(This);

	if (XG(remote_enabled) || XG(collect_vars) || XG(show_local_vars)) {
		/* Because include/require is treated as a stack level, we have to add used
		 * variables in include/required files to all the stack levels above, until
		 * we hit a function or the top level stack.  This is so that the variables
		 * show up correctly where they should be.  We always call
		 * add_used_variables on the current stack level, otherwise vars in include
		 * files do not show up in the locals list.  */
		for (le = XDEBUG_LLIST_TAIL(XG(stack)); le != NULL; le = XDEBUG_LLIST_PREV(le)) {
			xfse = XDEBUG_LLIST_VALP(le);
			add_used_variables(xfse, op_array);
			if (XDEBUG_IS_FUNCTION(xfse->function.type)) {
				break;
			}
		}
	}

	if (XG(do_code_coverage) && XG(code_coverage_unused)) {
		xdebug_prefill_code_coverage(op_array TSRMLS_CC);
	}

	/* If we're in an eval, we need to create an ID for it. This ID however
	 * depends on the debugger mechanism in use so we need to call a function
	 * in the handler for it */
	if (XG(remote_enabled) && XG(context).handler->register_eval_id && fse->function.type == XFUNC_EVAL) {
		eval_id = XG(context).handler->register_eval_id(&(XG(context)), fse);
	}

	/* Check for entry breakpoints */
	if (XG(remote_enabled) && XG(breakpoints_allowed)) {
		if (!handle_breakpoints(fse, XDEBUG_BRK_FUNC_CALL)) {
			XG(remote_enabled) = 0;
		}
	}

	if (XG(profiler_enabled)) {
		xdebug_profiler_function_user_begin(fse TSRMLS_CC);
	}
	xdebug_old_execute(op_array TSRMLS_CC);

	if (XG(profiler_enabled)) {
		xdebug_profiler_function_user_end(fse, op_array TSRMLS_CC);
	}

	xdebug_trace_function_end(fse, function_nr TSRMLS_CC);

	/* Store return value in the trace file */
	if (XG(collect_return) && do_return && XG(do_trace) && XG(trace_file)) {
		if (EG(return_value_ptr_ptr) && *EG(return_value_ptr_ptr)) {
			char* t = xdebug_return_trace_stack_retval(fse, *EG(return_value_ptr_ptr) TSRMLS_CC);
			fprintf(XG(trace_file), "%s", t);
			fflush(XG(trace_file));
			xdfree(t);
		}
	}

	/* Check for return breakpoints */
	if (XG(remote_enabled) && XG(breakpoints_allowed)) {
		if (!handle_breakpoints(fse, XDEBUG_BRK_FUNC_RETURN)) {
			XG(remote_enabled) = 0;
		}
	}

	/* If we're in an eval, we need to destroy the created ID again. */
	if (XG(remote_enabled) && XG(context).handler->unregister_eval_id && fse->function.type == XFUNC_EVAL) {
		XG(context).handler->unregister_eval_id(&(XG(context)), fse, eval_id);
	}

	fse->symbol_table = NULL;
	fse->execute_data = NULL;
	xdebug_llist_remove(XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), xdebug_stack_element_dtor);
	XG(level)--;
}

void xdebug_execute_internal(zend_execute_data *current_execute_data, int return_value_used TSRMLS_DC)
{
	zend_execute_data    *edata = EG(current_execute_data);
	function_stack_entry *fse;
	zend_op              *cur_opcode;
	int                   do_return = (XG(do_trace) && XG(trace_file));
	int                   function_nr = 0;

	XG(level)++;
	if (XG(level) == XG(max_nesting_level)) {
		php_error(E_ERROR, "Maximum function nesting level of '%ld' reached, aborting!", XG(max_nesting_level));
	}

	fse = xdebug_add_stack_frame(edata, edata->op_array, XDEBUG_INTERNAL TSRMLS_CC);

	function_nr = XG(function_count);
	xdebug_trace_function_begin(fse, function_nr TSRMLS_CC);

	/* Check for entry breakpoints */
	if (XG(remote_enabled) && XG(breakpoints_allowed)) {
		if (!handle_breakpoints(fse, XDEBUG_BRK_FUNC_CALL)) {
			XG(remote_enabled) = 0;
		}
	}
	
	if (XG(profiler_enabled)) {
		xdebug_profiler_function_internal_begin(fse TSRMLS_CC);
	}
	if (xdebug_old_execute_internal) {
		xdebug_old_execute_internal(current_execute_data, return_value_used TSRMLS_CC);
	} else {
		execute_internal(current_execute_data, return_value_used TSRMLS_CC);
	}

	if (XG(profiler_enabled)) {
		xdebug_profiler_function_internal_end(fse TSRMLS_CC);
	}

	xdebug_trace_function_end(fse, function_nr TSRMLS_CC);

	/* Store return value in the trace file */
	if (XG(collect_return) && do_return && XG(do_trace) && XG(trace_file)) {
		cur_opcode = *EG(opline_ptr);
		if (cur_opcode) {
			zval *ret = xdebug_zval_ptr(&(cur_opcode->result), current_execute_data->Ts TSRMLS_CC);
			char* t = xdebug_return_trace_stack_retval(fse, ret TSRMLS_CC);
			fprintf(XG(trace_file), "%s", t);
			fflush(XG(trace_file));
			xdfree(t);
		}
	}

	/* Check for return breakpoints */
	if (XG(remote_enabled) && XG(breakpoints_allowed)) {
		if (!handle_breakpoints(fse, XDEBUG_BRK_FUNC_RETURN)) {
			XG(remote_enabled) = 0;
		}
	}

	xdebug_llist_remove(XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), xdebug_stack_element_dtor);
	XG(level)--;
}

/* Opcode handler for exit, to be able to clean up the profiler */
int xdebug_exit_handler(ZEND_OPCODE_HANDLER_ARGS)
{
	if (XG(profiler_enabled)) {
		xdebug_profiler_deinit(TSRMLS_C);
	}
	
	return ZEND_USER_OPCODE_DISPATCH;
}


/* {{{ zend_op_array srm_compile_file (file_handle, type)
 *    This function provides a hook for the execution of bananas */
zend_op_array *xdebug_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC)
{
	zend_op_array *op_array;
	
	op_array = old_compile_file(file_handle, type TSRMLS_CC);

	if (op_array) {
		if (XG(do_code_coverage) && XG(code_coverage_unused && op_array->done_pass_two)) {
			xdebug_prefill_code_coverage(op_array TSRMLS_CC);
		}
	}
	return op_array;
}
/* }}} */

static int xdebug_header_handler(sapi_header_struct *h XG_SAPI_HEADER_OP_DC, sapi_headers_struct *s TSRMLS_DC)
{
	if (XG(headers)) {
		xdebug_llist_insert_next(XG(headers), XDEBUG_LLIST_TAIL(XG(headers)), xdstrdup(h->header));
	}
	if (xdebug_orig_header_handler) {
		return xdebug_orig_header_handler(h XG_SAPI_HEADER_OP_CC, s TSRMLS_CC);
	}
	return SAPI_HEADER_ADD;
}


/* {{{ proto void xdebug_set_time_limit(void)
   Dummy function to prevent time limit from being set within the script */
PHP_FUNCTION(xdebug_set_time_limit)
{
	if (!XG(remote_enabled)) {
		XG(orig_set_time_limit_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	}
}
/* }}} */

/* {{{ proto void xdebug_var_dump(mixed var [, ...] )
   Outputs a fancy string representation of a variable */
PHP_FUNCTION(xdebug_var_dump)
{
	zval ***args;
	int     argc;
	int     i, len;
	char   *val;
	
	argc = ZEND_NUM_ARGS();
	
	args = (zval ***)emalloc(argc * sizeof(zval **));
	if (ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
		efree(args);
		WRONG_PARAM_COUNT;
	}
	
	for (i = 0; i < argc; i++) {
		if (PG(html_errors)) {
			val = xdebug_get_zval_value_fancy(NULL, (zval*) *args[i], &len, 0, NULL TSRMLS_CC);
			PHPWRITE(val, len);
			xdfree(val);
		} else {
			xdebug_php_var_dump(args[i], 1 TSRMLS_CC);
		}
	}
	
	efree(args);
}
/* }}} */

/* {{{ proto void xdebug_debug_zval(mixed var [, ...] )
   Outputs a fancy string representation of a variable */
PHP_FUNCTION(xdebug_debug_zval)
{
	zval ***args;
	int     argc;
	int     i, len;
	char   *val;
	zval   *debugzval;
	
	argc = ZEND_NUM_ARGS();
	
	args = (zval ***)emalloc(argc * sizeof(zval **));
	if (ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
		efree(args);
		WRONG_PARAM_COUNT;
	}
	
	for (i = 0; i < argc; i++) {
		if (Z_TYPE_PP(args[i]) == IS_STRING) {
			XG(active_symbol_table) = EG(active_symbol_table);
			debugzval = xdebug_get_php_symbol(Z_STRVAL_PP(args[i]), Z_STRLEN_PP(args[i]) + 1);
			if (debugzval) {
				php_printf("%s: ", Z_STRVAL_PP(args[i]));
				if (PG(html_errors)) {
					val = xdebug_get_zval_value_fancy(NULL, debugzval, &len, 1, NULL TSRMLS_CC);
					PHPWRITE(val, len);
				} else {
					val = xdebug_get_zval_value(debugzval, 1, NULL);
					PHPWRITE(val, strlen(val));
				}
				xdfree(val);
				PHPWRITE("\n", 1);
			}
		}
	}
	
	efree(args);
}
/* }}} */

/* {{{ proto void xdebug_debug_zval_stdout(mixed var [, ...] )
   Outputs a fancy string representation of a variable */
PHP_FUNCTION(xdebug_debug_zval_stdout)
{
	zval ***args;
	int     argc;
	int     i;
	char   *val;
	zval   *debugzval;
	
	argc = ZEND_NUM_ARGS();
	
	args = (zval ***)emalloc(argc * sizeof(zval **));
	if (ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
		efree(args);
		WRONG_PARAM_COUNT;
	}
	
	for (i = 0; i < argc; i++) {
		if (Z_TYPE_PP(args[i]) == IS_STRING) {
			XG(active_symbol_table) = EG(active_symbol_table);
			debugzval = xdebug_get_php_symbol(Z_STRVAL_PP(args[i]), Z_STRLEN_PP(args[i]) + 1);
			if (debugzval) {
				printf("%s: ", Z_STRVAL_PP(args[i]));
				val = xdebug_get_zval_value(debugzval, 1, NULL);
				printf("%s(%zd)", val, strlen(val));
				xdfree(val);
				printf("\n");
			}
		}
	}
	
	efree(args);
}
/* }}} */

PHP_FUNCTION(xdebug_enable)
{
	zend_error_cb = new_error_cb;
	zend_throw_exception_hook = xdebug_throw_exception_hook;
}

PHP_FUNCTION(xdebug_disable)
{
	zend_error_cb = old_error_cb;
	zend_throw_exception_hook = NULL;
}

PHP_FUNCTION(xdebug_is_enabled)
{
	RETURN_BOOL(zend_error_cb == new_error_cb);
}

PHP_FUNCTION(xdebug_break)
{
	char *file;
	int   lineno;

	if (XG(remote_enabled)) {
		file = zend_get_executed_filename(TSRMLS_C);
		lineno = zend_get_executed_lineno(TSRMLS_C);

		if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_BREAK, NULL, NULL)) {
			XG(remote_enabled) = 0;
		}
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_start_error_collection)
{
	if (XG(do_collect_errors) == 1) {
		php_error(E_NOTICE, "Error collection was already started");
	}
	XG(do_collect_errors) = 1;
}

PHP_FUNCTION(xdebug_stop_error_collection)
{
	if (XG(do_collect_errors) == 0) {
		php_error(E_NOTICE, "Error collection was not started");
	}
	XG(do_collect_errors) = 0;
}

PHP_FUNCTION(xdebug_get_collected_errors)
{
	xdebug_llist_element *le;
	char                 *string;
	zend_bool             clear = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &clear) == FAILURE) {
		return;
	}

	array_init(return_value);
	for (le = XDEBUG_LLIST_HEAD(XG(collected_errors)); le != NULL; le = XDEBUG_LLIST_NEXT(le))	{
		string = XDEBUG_LLIST_VALP(le);
		add_next_index_string(return_value, string, 1);
	}

	if (clear) {
		xdebug_llist_destroy(XG(collected_errors), NULL);
		XG(collected_errors) = xdebug_llist_alloc(xdebug_llist_string_dtor);
	}
}

PHP_FUNCTION(xdebug_get_headers)
{
	xdebug_llist_element *le;
	char                 *string;

	array_init(return_value);
	for (le = XDEBUG_LLIST_HEAD(XG(headers)); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
		string = XDEBUG_LLIST_VALP(le);
		add_next_index_string(return_value, string, 1);
	}
	xdebug_llist_empty(XG(headers), NULL);
}


PHP_FUNCTION(xdebug_get_profiler_filename)
{
	if (XG(profile_filename)) {
		RETURN_STRING(XG(profile_filename), 1);
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_dump_aggr_profiling_data)
{
	char *prefix = NULL;
	int prefix_len;

	if (!XG(profiler_aggregate)) {
		RETURN_FALSE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &prefix, &prefix_len) == FAILURE) {
		return;
	}

	if (xdebug_profiler_output_aggr_data(prefix TSRMLS_CC) == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_clear_aggr_profiling_data)
{
	if (!XG(profiler_aggregate)) {
		RETURN_FALSE;
	}

	zend_hash_clean(&XG(aggr_calls));

	RETURN_TRUE;
}

#if HAVE_PHP_MEMORY_USAGE
PHP_FUNCTION(xdebug_memory_usage)
{
	RETURN_LONG(XG_MEMORY_USAGE());
}

PHP_FUNCTION(xdebug_peak_memory_usage)
{
	RETURN_LONG(XG_MEMORY_PEAK_USAGE());
}
#endif

PHP_FUNCTION(xdebug_time_index)
{
	RETURN_DOUBLE(xdebug_get_utime() - XG(start_time));
}

ZEND_DLEXPORT void xdebug_statement_call(zend_op_array *op_array)
{
	xdebug_llist_element *le;
	xdebug_brk_info      *brk;
	function_stack_entry *fse;
	int                   lineno;
	char                 *file;
	int                   file_len = 0;
	int                   level = 0;
	TSRMLS_FETCH();

	lineno = EG(current_execute_data)->opline->lineno;

	file = op_array->filename;
	file_len = strlen(file);

	if (XG(do_code_coverage)) {
		xdebug_count_line(file, lineno, 0, 0 TSRMLS_CC);
	}

	if (XG(remote_enabled)) {

		if (XG(context).do_break) {
			XG(context).do_break = 0;

			if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_BREAK, NULL, NULL)) {
				XG(remote_enabled) = 0;
				return;
			}
		}

		/* Get latest stack level */
		if (XG(stack)) {
			le = XDEBUG_LLIST_TAIL(XG(stack));
			fse = XDEBUG_LLIST_VALP(le);
			level = fse->level;
		} else {
			level = 0;
		}
		
		if (XG(context).do_finish && XG(context).next_level == level) { /* Check for "finish" */
			XG(context).do_finish = 0;

			if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_STEP, NULL, NULL)) {
				XG(remote_enabled) = 0;
				return;
			}
			return;
		}

		if (XG(context).do_next && XG(context).next_level >= level) { /* Check for "next" */
			XG(context).do_next = 0;

			if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_STEP, NULL, NULL)) {
				XG(remote_enabled) = 0;
				return;
			}
			return;
		}

		if (XG(context).do_step) { /* Check for "step" */
			XG(context).do_step = 0;

			if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_STEP, NULL, NULL)) {
				XG(remote_enabled) = 0;
				return;
			}
			return;
		}

		if (XG(context).line_breakpoints) {
			int   break_ok;
			int   old_error_reporting;
			zval  retval;

			for (le = XDEBUG_LLIST_HEAD(XG(context).line_breakpoints); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
				brk = XDEBUG_LLIST_VALP(le);

#if 0
				printf("b->d: %d; ln: %d; b->l: %d; b->f: %s; f: %s, f_l: %d; b->f_l: %d\n",
						brk->disabled, lineno, brk->lineno, brk->file, file, file_len, brk->file_len);
#endif
#if PHP_WIN32
				if (!brk->disabled && lineno == brk->lineno && strncasecmp(brk->file, file + file_len - brk->file_len, brk->file_len) == 0) {
#else
				if (!brk->disabled && lineno == brk->lineno && memcmp(brk->file, file + file_len - brk->file_len, brk->file_len) == 0) {
#endif
					break_ok = 1; /* Breaking is allowed by default */

					/* Check if we have a condition set for it */
					if (brk->condition) {
						/* If there is a condition, we disable breaking by
						 * default and only enabled it when the code evaluates
						 * to TRUE */
						break_ok = 0;

						/* Remember error reporting level */
						old_error_reporting = EG(error_reporting);
						EG(error_reporting) = 0;

						/* Check the condition */
						if (zend_eval_string(brk->condition, &retval, "xdebug conditional breakpoint" TSRMLS_CC) == SUCCESS) {
							convert_to_boolean(&retval);
							break_ok = retval.value.lval;
							zval_dtor(&retval);
						}

						/* Restore error reporting level */
						EG(error_reporting) = old_error_reporting;
					}
					if (break_ok && xdebug_handle_hit_value(brk)) {
						if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_BREAK, NULL, NULL)) {
							XG(remote_enabled) = 0;
							break;
						}
						return;
					}
				}
			}
		}
	}
}


ZEND_DLEXPORT int xdebug_zend_startup(zend_extension *extension)
{
	zend_xdebug_initialised = 1;

	return zend_startup_module(&xdebug_module_entry);
}

ZEND_DLEXPORT void xdebug_zend_shutdown(zend_extension *extension)
{
	/* Do nothing. */
}

ZEND_DLEXPORT void xdebug_init_oparray(zend_op_array *op_array)
{
	TSRMLS_FETCH();
	op_array->reserved[XG(reserved_offset)] = 0;
}

#ifndef ZEND_EXT_API
#define ZEND_EXT_API    ZEND_DLEXPORT
#endif
ZEND_EXTENSION();

ZEND_DLEXPORT zend_extension zend_extension_entry = {
	XDEBUG_NAME,
	XDEBUG_VERSION,
	XDEBUG_AUTHOR,
	XDEBUG_URL,
	XDEBUG_COPYRIGHT_SHORT,
	xdebug_zend_startup,
	xdebug_zend_shutdown,
	NULL,           /* activate_func_t */
	NULL,           /* deactivate_func_t */
	NULL,           /* message_handler_func_t */
	NULL,           /* op_array_handler_func_t */
	xdebug_statement_call, /* statement_handler_func_t */
	NULL,           /* fcall_begin_handler_func_t */
	NULL,           /* fcall_end_handler_func_t */
	xdebug_init_oparray,   /* op_array_ctor_func_t */
	NULL,           /* op_array_dtor_func_t */
	STANDARD_ZEND_EXTENSION_PROPERTIES
};

#endif
