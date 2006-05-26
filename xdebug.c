/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003, 2004, 2005, 2006 Derick Rethans            |
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
#include "ext/standard/php_smart_str.h"
#include "php_globals.h"
#include "ext/standard/php_var.h"

#include "zend.h"
#include "zend_API.h"
#include "zend_execute.h"
#include "zend_compile.h"
#include "zend_constants.h"
#include "zend_extensions.h"
#ifdef ZEND_ENGINE_2
# include "zend_exceptions.h"
# if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1) || PHP_MAJOR_VERSION >= 6
#  include "zend_vm.h"
# endif
#endif

#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_code_coverage.h"
#include "xdebug_com.h"
#include "xdebug_llist.h"
#include "xdebug_mm.h"
#include "xdebug_var.h"
#include "xdebug_profiler.h"
#include "xdebug_superglobals.h"
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

#ifdef ZEND_ENGINE_2
void xdebug_throw_exception_hook(zval *exception TSRMLS_DC);
#endif

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 0)
int (*old_exit_handler)(ZEND_OPCODE_HANDLER_ARGS);

static int (*old_jmp_handler)(ZEND_OPCODE_HANDLER_ARGS);
static int (*old_jmpz_handler)(ZEND_OPCODE_HANDLER_ARGS);
static int (*old_is_identical_handler)(ZEND_OPCODE_HANDLER_ARGS);
static int (*old_is_not_identical_handler)(ZEND_OPCODE_HANDLER_ARGS);
static int (*old_is_equal_handler)(ZEND_OPCODE_HANDLER_ARGS);
static int (*old_is_not_equal_handler)(ZEND_OPCODE_HANDLER_ARGS);
static int (*old_is_smaller_handler)(ZEND_OPCODE_HANDLER_ARGS);
static int (*old_is_smaller_or_equal_handler)(ZEND_OPCODE_HANDLER_ARGS);

static int xdebug_jmp_handler(ZEND_OPCODE_HANDLER_ARGS);
static int xdebug_jmpz_handler(ZEND_OPCODE_HANDLER_ARGS);
static int xdebug_is_identical_handler(ZEND_OPCODE_HANDLER_ARGS);
static int xdebug_is_not_identical_handler(ZEND_OPCODE_HANDLER_ARGS);
static int xdebug_is_equal_handler(ZEND_OPCODE_HANDLER_ARGS);
static int xdebug_is_not_equal_handler(ZEND_OPCODE_HANDLER_ARGS);
static int xdebug_is_smaller_handler(ZEND_OPCODE_HANDLER_ARGS);
static int xdebug_is_smaller_or_equal_handler(ZEND_OPCODE_HANDLER_ARGS);
#endif

#ifdef ZEND_ENGINE_2
int xdebug_exit_handler(ZEND_OPCODE_HANDLER_ARGS);
#endif

static zval *get_zval(zend_execute_data *zdata, znode *node, temp_variable *Ts, int *is_var);
static char* return_trace_stack_frame_begin(function_stack_entry* i, int fnr TSRMLS_DC);
static char* return_trace_stack_frame_end(function_stack_entry* i, int fnr TSRMLS_DC);
static char* return_trace_stack_retval(function_stack_entry* i, zval* retval TSRMLS_DC);

int zend_xdebug_initialised = 0;

function_entry xdebug_functions[] = {
	PHP_FE(xdebug_get_stack_depth,       NULL)
	PHP_FE(xdebug_get_function_stack,    NULL)
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

#if MEMORY_LIMIT
	PHP_FE(xdebug_memory_usage,          NULL)
	PHP_FE(xdebug_peak_memory_usage,     NULL)
#endif
	PHP_FE(xdebug_time_index,            NULL)

	PHP_FE(xdebug_start_code_coverage,   NULL)
	PHP_FE(xdebug_stop_code_coverage,    NULL)
	PHP_FE(xdebug_get_code_coverage,     NULL)
	PHP_FE(xdebug_get_function_count,    NULL)

	PHP_FE(xdebug_dump_superglobals,     NULL)
	{NULL, NULL, NULL}
};

