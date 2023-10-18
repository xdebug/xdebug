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

#ifndef __XDEBUG_LIBRARY_H__
#define __XDEBUG_LIBRARY_H__

#ifdef ZTS
# include "TSRM.h"
#endif

#include "zend.h"
#include "zend_API.h"
#include "compat.h"

extern int xdebug_global_mode;

typedef struct xdebug_var_name {
	zend_string *name;
	zval         data;
	int          is_variadic;
} xdebug_var_name;

#define XFUNC_UNKNOWN        0x00
#define XFUNC_NORMAL         0x01
#define XFUNC_STATIC_MEMBER  0x02
#define XFUNC_MEMBER         0x03

#define XFUNC_INCLUDES       0x10
#define XFUNC_EVAL           0x10
#define XFUNC_INCLUDE        0x11
#define XFUNC_INCLUDE_ONCE   0x12
#define XFUNC_REQUIRE        0x13
#define XFUNC_REQUIRE_ONCE   0x14
#define XFUNC_MAIN           0x15
#if PHP_VERSION_ID >= 80100
# define XFUNC_FIBER         0x16
#endif

#define XFUNC_ZEND_PASS      0x20

#define XDEBUG_IS_NORMAL_FUNCTION(f) ((f)->type == XFUNC_NORMAL || (f)->type == XFUNC_STATIC_MEMBER || (f)->type == XFUNC_MEMBER)

#define XDEBUG_REGISTER_LONG_CONSTANT(__c) REGISTER_LONG_CONSTANT(#__c, __c, CONST_CS|CONST_PERSISTENT)

#define XDEBUG_NONE          0
#define XDEBUG_JIT           1
#define XDEBUG_REQ           2

#define XDEBUG_BREAK         1
#define XDEBUG_STEP          2

#define XDEBUG_BUILT_IN      0
#define XDEBUG_USER_DEFINED  1

#define XDEBUG_MAX_FUNCTION_LEN 1024

#define XDEBUG_TRACE_OPTION_APPEND          0x01
#define XDEBUG_TRACE_OPTION_COMPUTERIZED    0x02
#define XDEBUG_TRACE_OPTION_HTML            0x04
#define XDEBUG_TRACE_OPTION_NAKED_FILENAME  0x08
#define XDEBUG_TRACE_OPTION_FLAMEGRAPH_COST 0x10
#define XDEBUG_TRACE_OPTION_FLAMEGRAPH_MEM  0x20

#define XDEBUG_CC_OPTION_UNUSED          1
#define XDEBUG_CC_OPTION_DEAD_CODE       2
#define XDEBUG_CC_OPTION_BRANCH_CHECK    4

#define STATUS_STARTING   0
#define STATUS_STOPPING   1
#define STATUS_STOPPED    2
#define STATUS_RUNNING    3
#define STATUS_BREAK      4

#define REASON_OK         0
#define REASON_ERROR      1
#define REASON_ABORTED    2
#define REASON_EXCEPTION  3

#define XDEBUG_CMDLOOP_NONBLOCK                      0
#define XDEBUG_CMDLOOP_BLOCK                         1

#define XDEBUG_CMDLOOP_NONBAIL                       0
#define XDEBUG_CMDLOOP_BAIL                          1

#define XDEBUG_ERROR_OK                              0
#define XDEBUG_ERROR_PARSE                           1
#define XDEBUG_ERROR_DUP_ARG                         2
#define XDEBUG_ERROR_INVALID_ARGS                    3
#define XDEBUG_ERROR_UNIMPLEMENTED                   4
#define XDEBUG_ERROR_COMMAND_UNAVAILABLE             5

#define XDEBUG_ERROR_CANT_OPEN_FILE                100
#define XDEBUG_ERROR_STREAM_REDIRECT_FAILED        101 /* unused */

#define XDEBUG_ERROR_BREAKPOINT_NOT_SET            200
#define XDEBUG_ERROR_BREAKPOINT_TYPE_NOT_SUPPORTED 201
#define XDEBUG_ERROR_BREAKPOINT_INVALID            202
#define XDEBUG_ERROR_BREAKPOINT_NO_CODE            203
#define XDEBUG_ERROR_BREAKPOINT_INVALID_STATE      204
#define XDEBUG_ERROR_NO_SUCH_BREAKPOINT            205
#define XDEBUG_ERROR_EVALUATING_CODE               206
#define XDEBUG_ERROR_INVALID_EXPRESSION            207 /* unused */