zend_module_entry xdebug_module_entry = {
	STANDARD_MODULE_HEADER,
	"xdebug",
	xdebug_functions,
	PHP_MINIT(xdebug),
	PHP_MSHUTDOWN(xdebug),
	PHP_RINIT(xdebug),
#ifndef ZEND_ENGINE_2
	PHP_RSHUTDOWN(xdebug),
#else
	NULL,
#endif
	PHP_MINFO(xdebug),
	XDEBUG_VERSION,
#ifdef ZEND_ENGINE_2
	ZEND_MODULE_POST_ZEND_DEACTIVATE_N(xdebug),
#else
	NULL,
	NULL,
#endif
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

static PHP_INI_MH(OnUpdateAllowedClients)
{
	return SUCCESS;
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

PHP_INI_BEGIN()
	/* Debugger settings */
	STD_PHP_INI_BOOLEAN("xdebug.auto_trace",      "0",                  PHP_INI_ALL,    OnUpdateBool,   auto_trace,        zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_output_dir",  "/tmp",               PHP_INI_ALL,    OnUpdateString, trace_output_dir,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_output_name", "crc32",              PHP_INI_ALL,    OnUpdateString, trace_output_name, zend_xdebug_globals, xdebug_globals)
#if ZEND_EXTENSION_API_NO < 90000000
	STD_PHP_INI_ENTRY("xdebug.trace_format",      "0",                  PHP_INI_ALL,    OnUpdateInt,    trace_format,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_options",     "0",                  PHP_INI_ALL,    OnUpdateInt,    trace_options,     zend_xdebug_globals, xdebug_globals)
#else
	STD_PHP_INI_ENTRY("xdebug.trace_format",      "0",                  PHP_INI_ALL,    OnUpdateLong,   trace_format,      zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.trace_options",     "0",                  PHP_INI_ALL,    OnUpdateLong,   trace_options,     zend_xdebug_globals, xdebug_globals)
#endif
	STD_PHP_INI_BOOLEAN("xdebug.collect_includes","1",                  PHP_INI_ALL,    OnUpdateBool,   collect_includes,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_params",  "0",                  PHP_INI_ALL,    OnUpdateBool,   collect_params,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.collect_return",  "0",                  PHP_INI_ALL,    OnUpdateBool,   collect_return,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.default_enable",  "1",                  PHP_INI_ALL,    OnUpdateBool,   default_enable,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.extended_info",   "1",                  PHP_INI_SYSTEM, OnUpdateBool,   extended_info,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.manual_url",        "http://www.php.net", PHP_INI_ALL,    OnUpdateString, manual_url,        zend_xdebug_globals, xdebug_globals)
#if ZEND_EXTENSION_API_NO < 90000000
	STD_PHP_INI_ENTRY("xdebug.max_nesting_level", "100",                PHP_INI_ALL,    OnUpdateInt,    max_nesting_level, zend_xdebug_globals, xdebug_globals)
#else
	STD_PHP_INI_ENTRY("xdebug.max_nesting_level", "100",                PHP_INI_ALL,    OnUpdateLong,   max_nesting_level, zend_xdebug_globals, xdebug_globals)
#endif
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
	STD_PHP_INI_ENTRY("xdebug.profiler_output_dir",       "/tmp",   PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, profiler_output_dir,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.profiler_output_name",      "crc32",  PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateString, profiler_output_name,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_enable_trigger", "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler_enable_trigger, zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_append",         "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler_append,         zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_BOOLEAN("xdebug.profiler_aggregate",      "0",      PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   profiler_aggregate,      zend_xdebug_globals, xdebug_globals)

	/* Remote debugger settings */
	STD_PHP_INI_BOOLEAN("xdebug.remote_enable",   "0",   PHP_INI_SYSTEM|PHP_INI_PERDIR, OnUpdateBool,   remote_enable,     zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_handler",    "dbgp",               PHP_INI_ALL,    OnUpdateString, remote_handler,    zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_host",       "localhost",          PHP_INI_ALL,    OnUpdateString, remote_host,       zend_xdebug_globals, xdebug_globals)
	PHP_INI_ENTRY("xdebug.remote_mode",           "req",                PHP_INI_ALL,    OnUpdateDebugMode)
#if ZEND_EXTENSION_API_NO < 90000000
	STD_PHP_INI_ENTRY("xdebug.remote_port",       "9000",               PHP_INI_ALL,    OnUpdateInt,    remote_port,       zend_xdebug_globals, xdebug_globals)
#else
	STD_PHP_INI_ENTRY("xdebug.remote_port",       "9000",               PHP_INI_ALL,    OnUpdateLong,   remote_port,       zend_xdebug_globals, xdebug_globals)
#endif
	STD_PHP_INI_BOOLEAN("xdebug.remote_autostart","0",                  PHP_INI_ALL,    OnUpdateBool,   remote_autostart,  zend_xdebug_globals, xdebug_globals)
	STD_PHP_INI_ENTRY("xdebug.remote_log",        "",                   PHP_INI_ALL,    OnUpdateString, remote_log,        zend_xdebug_globals, xdebug_globals)
	PHP_INI_ENTRY("xdebug.allowed_clients",       "",                   PHP_INI_SYSTEM, OnUpdateAllowedClients)
	PHP_INI_ENTRY("xdebug.idekey",                "",                   PHP_INI_ALL,    OnUpdateIDEKey)
PHP_INI_END()

static void php_xdebug_init_globals (zend_xdebug_globals *xg TSRMLS_DC)
{
	xg->stack                = NULL;
	xg->level                = 0;
	xg->do_trace             = 0;
	xg->trace_file           = NULL;
	xg->do_code_coverage     = 0;
	xg->error_handler        = NULL;
	xg->breakpoint_count     = 0;
	xg->ide_key              = NULL;

	xdebug_llist_init(&xg->server, dump_dtor);
	xdebug_llist_init(&xg->get, dump_dtor);
	xdebug_llist_init(&xg->post, dump_dtor);
	xdebug_llist_init(&xg->cookie, dump_dtor);
	xdebug_llist_init(&xg->files, dump_dtor);
	xdebug_llist_init(&xg->env, dump_dtor);
	xdebug_llist_init(&xg->request, dump_dtor);
	xdebug_llist_init(&xg->session, dump_dtor);
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
			name = "xdebug.profile_enable";
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
		}

		if (name) {
			zend_alter_ini_entry(name, strlen(name) + 1, envval, strlen(envval), PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
		}
	}

	xdebug_arg_dtor(parts);
}

#ifdef ZEND_ENGINE_2
/* Needed for code coverage as Zend doesn't always add EXT_STMT when expected */
# if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1) || PHP_MAJOR_VERSION >= 6
#define XDEBUG_OPCODE_OVERRIDE(f)  static int xdebug_##f##_handler(ZEND_OPCODE_HANDLER_ARGS) \
{ \
	if (XG(do_code_coverage)) { \
		zend_op *cur_opcode; \
		int      lineno; \
		char    *file; \
		int      file_len; \
		zend_op_array *op_array = execute_data->op_array; \
\
		cur_opcode = *EG(opline_ptr); \
		lineno = cur_opcode->lineno; \
\
		file = op_array->filename; \
		file_len = strlen(file); \
\
		xdebug_count_line(file, lineno, 0 TSRMLS_CC); \
	} \
	return ZEND_USER_OPCODE_DISPATCH; \
}
#else
#define XDEBUG_OPCODE_OVERRIDE(f)  static int xdebug_##f##_handler(ZEND_OPCODE_HANDLER_ARGS) \
{ \
	if (XG(do_code_coverage)) { \
		zend_op *cur_opcode; \
		int      lineno; \
		char    *file; \
		int      file_len; \
\
		cur_opcode = *EG(opline_ptr); \
		lineno = cur_opcode->lineno; \
\
		file = op_array->filename; \
		file_len = strlen(file); \
\
		xdebug_count_line(file, lineno, 0 TSRMLS_CC); \
	} \
	return old_##f##_handler(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU); \
}
#endif
XDEBUG_OPCODE_OVERRIDE(jmp)
XDEBUG_OPCODE_OVERRIDE(jmpz)
XDEBUG_OPCODE_OVERRIDE(is_identical)
XDEBUG_OPCODE_OVERRIDE(is_not_identical)
XDEBUG_OPCODE_OVERRIDE(is_equal)
XDEBUG_OPCODE_OVERRIDE(is_not_equal)
XDEBUG_OPCODE_OVERRIDE(is_smaller)
XDEBUG_OPCODE_OVERRIDE(is_smaller_or_equal)
#endif


PHP_MINIT_FUNCTION(xdebug)
{
	ZEND_INIT_MODULE_GLOBALS(xdebug, php_xdebug_init_globals, php_xdebug_shutdown_globals);
	REGISTER_INI_ENTRIES();
#if 0
#ifdef ZEND_ENGINE_2
# if PHP_MINOR_VERSION >= 1
	zend_vm_use_old_executor();
# endif
#endif
#endif
	/* get xdebug ini entries from the environment also */
	xdebug_env_config();

	/* initialize aggregate call information hash */
	zend_hash_init_ex(&XG(aggr_calls), 50, NULL, (dtor_func_t) profile_aggr_call_entry_dtor, 1, 0);

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

	/* Overload the "exit" opcode */
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 0)
	old_exit_handler = zend_opcode_handlers[ZEND_EXIT];
	zend_opcode_handlers[ZEND_EXIT] = xdebug_exit_handler;

	old_jmp_handler = zend_opcode_handlers[ZEND_JMP];
	old_jmpz_handler = zend_opcode_handlers[ZEND_JMPZ];
	old_is_identical_handler = zend_opcode_handlers[ZEND_IS_IDENTICAL];
	old_is_not_identical_handler = zend_opcode_handlers[ZEND_IS_NOT_IDENTICAL];
	old_is_equal_handler = zend_opcode_handlers[ZEND_IS_EQUAL];
	old_is_not_equal_handler = zend_opcode_handlers[ZEND_IS_NOT_EQUAL];
	old_is_smaller_handler = zend_opcode_handlers[ZEND_IS_SMALLER];
	old_is_smaller_or_equal_handler = zend_opcode_handlers[ZEND_IS_SMALLER_OR_EQUAL];

	zend_opcode_handlers[ZEND_JMP] = xdebug_jmp_handler;
	zend_opcode_handlers[ZEND_JMPZ] = xdebug_jmpz_handler;
	zend_opcode_handlers[ZEND_IS_IDENTICAL] = xdebug_is_identical_handler;
	zend_opcode_handlers[ZEND_IS_NOT_IDENTICAL] = xdebug_is_not_identical_handler;
	zend_opcode_handlers[ZEND_IS_EQUAL] = xdebug_is_equal_handler;
	zend_opcode_handlers[ZEND_IS_NOT_EQUAL] = xdebug_is_not_equal_handler;
	zend_opcode_handlers[ZEND_IS_SMALLER] = xdebug_is_smaller_handler;
	zend_opcode_handlers[ZEND_IS_SMALLER_OR_EQUAL] = xdebug_is_smaller_or_equal_handler;
#endif
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1) || PHP_MAJOR_VERSION >= 6
	zend_set_user_opcode_handler(ZEND_EXIT, xdebug_exit_handler);
	zend_set_user_opcode_handler(ZEND_JMP,  xdebug_jmp_handler);
	zend_set_user_opcode_handler(ZEND_JMPZ, xdebug_jmpz_handler);
	zend_set_user_opcode_handler(ZEND_IS_IDENTICAL, xdebug_is_identical_handler);
	zend_set_user_opcode_handler(ZEND_IS_NOT_IDENTICAL, xdebug_is_not_identical_handler);
	zend_set_user_opcode_handler(ZEND_IS_EQUAL, xdebug_is_equal_handler);
	zend_set_user_opcode_handler(ZEND_IS_NOT_EQUAL, xdebug_is_not_equal_handler);
	zend_set_user_opcode_handler(ZEND_IS_SMALLER, xdebug_is_smaller_handler);
	zend_set_user_opcode_handler(ZEND_IS_SMALLER_OR_EQUAL, xdebug_is_smaller_or_equal_handler);
#endif

	if (zend_xdebug_initialised == 0) {
		zend_error(E_WARNING, "Xdebug MUST be loaded as a Zend extension");
	}

	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_APPEND", XDEBUG_TRACE_OPTION_APPEND, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_TRACE_COMPUTERIZED", XDEBUG_TRACE_OPTION_COMPUTERIZED, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("XDEBUG_CC_UNUSED", XDEBUG_CC_OPTION_UNUSED, CONST_CS | CONST_PERSISTENT);

	XG(breakpoint_count) = 0;
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

void used_var_dtor (void *elem)
{
	char *s = elem;

	if (s) {
		xdfree(s);
	}
}

void stack_element_dtor (void *dummy, void *elem)
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
			xdebug_hash_destroy(e->used_vars);
		}

		if (e->profile.call_list) {
			xdebug_llist_destroy(e->profile.call_list, NULL);
		}

		xdfree(e);
	}
}

PHP_RINIT_FUNCTION(xdebug)
{
	zend_function *orig;
	char *idekey = zend_ini_string("xdebug.idekey", sizeof("xdebug.idekey"), 0);
	
	XG(level)         = 0;
	XG(do_trace)      = 0;
	XG(do_code_coverage) = 0;
	XG(code_coverage) = xdebug_hash_alloc(32, xdebug_coverage_file_dtor);
	XG(code_coverage_op_array_cache) = xdebug_hash_alloc(1024, NULL);
	XG(stack)         = xdebug_llist_alloc(stack_element_dtor);
	XG(trace_file)    = NULL;
	XG(tracefile_name) = NULL;
	XG(profile_file)  = NULL;
	XG(profile_filename) = NULL;
	XG(error_handler) = NULL;
	XG(prev_memory)   = 0;
	XG(function_count) = 0;
	XG(active_symbol_table) = NULL;
	
	if (idekey && *idekey) {
		if (XG(ide_key)) {
			xdfree(XG(ide_key));
		}
		XG(ide_key) = xdstrdup(idekey);
	}

	/* Only enabled extended info when it is not disabled */
	CG(extended_info) = XG(extended_info);

	if (XG(default_enable)) {
		zend_error_cb = new_error_cb;

#ifdef ZEND_ENGINE_2
		zend_throw_exception_hook = xdebug_throw_exception_hook;
#endif
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
	zend_hash_find(EG(function_table), "var_dump", 9, (void **)&orig);
	XG(orig_var_dump_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_xdebug_var_dump;

	/* Override set_time_limit with our own function to prevent timing out while debugging */
	zend_hash_find(EG(function_table), "set_time_limit", 15, (void **)&orig);
	XG(orig_set_time_limit_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_xdebug_set_time_limit;

	return SUCCESS;
}

#ifdef ZEND_ENGINE_2
ZEND_MODULE_POST_ZEND_DEACTIVATE_D(xdebug)
{
	TSRMLS_FETCH();
#else
PHP_RSHUTDOWN_FUNCTION(xdebug)
{
#endif
	zend_function *orig;

	xdebug_llist_destroy(XG(stack), NULL);
	XG(stack) = NULL;

	if (XG(do_trace) && XG(trace_file)) {
		xdebug_stop_trace(TSRMLS_C);
	}

	if (XG(profile_file)) {
		fclose(XG(profile_file));
	}

	if (XG(profile_filename)) {
		efree(XG(profile_filename));
	}

	if (XG(error_handler)) {
		efree(XG(error_handler));
	}

	if (XG(ide_key)) {
		xdfree(XG(ide_key));
		XG(ide_key) = NULL;
	}

	XG(level)            = 0;
	XG(do_trace)         = 0;
	XG(do_code_coverage) = 0;

	xdebug_hash_destroy(XG(code_coverage));
	xdebug_hash_destroy(XG(code_coverage_op_array_cache));

	if (XG(remote_enabled)) {
		XG(context).handler->remote_deinit(&(XG(context)));
		xdebug_close_socket(XG(context).socket); 
		if (XG(context).program_name) {
			xdfree(XG(context).program_name);
		}
	}

	if (XG(context.list.last_file)) {
		xdfree(XG(context).list.last_file);
	}

	/* Reset var_dump and set_time_limit to the original function */
	zend_hash_find(EG(function_table), "var_dump", 9, (void **)&orig);
	orig->internal_function.handler = XG(orig_var_dump_func);
	zend_hash_find(EG(function_table), "set_time_limit", 15, (void **)&orig);
	orig->internal_function.handler = XG(orig_set_time_limit_func);

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

void xdebug_build_fname(xdebug_func *tmp, zend_execute_data *edata TSRMLS_DC)
{
	memset(tmp, 0, sizeof(xdebug_func));

	if (edata) {
		if (edata->function_state.function->common.function_name) {
#if ZEND_EXTENSION_API_NO < 90000000
			if (edata->ce) {
				tmp->type = XFUNC_STATIC_MEMBER;
				tmp->class = xdstrdup(edata->ce->name);
			} else if (edata->object.ptr) {
				tmp->type = XFUNC_MEMBER;
				tmp->class = xdstrdup(edata->object.ptr->value.obj.ce->name);
			} else {
				tmp->type = XFUNC_NORMAL;
			}
#else
			if (edata->object) {
				tmp->type = XFUNC_MEMBER;
				if (edata->function_state.function->common.scope) { /* __autoload has no scope */
					tmp->class = xdstrdup(edata->function_state.function->common.scope->name);
				}
			} else if (EG(scope) && edata->function_state.function->common.scope && edata->function_state.function->common.scope->name) {
				tmp->type = XFUNC_STATIC_MEMBER;
				tmp->class = xdstrdup(edata->function_state.function->common.scope->name);
			} else {
				tmp->type = XFUNC_NORMAL;
			}
#endif
			tmp->function = xdstrdup(edata->function_state.function->common.function_name);
		} else {
			switch (edata->opline->op2.u.constant.value.lval) {
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
		}
	}
}

static void trace_function_begin(function_stack_entry *fse, int function_nr TSRMLS_DC)
{
	if (XG(do_trace) && XG(trace_file)) {
		char *t = return_trace_stack_frame_begin(fse, function_nr TSRMLS_CC);
		if (fprintf(XG(trace_file), "%s", t) < 0) {
			fclose(XG(trace_file));
			XG(trace_file) = NULL;
		} else {
			fflush(XG(trace_file));
		}
		xdfree(t);
	}
}

static void trace_function_end(function_stack_entry *fse, int function_nr TSRMLS_DC)
{
	if (XG(do_trace) && XG(trace_file)) {
		char *t = return_trace_stack_frame_end(fse, function_nr TSRMLS_CC);
		if (fprintf(XG(trace_file), "%s", t) < 0) {
			fclose(XG(trace_file));
			XG(trace_file) = NULL;
		} else {
			fflush(XG(trace_file));
		}
		xdfree(t);
	}
}

static function_stack_entry *add_stack_frame(zend_execute_data *zdata, zend_op_array *op_array, int type TSRMLS_DC)
{
	function_stack_entry *tmp;
	zend_op              *cur_opcode;
	zval                **param;
	void                **p;
	int                   arg_count = 0;
	int                   i = 0;
	char                 *aggr_key;
	int                   aggr_key_len;

	if (EG(argument_stack).top >= 2) {
		p = EG(argument_stack).top_element - 2;
		arg_count = (ulong) *p;
	}

	tmp = xdmalloc (sizeof (function_stack_entry));
	tmp->var           = NULL;
	tmp->varc          = 0;
	tmp->refcount      = 1;
	tmp->level         = XG(level);
	tmp->arg_done      = 0;
	tmp->used_vars     = NULL;
	tmp->user_defined  = type;
	tmp->filename      = NULL;
	tmp->include_filename  = NULL;
	tmp->profile.call_list = xdebug_llist_alloc(profile_call_entry_dtor);
	tmp->op_array      = op_array;
	tmp->symbol_table  = NULL;

	if (EG(current_execute_data) && EG(current_execute_data)->op_array) {
		/* Normal function calls */
		tmp->filename  = xdstrdup(EG(current_execute_data)->op_array->filename);
		XG(function_count)++;
	} else if (EG(current_execute_data) &&
		EG(current_execute_data)->prev_execute_data &&
		XDEBUG_LLIST_TAIL(XG(stack))
	) {
		/* Ugly hack for call_user_*() type function calls */
		zend_function *tmpf = EG(current_execute_data)->prev_execute_data->function_state.function;
		if (tmpf && (tmpf->common.type != 3) && tmpf->common.function_name) {
			if (
				(strcmp(tmpf->common.function_name, "call_user_func") == 0) ||
				(strcmp(tmpf->common.function_name, "call_user_func_array") == 0) ||
				(strcmp(tmpf->common.function_name, "call_user_func_method") == 0) ||
				(strcmp(tmpf->common.function_name, "call_user_func_method_array") == 0)
			) {
				tmp->filename = xdstrdup(((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))))->filename);
			}
		}
		XG(function_count)++;
	}
	if (!tmp->filename) {
		/* Includes/main script etc */
		tmp->filename  = (op_array && op_array->filename) ? xdstrdup(op_array->filename): NULL;
	}
	/* Call user function locations */
	if (!tmp->filename && XDEBUG_LLIST_TAIL(XG(stack)) && XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))) ) {
		tmp->filename = xdstrdup(((function_stack_entry*) XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack))))->filename);
	}
#if MEMORY_LIMIT
	tmp->prev_memory = XG(prev_memory);
	tmp->memory = AG(allocated_memory);
	XG(prev_memory) = tmp->memory;
#else
	tmp->memory = 0;
	tmp->prev_memory = 0;
#endif
	tmp->time   = xdebug_get_utime();
	tmp->lineno = 0;

	xdebug_build_fname(&(tmp->function), zdata TSRMLS_CC);
	if (!tmp->function.type) {
		tmp->function.function = xdstrdup("{main}");
		tmp->function.class    = NULL;
		tmp->function.type     = XFUNC_NORMAL;

	} else if (tmp->function.type & XFUNC_INCLUDES) {
		cur_opcode = *EG(opline_ptr);
		tmp->lineno = cur_opcode->lineno;

#if (PHP_MAJOR_VERSION == 6) || \
	(PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1) || \
	(PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 0 && PHP_RELEASE_VERSION > 5) || \
	(PHP_MAJOR_VERSION == 4 && PHP_MINOR_VERSION == 4 && PHP_RELEASE_VERSION > 0)
		if (tmp->function.type == XFUNC_EVAL) {
			int   is_var;

			tmp->include_filename = get_zval_value(get_zval(zdata, &zdata->opline->op1, zdata->Ts, &is_var), 0);
		} else if (XG(collect_includes)) {
			tmp->include_filename = xdstrdup(zend_get_executed_filename(TSRMLS_C));
		}
#endif
	} else  {
		if (EG(current_execute_data)->opline) {
			cur_opcode = EG(current_execute_data)->opline;
			if (cur_opcode) {
				tmp->lineno = cur_opcode->lineno;
			}
		}
		if (XG(collect_params)) {
			tmp->var = xdmalloc(arg_count * sizeof (xdebug_var));
			for (i = 0; i < arg_count; i++) {
				tmp->var[tmp->varc].name  = NULL;
				param = NULL;
				if (zend_ptr_stack_get_arg(tmp->varc + 1, (void**) &param TSRMLS_CC) == SUCCESS) {
					if (param) {
						tmp->var[tmp->varc].addr = *param;
					} else {
						tmp->var[tmp->varc].addr = NULL;
					}
				} else {
					tmp->var[tmp->varc].addr = NULL;
				}
				tmp->varc++;
			}
		}
	}

	if (XG(do_code_coverage)) {
		xdebug_count_line(tmp->filename, tmp->lineno, 0 TSRMLS_CC);
	}

	if (XG(profiler_aggregate)) {
		char *func_name = show_fname(tmp->function, 0, 0 TSRMLS_CC);

		aggr_key = xdebug_sprintf("%s.%s.%d", tmp->filename, func_name, tmp->lineno);
		aggr_key_len = strlen(aggr_key);

		if (zend_hash_find(&XG(aggr_calls), aggr_key, aggr_key_len+1, (void**)&tmp->aggr_entry) == FAILURE) {
			xdebug_aggregate_entry xae;

			if (tmp->user_defined == XDEBUG_EXTERNAL) {
				xae.filename = xdstrdup(tmp->op_array->filename);
			} else {
				xae.filename = xdstrdup("php:internal");
			}
			xae.function = func_name;
			xae.lineno = tmp->lineno;
			xae.user_defined = tmp->user_defined;
			xae.call_count = 0;
			xae.time_own = 0;
			xae.time_inclusive = 0;
#if MEMORY_LIMIT
			xae.mem_used = 0;
#endif
			xae.call_list = NULL;

			zend_hash_add(&XG(aggr_calls), aggr_key, aggr_key_len+1, (void*)&xae, sizeof(xdebug_aggregate_entry), (void**)&tmp->aggr_entry);
		}
	}

	if (XDEBUG_LLIST_TAIL(XG(stack))) {
		function_stack_entry *prev = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack)));
		tmp->prev = prev;
		if (XG(profiler_aggregate)) {
			if (prev->aggr_entry->call_list) {
				if (!zend_hash_exists(prev->aggr_entry->call_list, aggr_key, aggr_key_len+1)) {
					zend_hash_add(prev->aggr_entry->call_list, aggr_key, aggr_key_len+1, (void*)&tmp->aggr_entry, sizeof(xdebug_aggregate_entry*), NULL);
				}
			} else {
				prev->aggr_entry->call_list = xdmalloc(sizeof(HashTable));
				zend_hash_init_ex(prev->aggr_entry->call_list, 1, NULL, NULL, 1, 0);
				zend_hash_add(prev->aggr_entry->call_list, aggr_key, aggr_key_len+1, (void*)&tmp->aggr_entry, sizeof(xdebug_aggregate_entry*), NULL);
			}
		}
	} else {
		tmp->prev = 0;
	}
	xdebug_llist_insert_next(XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), tmp);

	if (XG(profiler_aggregate)) {
		xdfree(aggr_key);
	}

	return tmp;
}