#define XDEBUG_ERROR_PROPERTY_NON_EXISTENT         300
#define XDEBUG_ERROR_PROPERTY_NON_EXISTANT         300 /* compatibility typo */
#define XDEBUG_ERROR_STACK_DEPTH_INVALID           301
#define XDEBUG_ERROR_CONTEXT_INVALID               302 /* unused */

#define XDEBUG_ERROR_PROFILING_NOT_STARTED         800

#define XDEBUG_ERROR_ENCODING_NOT_SUPPORTED        900

typedef struct _xdebug_func {
	zend_string *object_class;
	zend_string *scope_class;
	zend_string *function;
	int   type;
	int   internal;
} xdebug_func;

typedef struct xdebug_profile {
	uint64_t      nanotime;
	uint64_t      nanotime_mark;
	long          memory;
	long          mem_mark;
	xdebug_llist *call_list;
} xdebug_profile;

typedef struct _function_stack_entry {
	/* function properties */
	xdebug_func    function;
	unsigned int   function_nr;
	unsigned short user_defined:1;
	unsigned short level:15;

	/* argument properties */
	unsigned short     varc;
	xdebug_var_name   *var;
	zval              *return_value;
	xdebug_llist      *declared_vars;
	HashTable         *symbol_table;
	zend_execute_data *execute_data;
	unsigned char      is_variadic;
	unsigned char      is_trampoline;
	unsigned char      arg_done;

	/* debugging properties */
	bool has_line_breakpoints;

	/* filter properties */
	unsigned char filtered_code_coverage;
	unsigned char filtered_stack;
	unsigned char filtered_tracing;

	/* coverage properties */
	bool         code_coverage_init;
	char        *code_coverage_function_name;
	zend_string *code_coverage_filename;

	/* location properties */
	int          lineno;
	zend_string *filename;
	zend_string *include_filename;

	/* tracing properties */
	signed long  memory;
	signed long  prev_memory;
	uint64_t     nanotime;
	bool         function_call_traced;

	/* profiling properties */
	xdebug_profile profile;
	struct {
		int          lineno;
		zend_string *filename;
		zend_string *function;
	} profiler;

	/* misc properties */
	zend_op_array *op_array;
#if PHP_VERSION_ID >= 80100
	void           (*soap_error_cb)(int type, zend_string *error_filename, const uint32_t error_lineno, zend_string *message);
#else
	void           (*soap_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, zend_string *message);
#endif
} function_stack_entry;

function_stack_entry *xdebug_get_stack_frame(int nr);


xdebug_hash* xdebug_declared_var_hash_from_llist(xdebug_llist *list);
int xdebug_trigger_enabled(int setting, const char *var_name, char *var_value);

typedef struct _xdebug_multi_opcode_handler_t xdebug_multi_opcode_handler_t;

struct _xdebug_multi_opcode_handler_t
{
	user_opcode_handler_t          handler;
	xdebug_multi_opcode_handler_t *next;
};

typedef struct _xdebug_library_globals_t {
	int                    start_with_request; /* One of the XDEBUG_START_WITH_REQUEST_* constants */
	int                    start_upon_error;   /* One of the XDEBUG_START_UPON_ERROR_* constants */
	int                    mode_from_environment; /* Keeps track whether the mode was set with XDEBUG_MODE for diagnostics purposes */

	zend_execute_data     *active_execute_data;
	function_stack_entry  *active_stack_entry;
	HashTable             *active_symbol_table;
	zval                  *active_object;

	/* Headers */
	xdebug_llist *headers;

	zend_bool     dumped;

	/* used for collection errors */
	zend_bool     do_collect_errors;

	FILE         *log_file;  /* File handler for protocol log */
	zend_bool     log_opened_message_sent;
	char         *log_open_timestring;
	xdebug_str   *diagnosis_buffer;

	user_opcode_handler_t          original_opcode_handlers[256];
	xdebug_multi_opcode_handler_t *opcode_multi_handlers[256];
	xdebug_set                    *opcode_handlers_set;
} xdebug_library_globals_t;

typedef struct _xdebug_library_settings_t {
	char         *requested_mode;

	char         *output_dir;
	char         *trigger_value;

	char         *file_link_format;
	char         *filename_format;

	/* Whether to use zlib compression for profiling and trace files, if ZLIB support
	 * is enabled */
	zend_bool     use_compression;

	/* variable dumping limitation settings */
	zend_long     display_max_children;
	zend_long     display_max_data;
	zend_long     display_max_depth;

	/* Logging settings */
	char         *log;       /* Filename to log protocol communication to */
	zend_long     log_level; /* Log level XDEBUG_LOG_{ERR,WARN,INFO,DEBUG} */
} xdebug_library_settings_t;