static void add_used_variables(function_stack_entry *fse, zend_op_array *op_array)
{
	int i = 0; 
	int j = op_array->size;

	if (!fse->used_vars) {
		fse->used_vars = xdebug_hash_alloc(64, used_var_dtor);
	}

	while (i < j) {
# if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1) || PHP_MAJOR_VERSION >= 6
		char *cv = NULL;
		int cv_len;

		if (op_array->opcodes[i].op1.op_type == IS_CV) {
			cv = zend_get_compiled_variable_name(op_array, op_array->opcodes[i].op1.u.var, &cv_len);
			xdebug_hash_update(fse->used_vars, cv, cv_len, xdstrdup(cv));
		}
		if (op_array->opcodes[i].op2.op_type == IS_CV) {
			cv = zend_get_compiled_variable_name(op_array, op_array->opcodes[i].op2.u.var, &cv_len);
			xdebug_hash_update(fse->used_vars, cv, cv_len, xdstrdup(cv));
		}
		/* FIXME: Add support for function arguments too */
#else
		if (op_array->opcodes[i].opcode == ZEND_FETCH_R || op_array->opcodes[i].opcode == ZEND_FETCH_W) {
			if (op_array->opcodes[i].op1.op_type == IS_CONST) {
				if (Z_TYPE(op_array->opcodes[i].op1.u.constant) == IS_STRING) {
					xdebug_hash_update(
						fse->used_vars, 
						op_array->opcodes[i].op1.u.constant.value.str.val,
						op_array->opcodes[i].op1.u.constant.value.str.len,
						xdstrdup(op_array->opcodes[i].op1.u.constant.value.str.val)
					);
				} else { /* unusual but not impossible situation */
					int use_copy;
					zval tmp_zval;

					zend_make_printable_zval(&(op_array->opcodes[i].op1.u.constant), &tmp_zval, &use_copy);

					xdebug_hash_update(fse->used_vars, tmp_zval.value.str.val, tmp_zval.value.str.len, xdstrdup(tmp_zval.value.str.val));

					zval_dtor(&tmp_zval);
				}
			}
		}
#endif
		i++;
	}
}

static int handle_hit_value(xdebug_brk_info *brk_info)
{
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
				if (handle_hit_value(extra_brk_info)) {
					if (fse->user_defined == XDEBUG_INTERNAL || (breakpoint_type == XDEBUG_BRK_FUNC_RETURN)) {
						if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), fse->filename, fse->lineno, XDEBUG_BREAK)) {
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
		tmp_name = xdebug_sprintf("%s::%s", fse->function.class, fse->function.function);

		if (xdebug_hash_find(XG(context).class_breakpoints, tmp_name, strlen(tmp_name), (void *) &extra_brk_info)) {
			/* Yup, breakpoint found, call handler if the breakpoint is not
			 * disabled AND handle_hit_value is happy */
			if (!extra_brk_info->disabled) {
				if (handle_hit_value(extra_brk_info)) {
					XG(context).do_break = 1;
				}
			}
		}
		xdfree(tmp_name);
	}
	return 1;
}

#if PHP_API_VERSION >= 20030820
#define COOKIE_ENCODE , 1
#else
#define COOKIE_ENCODE
#endif

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

	if (XG(level) == 0) {
		/* Set session cookie if requested */
		if (
			((
				PG(http_globals)[TRACK_VARS_GET] &&
				zend_hash_find(PG(http_globals)[TRACK_VARS_GET]->value.ht, "XDEBUG_SESSION_START", sizeof("XDEBUG_SESSION_START"), (void **) &dummy) == SUCCESS
			) || (
				PG(http_globals)[TRACK_VARS_POST] &&
				zend_hash_find(PG(http_globals)[TRACK_VARS_POST]->value.ht, "XDEBUG_SESSION_START", sizeof("XDEBUG_SESSION_START"), (void **) &dummy) == SUCCESS
			)) && !(
				PG(http_globals)[TRACK_VARS_COOKIE] &&
				zend_hash_find(PG(http_globals)[TRACK_VARS_COOKIE]->value.ht, "XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), (void **) &dummy) == SUCCESS
			)
			&& !SG(headers_sent)
		) {
			convert_to_string_ex(dummy);
			magic_cookie = xdstrdup(Z_STRVAL_PP(dummy));
			if (XG(ide_key)) {
				xdfree(XG(ide_key));
			}
			XG(ide_key) = xdstrdup(Z_STRVAL_PP(dummy));
			php_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), Z_STRVAL_PP(dummy), Z_STRLEN_PP(dummy), time(NULL) + 3600, "/", 1, NULL, 0, 0 COOKIE_ENCODE TSRMLS_CC);
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
				php_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), XG(ide_key), strlen(XG(ide_key)), time(NULL) + 3600, "/", 1, NULL, 0, 0 COOKIE_ENCODE TSRMLS_CC);
			}
		}
			

		/* Remove session cookie if requested */
		if (
			(
				PG(http_globals)[TRACK_VARS_GET] &&
				zend_hash_find(PG(http_globals)[TRACK_VARS_GET]->value.ht, "XDEBUG_SESSION_STOP", sizeof("XDEBUG_SESSION_STOP"), (void **) &dummy) == SUCCESS
			) || (
				PG(http_globals)[TRACK_VARS_POST] &&
				zend_hash_find(PG(http_globals)[TRACK_VARS_POST]->value.ht, "XDEBUG_SESSION_STOP", sizeof("XDEBUG_SESSION_STOP"), (void **) &dummy) == SUCCESS
			)
			&& !SG(headers_sent)
		) {
			php_setcookie("XDEBUG_SESSION", sizeof("XDEBUG_SESSION"), "", 0, time(NULL) + 3600, "/", 1, NULL, 0, 0 COOKIE_ENCODE TSRMLS_CC);
		}

		/* Start remote context if requested */
		if (
			(magic_cookie || XG(remote_autostart)) &&
			!XG(remote_enabled) &&
			XG(remote_enable) &&
			(XG(remote_mode) == XDEBUG_REQ)
		) {
			/* Initialize debugging session */
			XG(context).socket = xdebug_create_socket(XG(remote_host), XG(remote_port));
			if (XG(context).socket >= 0) {
				XG(remote_enabled) = 1;

				/* Get handler from mode */
				XG(context).handler = xdebug_handler_get(XG(remote_handler));
				XG(context).program_name = xdstrdup(op_array->filename);
				if (!XG(context).handler->remote_init(&(XG(context)), XDEBUG_REQ)) {
					XG(remote_enabled) = 0;
				}

				/* Turn off script time outs */
				zend_alter_ini_entry("max_execution_time", sizeof("max_execution_time"), "0", strlen("0"), PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
			}
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
						PG(http_globals)[TRACK_VARS_GET] && 
						zend_hash_find(PG(http_globals)[TRACK_VARS_GET]->value.ht, "XDEBUG_PROFILE", sizeof("XDEBUG_PROFILE"), (void **) &dummy) == SUCCESS
					) || (
						PG(http_globals)[TRACK_VARS_POST] && 
						zend_hash_find(PG(http_globals)[TRACK_VARS_POST]->value.ht, "XDEBUG_PROFILE", sizeof("XDEBUG_PROFILE"), (void **) &dummy) == SUCCESS
					)
				)
			)
		) {
			if (xdebug_profiler_init(op_array->filename TSRMLS_CC) == SUCCESS) {
				XG(profiler_enabled) = 1;
			}
		}
		xdfree(magic_cookie);
		magic_cookie = NULL;
	}

	XG(level)++;
	if (XG(level) == XG(max_nesting_level)) {
		php_error(E_ERROR, "Maximum function nesting level of '%ld' reached, aborting!", XG(max_nesting_level));
	}

	fse = add_stack_frame(edata, op_array, XDEBUG_EXTERNAL TSRMLS_CC);

	function_nr = XG(function_count);
	trace_function_begin(fse, function_nr TSRMLS_CC);

	fse->symbol_table = EG(active_symbol_table);

	/* Because include/require is treated as a stack level, we have to add used
	 * variables in include/required files to all the stack levels above, until
	 * we hit a function or the to level stack.  This is so that the variables
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

	if (XG(do_code_coverage) && XG(code_coverage_unused)) {
		xdebug_prefil_code_coverage(fse, op_array TSRMLS_CC);
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

	trace_function_end(fse, function_nr TSRMLS_CC);

	if (XG(collect_return) && do_return && XG(do_trace) && XG(trace_file)) {
		if (EG(return_value_ptr_ptr) && *EG(return_value_ptr_ptr)) {
			char* t = return_trace_stack_retval(fse, *EG(return_value_ptr_ptr) TSRMLS_CC);
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
	xdebug_llist_remove(XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), stack_element_dtor);
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

	fse = add_stack_frame(edata, edata->op_array, XDEBUG_INTERNAL TSRMLS_CC);

	function_nr = XG(function_count);
	trace_function_begin(fse, function_nr TSRMLS_CC);

	/* Check for entry breakpoints */
	if (XG(remote_enabled) && XG(breakpoints_allowed)) {
		if (!handle_breakpoints(fse, XDEBUG_BRK_FUNC_CALL)) {
			XG(remote_enabled) = 0;
		}
	}
	
	if (XG(profiler_enabled)) {
		xdebug_profiler_function_internal_begin(fse TSRMLS_CC);
	}
	execute_internal(current_execute_data, return_value_used TSRMLS_CC);

	if (XG(profiler_enabled)) {
		xdebug_profiler_function_internal_end(fse TSRMLS_CC);
	}

	trace_function_end(fse, function_nr TSRMLS_CC);

	if (XG(collect_return) && do_return && XG(do_trace) && XG(trace_file)) {
		cur_opcode = *EG(opline_ptr);
		if (cur_opcode) {
			zval *ret = xdebug_zval_ptr(&(cur_opcode->result), current_execute_data->Ts TSRMLS_CC);
			char* t = return_trace_stack_retval(fse, ret TSRMLS_CC);
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

	xdebug_llist_remove(XG(stack), XDEBUG_LLIST_TAIL(XG(stack)), stack_element_dtor);
	XG(level)--;
}

static void dump_used_var_with_contents(void *htmlq, xdebug_hash_element* he)
{
	int        html = *(int *)htmlq;
	int        len;
	zval      *zvar;
	char      *contents;
	char      *name = (char*) he->ptr;
	HashTable *tmp_ht;
	TSRMLS_FETCH();

	if (!he->ptr) {
		return;
	}

	/* Bail out on $this and $GLOBALS */
	if (strcmp(name, "this") == 0 || strcmp(name, "GLOBALS") == 0) {
		return;
	}

	tmp_ht = XG(active_symbol_table);
	XG(active_symbol_table) = EG(active_symbol_table);
	zvar = xdebug_get_php_symbol(name, strlen(name) + 1);
	XG(active_symbol_table) = tmp_ht;

	if (!zvar) {
		return;
	}
	if (html) {
		contents = get_zval_value_fancy(NULL, zvar, &len, 0 TSRMLS_CC);
		if (contents) {
			php_printf("<tr><td colspan='2' align='right' bgcolor='#ccffcc'>$%s = </td><td bgcolor='#ccffcc'>", name);
			PHPWRITE(contents, len);
			php_printf("</td></tr>\n");
		} else {
			php_printf("<tr><td bgcolor='#ccffcc'>$%s</td><td bgcolor='#ccffcc' colspan='2'><i>Undefined</i></td></tr>\n", name);
		}
	} else {
		contents = get_zval_value(zvar, 0);
		if (contents) {
			php_printf("  $%s = %s\n", name, contents);
		} else {
			php_printf("  $%s = *uninitialized*\n", name);
		}
	}
	xdfree(contents);
}


static void print_stack(int html, const char *error_type_str, char *buffer, const char *error_filename, const int error_lineno, int log_only TSRMLS_DC)
{
	char *error_format, *error_string;
	xdebug_llist_element *le;
	int is_cli = (strcmp("cli", sapi_module.name) == 0);
	function_stack_entry *i;
	int len, dummy;

	if (html && !log_only) {
		php_printf("<br />\n<font size='1'><table border='1' cellspacing='0'>\n");
	}

	error_format = html ?
		"<tr><th align='left' bgcolor='#ee5555' colspan=\"3\">%s: <i>%s</i> in <i>%s</i> on line <i>%d</i></th></tr>\n"
		: "\n%s: %s in %s on line %d\n";
	if (!log_only) {
		if (html) {
			error_string = xmlize(buffer, strlen(buffer), &dummy);
			php_printf(error_format, error_type_str, error_string, error_filename, error_lineno);
			efree(error_string);
		} else {
			php_printf(error_format, error_type_str, buffer, error_filename, error_lineno);
		}
	}

	if (XG(stack) && XG(stack)->size) {
		i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_HEAD(XG(stack)));
		if (!log_only) {
			if (html) {
				php_printf("<tr><th align='left' bgcolor='#7777dd' colspan='3'>Call Stack</th></tr>\n");
				php_printf("<tr><th align='left' bgcolor='#9999ee'>#</th><th align='left' bgcolor='#9999ee'>Function</th><th align='left' bgcolor='#9999ee'>Location</th></tr>\n");
			} else {
				php_printf("\nCall Stack:\n");
			}
		}

		if (PG(log_errors) && !is_cli) {
			php_log_err("PHP Stack trace:" TSRMLS_CC);
		}

		for (le = XDEBUG_LLIST_HEAD(XG(stack)); le != NULL; le = XDEBUG_LLIST_NEXT(le))
		{
			int c = 0; /* Comma flag */
			int j = 0; /* Counter */
			char *tmp_name;
			xdebug_str log_buffer = {0, 0, NULL};
			
			i = XDEBUG_LLIST_VALP(le);
			tmp_name = show_fname(i->function, html, 0 TSRMLS_CC);
			if (!log_only) {
				if (html) {
					php_printf("<tr><td bgcolor='#ddddff' align='center'>%d</td><td bgcolor='#ddddff'>%s(", i->level, tmp_name);
				} else {
					php_printf("%10.4f ", i->time - XG(start_time));
#if MEMORY_LIMIT
					php_printf("%10ld ", i->memory);
#endif
					php_printf("%3d. %s(", i->level, tmp_name);
				}
			}
			if (PG(log_errors) && !is_cli) {
				xdebug_str_add(&log_buffer, xdebug_sprintf("PHP %3d. %s(", i->level, tmp_name), 1);
			}
			xdfree(tmp_name);

			/* Printing vars */
			for (j = 0; j < i->varc; j++) {
				char *tmp_varname, *tmp_value, *tmp_fancy_value;

				if (c) {
					if (!log_only) {
						php_printf(", ");
					}
					if (PG(log_errors) && !is_cli) {
						xdebug_str_addl(&log_buffer, ", ", 2, 0);
					}
				} else {
					c = 1;
				}
				tmp_varname = i->var[j].name ? xdebug_sprintf("$%s = ", i->var[j].name) : xdstrdup("");
				tmp_value = get_zval_value(i->var[j].addr, 0);
				if (!log_only) {
					if (html) {
						tmp_fancy_value = get_zval_value_fancy(tmp_varname, i->var[j].addr, &len, 0 TSRMLS_CC);
						PHPWRITE(tmp_fancy_value, len);
						xdfree(tmp_fancy_value);
					} else {
						php_printf("%s%s", tmp_varname, tmp_value);
					}
				}
				if (PG(log_errors) && !is_cli) {
					xdebug_str_add(&log_buffer, tmp_varname, 0);
					xdebug_str_add(&log_buffer, tmp_value, 0);
				}
				xdfree(tmp_varname);
				xdfree(tmp_value);
			}

			if (!log_only) {
				if (i->include_filename) {
					if (html) {
						php_printf("<font color='#00bb00'>'%s'</font>", i->include_filename);
					} else {
						php_printf("'%s'", i->include_filename);
					}
				}

				if (html) {
					php_printf(")</td><td bgcolor='#ddddff'>%s<b>:</b>%d</td></tr>\n", i->filename, i->lineno);
				} else {
					php_printf(") %s:%d\n", i->filename, i->lineno);
				}
			}
			if (PG(log_errors) && !is_cli) {
				xdebug_str_add(&log_buffer, xdebug_sprintf(") %s:%d", i->filename, i->lineno), 1);
				php_log_err(log_buffer.d TSRMLS_CC);
			}
		}

		if (!log_only)  {
			if (XG(dump_globals)) {
				dump_superglobals(html, PG(log_errors) && !is_cli TSRMLS_CC);
			}

			if (XG(show_local_vars) && XG(stack) && XDEBUG_LLIST_TAIL(XG(stack))) {
				int scope_nr = XG(stack)->size;
				
				i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack)));
				if (i->user_defined == XDEBUG_INTERNAL && XDEBUG_LLIST_PREV(XDEBUG_LLIST_TAIL(XG(stack))) && XDEBUG_LLIST_VALP(XDEBUG_LLIST_PREV(XDEBUG_LLIST_TAIL(XG(stack))))) {
					i = XDEBUG_LLIST_VALP(XDEBUG_LLIST_PREV(XDEBUG_LLIST_TAIL(XG(stack))));
					scope_nr--;
				}
				if (i->used_vars && i->used_vars->size) {
					if (html) {
						php_printf("<tr><th align='left' colspan='3' bgcolor='#33aa33'>Variables in local scope (#%d)</th></tr>\n", scope_nr);
						php_printf("<tr><th align='left' colspan='2' bgcolor='#55cc55'>Variable</th><th align='left' bgcolor='#55cc55'>Value</th></tr>\n");
					} else {
						php_printf("\n\nVariables in local scope:\n");
					}
					xdebug_hash_apply(i->used_vars, (void*) &html, dump_used_var_with_contents);
				}
			}
		}

		if (html && !log_only) {
			php_printf("</table></font>\n");
		}
	}
}