void xdebug_init_library_globals(xdebug_library_globals_t *xg);

void xdebug_library_zend_startup(void);
void xdebug_library_zend_shutdown(void);
void xdebug_library_minit(void);
void xdebug_library_mshutdown(void);
void xdebug_library_rinit(void);
void xdebug_library_post_deactivate(void);

void xdebug_disable_opcache_optimizer(void);

#define XDEBUG_MODE_OFF             0
#define XDEBUG_MODE_DEVELOP      1<<0
#define XDEBUG_MODE_COVERAGE     1<<1
#define XDEBUG_MODE_STEP_DEBUG   1<<2
#define XDEBUG_MODE_GCSTATS      1<<3
#define XDEBUG_MODE_PROFILING    1<<4
#define XDEBUG_MODE_TRACING      1<<5
int xdebug_lib_set_mode(const char *mode);

#define XDEBUG_MODE_IS_OFF() ((xdebug_global_mode == XDEBUG_MODE_OFF))
#define XDEBUG_MODE_IS(v) ((xdebug_global_mode & (v)) ? 1 : 0)
#define RETURN_IF_MODE_IS_NOT(m) if (!XDEBUG_MODE_IS((m))) { return; }
#define RETURN_FALSE_IF_MODE_IS_NOT(m) if (!XDEBUG_MODE_IS((m))) { RETURN_FALSE; }
#define WARN_AND_RETURN_IF_MODE_IS_NOT(m) if (!XDEBUG_MODE_IS((m))) { php_error(E_NOTICE, "Functionality is not enabled"); return; }

#define XDEBUG_START_WITH_REQUEST_DEFAULT     1
#define XDEBUG_START_WITH_REQUEST_YES         2
#define XDEBUG_START_WITH_REQUEST_NO          3
#define XDEBUG_START_WITH_REQUEST_TRIGGER     4
int xdebug_lib_set_start_with_request(char *value);
int xdebug_lib_start_with_request(int for_mode);
int xdebug_lib_start_with_trigger(int for_mode, char **found_trigger_value);
int xdebug_lib_start_if_mode_is_trigger(int for_mode);
int xdebug_lib_never_start_with_request(void);
int xdebug_lib_get_start_with_request(void);
int xdebug_lib_has_shared_secret(void);

#define XDEBUG_START_UPON_ERROR_DEFAULT     1
#define XDEBUG_START_UPON_ERROR_YES         2
#define XDEBUG_START_UPON_ERROR_NO          3
int xdebug_lib_set_start_upon_error(char *value);
int xdebug_lib_start_upon_error(void);
int xdebug_lib_get_start_upon_error(void);

const char *xdebug_lib_mode_from_value(int mode);

void xdebug_lib_set_active_data(zend_execute_data *execute_data);
void xdebug_lib_set_active_stack_entry(function_stack_entry *fse);
void xdebug_lib_set_active_symbol_table(HashTable *symbol_table);

int xdebug_lib_has_active_data(void);
int xdebug_lib_has_active_function(void);
int xdebug_lib_has_active_object(void);
int xdebug_lib_has_active_symbol_table(void);

zend_execute_data *xdebug_lib_get_active_data(void);
zend_op_array *xdebug_lib_get_active_func_oparray(void);
zval *xdebug_lib_get_active_object(void);
function_stack_entry *xdebug_lib_get_active_stack_entry(void);
HashTable *xdebug_lib_get_active_symbol_table(void);

int xdebug_isset_opcode_handler(int opcode);
void xdebug_set_opcode_handler(int opcode, user_opcode_handler_t handler);
void xdebug_unset_opcode_handler(int opcode);
void xdebug_set_opcode_multi_handler(int opcode);
void xdebug_register_with_opcode_multi_handler(int opcode, user_opcode_handler_t handler);
int xdebug_call_original_opcode_handler_if_set(int opcode, XDEBUG_OPCODE_HANDLER_ARGS);

char *xdebug_lib_get_output_dir(void);

void xdebug_llist_string_dtor(void *dummy, void *elem);
zend_string* xdebug_wrap_location_around_function_name(const char *prefix, zend_op_array *opa, zend_string *fname);
zend_string* xdebug_wrap_closure_location_around_function_name(zend_op_array *opa, zend_string *fname);

void xdebug_lib_register_compiled_variables(function_stack_entry *fse);
#endif