static char* return_trace_stack_retval(function_stack_entry* i, zval* retval TSRMLS_DC)
{
	int        j = 0; /* Counter */
	xdebug_str str = {0, 0, NULL};
	char      *tmp_value;

	if (XG(trace_format) != 0) {
		return xdstrdup("");
	}

	xdebug_str_addl(&str, "                    ", 20, 0);
	if (XG(show_mem_delta)) {
		xdebug_str_addl(&str, "        ", 8, 0);
	}
	for (j = 0; j < i->level; j++) {
		xdebug_str_addl(&str, "  ", 2, 0);
	}
	xdebug_str_addl(&str, "   >=> ", 7, 0);

	tmp_value = get_zval_value(retval, 0);
	if (tmp_value) {
		xdebug_str_add(&str, tmp_value, 1);
	}
	xdebug_str_addl(&str, "\n", 2, 0);

	return str.d;
}

static char* return_trace_stack_frame_begin_normal(function_stack_entry* i TSRMLS_DC)
{
	int c = 0; /* Comma flag */
	int j = 0; /* Counter */
	char *tmp_name;
	xdebug_str str = {0, 0, NULL};

	tmp_name = show_fname(i->function, 0, 0 TSRMLS_CC);

	xdebug_str_add(&str, xdebug_sprintf("%10.4f ", i->time - XG(start_time)), 1);
	xdebug_str_add(&str, xdebug_sprintf("%10lu ", i->memory), 1);
	if (XG(show_mem_delta)) {
		xdebug_str_add(&str, xdebug_sprintf("%+8ld ", i->memory - i->prev_memory), 1);
	}
	for (j = 0; j < i->level; j++) {
		xdebug_str_addl(&str, "  ", 2, 0);
	}
	xdebug_str_add(&str, xdebug_sprintf("-> %s(", tmp_name), 1);

	xdfree(tmp_name);

	/* Printing vars */
	for (j = 0; j < i->varc; j++) {
		char *tmp_varname;
		char *tmp_value;

		if (c) {
			xdebug_str_addl(&str, ", ", 2, 0);
		} else {
			c = 1;
		}

		tmp_varname = i->var[j].name ? xdebug_sprintf("$%s = ", i->var[j].name) : xdstrdup("");
		xdebug_str_add(&str, tmp_varname, 1);

		tmp_value = get_zval_value(i->var[j].addr, 0);
		if (tmp_value) {
			xdebug_str_add(&str, tmp_value, 1);
		} else {
			xdebug_str_add(&str, "???", 0);
		}
	}

	if (i->include_filename) {
		xdebug_str_add(&str, i->include_filename, 0);
	}

	xdebug_str_add(&str, xdebug_sprintf(") %s:%d\n", i->filename, i->lineno), 1);

	return str.d;
}

#define return_trace_stack_frame_begin_computerized(i,f)  return_trace_stack_frame_computerized((i), (f), 0 TSRMLS_CC)
#define return_trace_stack_frame_end_computerized(i,f)    return_trace_stack_frame_computerized((i), (f), 1 TSRMLS_CC)

static char* return_trace_stack_frame_computerized(function_stack_entry* i, int fnr, int whence TSRMLS_DC)
{
	char *tmp_name;
	xdebug_str str = {0, 0, NULL};

	xdebug_str_add(&str, xdebug_sprintf("%d\t", i->level), 1);
	xdebug_str_add(&str, xdebug_sprintf("%d\t", fnr), 1);
	if (whence == 0) { /* start */
		tmp_name = show_fname(i->function, 0, 0 TSRMLS_CC);

		xdebug_str_add(&str, "0\t", 0);
		xdebug_str_add(&str, xdebug_sprintf("%f\t", i->time - XG(start_time)), 1);
		xdebug_str_add(&str, xdebug_sprintf("%lu\t", i->memory), 1);
		xdebug_str_add(&str, xdebug_sprintf("%s\t", tmp_name), 1);
		xdebug_str_add(&str, xdebug_sprintf("%d\t", i->user_defined == XDEBUG_EXTERNAL ? 1 : 0), 1);
		xdfree(tmp_name);

		if (i->include_filename) {
			xdebug_str_add(&str, i->include_filename, 0);
		}

		xdebug_str_add(&str, xdebug_sprintf("\t%s\t%d\n", i->filename, i->lineno), 1);

	} else if (whence == 1) { /* end */
		xdebug_str_add(&str, "1\t", 0);
		xdebug_str_add(&str, xdebug_sprintf("%f\t", xdebug_get_utime() - XG(start_time)), 1);
#if MEMORY_LIMIT
		xdebug_str_add(&str, xdebug_sprintf("%lu\n", AG(allocated_memory)), 1);
#endif
	}

	return str.d;
}


static char* return_trace_stack_frame_begin(function_stack_entry* i, int fnr TSRMLS_DC)
{
	switch (XG(trace_format)) {
		case 0:
			return return_trace_stack_frame_begin_normal(i TSRMLS_CC);
		case 1:
			return return_trace_stack_frame_begin_computerized(i, fnr);
		default:
			return xdstrdup("");
	}
}


static char* return_trace_stack_frame_end(function_stack_entry* i, int fnr TSRMLS_DC)
{
	switch (XG(trace_format)) {
		case 1:
			return return_trace_stack_frame_end_computerized(i, fnr);
		default:
			return xdstrdup("");
	}
}

#ifdef ZEND_ENGINE_2
void xdebug_throw_exception_hook(zval *exception TSRMLS_DC)
{
	zval *message, *file, *line;
	zend_class_entry *default_ce, *exception_ce;
	xdebug_brk_info *extra_brk_info;
	int do_exception = 0;

	if (!exception) {
		return;
	}

#if (PHP_MAJOR_VERSION >= 6) || ((PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION >= 2))
	default_ce = zend_exception_get_default(TSRMLS_C);
#else
	default_ce = zend_exception_get_default();
#endif
	exception_ce = zend_get_class_entry(exception TSRMLS_CC);

	message = zend_read_property(default_ce, exception, "message", sizeof("message")-1, 0 TSRMLS_CC);
	file =    zend_read_property(default_ce, exception, "file",    sizeof("file")-1,    0 TSRMLS_CC);
	line =    zend_read_property(default_ce, exception, "line",    sizeof("line")-1,    0 TSRMLS_CC);

	if (XG(show_ex_trace)) {
		print_stack(!(strcmp("cli", sapi_module.name) == 0), exception_ce->name, Z_STRVAL_P(message), Z_STRVAL_P(file), Z_LVAL_P(line), !PG(display_errors) TSRMLS_CC);
	}

	/* Start JIT if requested and not yet enabled */
	if (!XG(remote_enabled) && XG(remote_enable) && (XG(remote_mode) == XDEBUG_JIT)) {
		XG(context).socket = xdebug_create_socket(XG(remote_host), XG(remote_port));
		if (XG(context).socket >= 0) {
			XG(remote_enabled) = 1;
			XG(context).program_name = NULL;

			/* Get handler from mode */
			XG(context).handler = xdebug_handler_get(XG(remote_handler));
			XG(context).handler->remote_init(&(XG(context)), XDEBUG_JIT);
		}
	}
	if (XG(remote_enabled)) {
		/* Check if we have a breakpoint on this exception */
		if (xdebug_hash_find(XG(context).exception_breakpoints, exception_ce->name, strlen(exception_ce->name), (void *) &extra_brk_info)) {
			if (handle_hit_value(extra_brk_info)) {
				if (!XG(context).handler->remote_error(&(XG(context)), 0, exception_ce->name, Z_STRVAL_P(message), Z_STRVAL_P(file), Z_LVAL_P(line), XG(stack))) {
					XG(remote_enabled) = 0;
				}
			}
		}
	}
}

/* Opcode handler for exit, to be able to clean up the profiler */
int xdebug_exit_handler(ZEND_OPCODE_HANDLER_ARGS)
{
	if (XG(profiler_enabled)) {
		xdebug_profiler_deinit(TSRMLS_C);
	}
	
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1) || PHP_MAJOR_VERSION >= 6
	return ZEND_USER_OPCODE_DISPATCH;
#else
	return old_exit_handler(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
#endif
}
#endif

void xdebug_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args)
{
	char *buffer, *error_type_str;
	int buffer_len;

	TSRMLS_FETCH();

	buffer_len = vspprintf(&buffer, PG(log_errors_max_len), format, args);

	error_type_str = error_type(type);

#if PHP_MAJOR_VERSION >= 5
	/* according to error handling mode, suppress error, throw exception or show it */
	if (PG(error_handling) != EH_NORMAL) {
		switch (type) {
			case E_CORE_ERROR:
			case E_COMPILE_ERROR:
			case E_PARSE:
				/* fatal errors are real errors and cannot be made exceptions */
				break;
			case E_STRICT:
				/* for the sake of BC to old damaged code */
				break;
			case E_NOTICE:
			case E_USER_NOTICE:
				/* notices are no errors and are not treated as such like E_WARNINGS */
				break;
			default:
				/* throw an exception if we are in EH_THROW mode
				 * but DO NOT overwrite a pending exception
				 */
				if (PG(error_handling) == EH_THROW && !EG(exception)) {
					zend_throw_error_exception(PG(exception_class), buffer, 0, type TSRMLS_CC);
				}
				efree(buffer);
				return;
		}
	}
#endif

	if (XG(error_handler)) { /* If an error handler is set, use it */
/*
		call_handler(error_type_str, buffer, error_filename, error_lineno TSRMLS_CC);
*/
	} else if ((EG(error_reporting) & type)) { /* Otherwise print the default stack trace */
		print_stack(!(strcmp("cli", sapi_module.name) == 0), error_type_str, buffer, error_filename, error_lineno, !PG(display_errors) TSRMLS_CC);
	}

	/* Log to logger */
	if (PG(log_errors) && !(strcmp("cli", sapi_module.name) == 0) && (EG(error_reporting) & type)) {
		char log_buffer[1024];

#ifdef PHP_WIN32
		if (type==E_CORE_ERROR || type==E_CORE_WARNING) {
			MessageBox(NULL, buffer, error_type_str, MB_OK|ZEND_SERVICE_MB_STYLE);
		}
#endif
		snprintf(log_buffer, 1024, "PHP %s:  %s in %s on line %d", error_type_str, buffer, error_filename, error_lineno);
		php_log_err(log_buffer TSRMLS_CC);
	}
	xdfree(error_type_str);

	/* Bail out if we can't recover */
	switch (type) {
		case E_CORE_ERROR:
		/* no break - intentionally */
		case E_ERROR:
		/*case E_PARSE: the parser would return 1 (failure), we can bail out nicely */
		case E_COMPILE_ERROR:
		case E_USER_ERROR:
			EG(exit_status) = 255;
#if MEMORY_LIMIT
			/* restore memory limit */
			AG(memory_limit) = PG(memory_limit);
#endif
			zend_bailout();
			return;
	}

	if (PG(track_errors) && EG(active_symbol_table)) {
		zval *tmp;

		ALLOC_ZVAL(tmp);
		INIT_PZVAL(tmp);
		Z_STRVAL_P(tmp) = (char *) estrndup(buffer, buffer_len);
		Z_STRLEN_P(tmp) = buffer_len;
		Z_TYPE_P(tmp) = IS_STRING;
		zend_hash_update(EG(active_symbol_table), "php_errormsg", sizeof("php_errormsg"), (void **) & tmp, sizeof(zval *), NULL);
	}

	efree(buffer);
}

/* {{{ zend_op_array srm_compile_file (file_handle, type)
 *    This function provides a hook for the execution of bananas */
zend_op_array *xdebug_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC)
{
	zend_op_array *op_array;
	
	op_array = old_compile_file(file_handle, type TSRMLS_CC);
		
	return op_array;
}
/* }}} */

/* {{{ proto integet xdebug_get_stack_depth()
   Returns the stack depth */
PHP_FUNCTION(xdebug_get_stack_depth)
{
	/* We substract one so that the function call to xdebug_get_stack_depth()
	 * is not part of the returned depth. */
	RETURN_LONG(XG(stack)->size - 1);
}

/* {{{ proto array xdebug_get_function_stack()
   Returns an array representing the current stack */
PHP_FUNCTION(xdebug_get_function_stack)
{
	xdebug_llist_element *le;
	int                   j;
	unsigned int          k;
	zval                 *frame;
	zval                 *params;
	char                 *argument;

	array_init(return_value);
	le = XDEBUG_LLIST_HEAD(XG(stack));

	for (k = 0; k < XG(stack)->size - 1; k++, le = XDEBUG_LLIST_NEXT(le)) {
		function_stack_entry *i = XDEBUG_LLIST_VALP(le);

		if (i->function.function) {
			if (strcmp(i->function.function, "xdebug_get_function_stack") == 0) {
				return;
			}
		}

		/* Initialize frame array */
		MAKE_STD_ZVAL(frame);
		array_init(frame);

		/* Add data */
		if (i->function.function) {
			add_assoc_string_ex(frame, "function", sizeof("function"), i->function.function, 1);
		}
		if (i->function.class) {
			add_assoc_string_ex(frame, "class",    sizeof("class"),    i->function.class,    1);
		}
		add_assoc_string_ex(frame, "file", sizeof("file"), i->filename, 1);
		add_assoc_long_ex(frame, "line", sizeof("line"), i->lineno);

		/* Add parameters */
		MAKE_STD_ZVAL(params);
		array_init(params);
		for (j = 0; j < i->varc; j++) {
			argument = get_zval_value(i->var[j].addr, 0);
			if (i->var[j].name) {
				add_assoc_string_ex(params, i->var[j].name, strlen(i->var[j].name) + 1, argument, 1);
			} else {
				add_index_string(params, j, argument, 1);
			}
			xdfree(argument);
		}
		add_assoc_zval_ex(frame, "params", sizeof("params"), params);

		if (i->include_filename) {
			add_assoc_string_ex(frame, "include_filename", sizeof("include_filename"), i->include_filename, 1);
		}

		add_next_index_zval(return_value, frame);
	}
}
/* }}} */

static void attach_used_var_names(void *return_value, xdebug_hash_element *he)
{
	char *name = (char*) he->ptr;
	
	add_next_index_string(return_value, name, 1);
}

/* {{{ proto array xdebug_get_declared_vars()
   Returns an array representing the current stack */
PHP_FUNCTION(xdebug_get_declared_vars)
{
	xdebug_llist_element *le;
	int                   j;
	function_stack_entry *i;

	array_init(return_value);
	le = XDEBUG_LLIST_TAIL(XG(stack));
	le = XDEBUG_LLIST_PREV(le);
	i = XDEBUG_LLIST_VALP(le);
	
	/* Add declared vars */
	if (i->used_vars) {
		xdebug_hash_apply(i->used_vars, (void *) return_value, attach_used_var_names);
	}
	/* Add params */
	if (i->var) {
		for (j = 0; j < i->varc; j++) {
			if (i->var[j].name) {
				add_next_index_string(return_value, i->var[j].name, 1);
			}
		}
	}
}
/* }}} */

/* {{{ proto string xdebug_call_class()
   Returns the name of the calling class */
PHP_FUNCTION(xdebug_call_class)
{
	xdebug_llist_element *le;
	function_stack_entry *i;

	le = XDEBUG_LLIST_TAIL(XG(stack));
	if (le) {
		if (le->prev) {
			le = XDEBUG_LLIST_PREV(le);
			if (le->prev) {
				le = XDEBUG_LLIST_PREV(le);
			}
		}
		i = XDEBUG_LLIST_VALP(le);

		RETURN_STRING(i->function.class ? i->function.class : "", 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string xdebug_call_function()
   Returns the function name from which the current function was called from. */
PHP_FUNCTION(xdebug_call_function)
{
	xdebug_llist_element *le;
	function_stack_entry *i;

	le = XDEBUG_LLIST_TAIL(XG(stack));
	if (le) {
		if (le->prev) {
			le = XDEBUG_LLIST_PREV(le);
			if (le->prev) {
				le = XDEBUG_LLIST_PREV(le);
			}
		}
		i = XDEBUG_LLIST_VALP(le);

		RETURN_STRING(i->function.function ? i->function.function : "{}", 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string xdebug_call_line()
   Returns the line number where the current function was called from. */
PHP_FUNCTION(xdebug_call_line)
{
	xdebug_llist_element *le;
	function_stack_entry *i;

	le = XDEBUG_LLIST_TAIL(XG(stack));
	if (le) {
		if (le->prev) {
			le = XDEBUG_LLIST_PREV(le);
		}
		i = XDEBUG_LLIST_VALP(le);

		RETURN_LONG(i->lineno);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int xdebug_call_file()
   Returns the filename where the current function was called from. */
PHP_FUNCTION(xdebug_call_file)
{
	xdebug_llist_element *le;
	function_stack_entry *i;

	le = XDEBUG_LLIST_TAIL(XG(stack));
	if (le) {
		if (le->prev) {
			le = XDEBUG_LLIST_PREV(le);
		}
		i = XDEBUG_LLIST_VALP(le);

		RETURN_STRING(i->filename, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

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
			val = get_zval_value_fancy(NULL, (zval*) *args[i], &len, 0 TSRMLS_CC);
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
					val = get_zval_value_fancy(NULL, debugzval, &len, 1 TSRMLS_CC);
					PHPWRITE(val, len);
				} else {
					val = get_zval_value(debugzval, 1);
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
				val = get_zval_value(debugzval, 1);
				printf("%s(%d)", val, strlen(val));
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
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1) || PHP_MAJOR_VERSION >= 6
	zend_throw_exception_hook = xdebug_throw_exception_hook;
#endif
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 0)
	zend_opcode_handlers[ZEND_EXIT] = xdebug_exit_handler;
#endif
}

PHP_FUNCTION(xdebug_disable)
{
	zend_error_cb = old_error_cb;
#ifdef ZEND_ENGINE_2
# if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 0)
	zend_opcode_handlers[ZEND_EXIT] = old_exit_handler;
# endif
	zend_throw_exception_hook = NULL;
#endif
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

		if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_BREAK)) {
			XG(remote_enabled) = 0;
		}
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(xdebug_start_trace)
{
	char *fname = NULL;
	int   fname_len = 0;
	char *trace_fname;
	long  options = 0;

	if (XG(do_trace) == 0) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &fname, &fname_len, &options) == FAILURE) {
			return;
		}

		if (fname && strlen(fname)) {
			if ((trace_fname = xdebug_start_trace(fname, options TSRMLS_CC)) != NULL) {
				XG(do_trace) = 1;
				RETVAL_STRING(trace_fname, 1);
				xdfree(trace_fname);
				return;
			} else {
				php_error(E_NOTICE, "Trace could not be started");
			}
		} else {
			php_error(E_NOTICE, "Filename can not be empty");
		}
		XG(do_trace) = 0;
		RETURN_FALSE;
	} else {
		php_error(E_NOTICE, "Function trace already started");
		RETURN_FALSE;
	}
}

char* xdebug_start_trace(char* fname, long options TSRMLS_DC)
{
	char *str_time;
	char *filename;
	char  cwd[128];

	if (fname) {
		filename = xdebug_sprintf("%s.xt", fname);
	} else {
		if (strcmp(XG(trace_output_name), "crc32") == 0) {
			VCWD_GETCWD(cwd, 127);
			filename = xdebug_sprintf("%s/trace.%lu.xt", XG(trace_output_dir), xdebug_crc32(cwd, strlen(cwd)));
		} else if (strcmp(XG(trace_output_name), "timestamp") == 0) {
			time_t the_time = time(NULL);
			filename = xdebug_sprintf("%s/trace.%ld.xt", XG(trace_output_dir), the_time);
		} else {
			filename = xdebug_sprintf("%s/trace.%ld.xt", XG(trace_output_dir), getpid());
		}
	}
	if (options & XDEBUG_TRACE_OPTION_APPEND) {
		XG(trace_file) = fopen(filename, "a");
	} else {
		XG(trace_file) = fopen(filename, "w");
	}
	if (options & XDEBUG_TRACE_OPTION_COMPUTERIZED) {
		XG(trace_format) = 1;
	}
	XG(tracefile_name) = estrdup(filename);
	if (XG(trace_file)) {
		if (XG(trace_format) == 1) {
			fprintf(XG(trace_file), "Version: %s\n", XDEBUG_VERSION);
		}
		str_time = xdebug_get_time();
		fprintf(XG(trace_file), "TRACE START [%s]\n", str_time);
		XG(do_trace) = 1;
		xdfree(str_time);
		return filename;
	}
	return NULL;
}

void xdebug_stop_trace(TSRMLS_D)
{
	char   *str_time;
	double  u_time;

	XG(do_trace) = 0;
	if (XG(trace_file)) {
		u_time = xdebug_get_utime();
		fprintf(XG(trace_file), "%10.4f ", u_time - XG(start_time));
#if MEMORY_LIMIT
		fprintf(XG(trace_file), "%10u", AG(allocated_memory));
#else
		fprintf(XG(trace_file), "%10u", 0);
#endif
		fprintf(XG(trace_file), "\n");
		str_time = xdebug_get_time();
		fprintf(XG(trace_file), "TRACE END   [%s]\n\n", str_time);
		fclose(XG(trace_file));
		XG(trace_file) = NULL;
		xdfree(str_time);
	}
	if (XG(tracefile_name)) {
		efree(XG(tracefile_name));
		XG(tracefile_name) = NULL;
	}
}

PHP_FUNCTION(xdebug_stop_trace)
{
	if (XG(do_trace) == 1) {
		xdebug_stop_trace(TSRMLS_C);
	} else {
		php_error(E_NOTICE, "Function trace was not started");
	}
}

PHP_FUNCTION(xdebug_get_tracefile_name)
{
	if (XG(tracefile_name)) {
		RETURN_STRING(XG(tracefile_name), 1);
	} else {
		RETURN_FALSE;
	}
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

#if MEMORY_LIMIT
PHP_FUNCTION(xdebug_memory_usage)
{
	RETURN_LONG(AG(allocated_memory));
}

PHP_FUNCTION(xdebug_peak_memory_usage)
{
	RETURN_LONG(AG(allocated_memory_peak));
}
#endif

PHP_FUNCTION(xdebug_time_index)
{
	RETURN_DOUBLE(xdebug_get_utime() - XG(start_time));
}

PHP_FUNCTION(xdebug_set_error_handler)
{
	char *handler;
	int   handler_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &handler, &handler_len) == FAILURE) {
		return;
	}

	if (XG(error_handler)) {
		efree(XG(error_handler));
	}
	XG(error_handler) = estrndup(handler, handler_len);
}

/*************************************************************************************************************************************/
#define T(offset) (*(temp_variable *)((char *) Ts + offset))

static zval *get_zval(zend_execute_data *zdata, znode *node, temp_variable *Ts, int *is_var)
{
	switch (node->op_type) {
		case IS_CONST:
			return &node->u.constant;
			break;

		case IS_TMP_VAR:
			*is_var = 1;
#ifdef ZEND_ENGINE_2
			return &T(node->u.var).tmp_var;
#else
			return &Ts[node->u.var].tmp_var;
#endif
			break;

		case IS_VAR:
			*is_var = 1;
#ifdef ZEND_ENGINE_2
			if (T(node->u.var).var.ptr) {
				return T(node->u.var).var.ptr;
			} else {
				fprintf(stderr, "\nIS_VAR\n");
			}
#else
			if (Ts[node->u.var].var.ptr) {
				return Ts[node->u.var].var.ptr;
			} else {
				fprintf(stderr, "\nIS_VAR\n");
			}
#endif
			break;

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1) || (PHP_MAJOR_VERSION >= 6)
		case IS_CV:
			return *zend_get_compiled_variable_value(zdata, node->u.constant.value.lval);
			break;
#endif

		case IS_UNUSED:
			fprintf(stderr, "\nIS_UNUSED\n");
			break;

		default:
			fprintf(stderr, "\ndefault %d\n", node->op_type);
			break;
	}

	*is_var = 1;

	return NULL;
}


ZEND_DLEXPORT void xdebug_statement_call(zend_op_array *op_array)
{
	xdebug_llist_element *le;
	xdebug_brk_info      *brk;
	function_stack_entry *fse;
	zend_op              *cur_opcode;
	int                   lineno;
	char                 *file;
	int                   file_len = 0;
	int                   level = 0;
	TSRMLS_FETCH();

	cur_opcode = *EG(opline_ptr);
	lineno = cur_opcode->lineno;

	file = op_array->filename;
	file_len = strlen(file);

	if (XG(do_code_coverage)) {
		xdebug_count_line(file, lineno, 0 TSRMLS_CC);
	}

	if (XG(remote_enabled)) {

		if (XG(context).do_break) {
			XG(context).do_break = 0;

			if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_BREAK)) {
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

			if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_STEP)) {
				XG(remote_enabled) = 0;
				return;
			}
		} else if (XG(context).do_next && XG(context).next_level >= level) { /* Check for "next" */
			XG(context).do_next = 0;

			if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_STEP)) {
				XG(remote_enabled) = 0;
				return;
			}
		} else if (XG(context).do_step) { /* Check for "step" */
			XG(context).do_step = 0;

			if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_STEP)) {
				XG(remote_enabled) = 0;
				return;
			}
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
				if (!brk->disabled && lineno == brk->lineno && memcmp(brk->file, file + file_len - brk->file_len, brk->file_len) == 0) {
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
					if (break_ok && handle_hit_value(brk)) {
						if (!XG(context).handler->remote_breakpoint(&(XG(context)), XG(stack), file, lineno, XDEBUG_BREAK)) {
							XG(remote_enabled) = 0;
							break;
						}
						break;
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

#ifndef ZEND_EXT_API
#define ZEND_EXT_API    ZEND_DLEXPORT
#endif
ZEND_EXTENSION();

ZEND_DLEXPORT zend_extension zend_extension_entry = {
	XDEBUG_NAME,
	XDEBUG_VERSION,
	XDEBUG_AUTHOR,
	XDEBUG_URL,
	"Copyright (c) 2002, 2003, 2004, 2005, 2006",
	xdebug_zend_startup,
	xdebug_zend_shutdown,
	NULL,           /* activate_func_t */
	NULL,           /* deactivate_func_t */
	NULL,           /* message_handler_func_t */
	NULL,           /* op_array_handler_func_t */
	xdebug_statement_call, /* statement_handler_func_t */
	NULL,           /* fcall_begin_handler_func_t */
	NULL,           /* fcall_end_handler_func_t */
	NULL,           /* op_array_ctor_func_t */
	NULL,           /* op_array_dtor_func_t */
	STANDARD_ZEND_EXTENSION_PROPERTIES
};

#endif
