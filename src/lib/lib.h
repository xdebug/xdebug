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

#ifndef __XDEBUG_LIBRARY_H__
#define __XDEBUG_LIBRARY_H__

#ifdef ZTS
# include "TSRM.h"
#endif

#include "zend.h"
#include "zend_API.h"
#include "compat.h"

#define MICRO_IN_SEC 1000000.00

typedef struct xdebug_var_name {
	char    *name;
	size_t   length;
	zval     data;
	int      is_variadic;
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

#define XFUNC_ZEND_PASS      0x20

#define XDEBUG_IS_NORMAL_FUNCTION(f) ((f)->type == XFUNC_NORMAL || (f)->type == XFUNC_STATIC_MEMBER || (f)->type == XFUNC_MEMBER)

#define XDEBUG_REGISTER_LONG_CONSTANT(__c) REGISTER_LONG_CONSTANT(#__c, __c, CONST_CS|CONST_PERSISTENT)

#define XDEBUG_NONE          0
#define XDEBUG_JIT           1
#define XDEBUG_REQ           2

#define XDEBUG_BREAK         1
#define XDEBUG_STEP          2

#define XDEBUG_BUILT_IN      1
#define XDEBUG_USER_DEFINED  2

#define XDEBUG_MAX_FUNCTION_LEN 1024

#define XDEBUG_TRACE_OPTION_APPEND         1
#define XDEBUG_TRACE_OPTION_COMPUTERIZED   2
#define XDEBUG_TRACE_OPTION_HTML           4
#define XDEBUG_TRACE_OPTION_NAKED_FILENAME 8

#define XDEBUG_CC_OPTION_UNUSED          1
#define XDEBUG_CC_OPTION_DEAD_CODE       2
#define XDEBUG_CC_OPTION_BRANCH_CHECK    4

#define XDEBUG_LOG_ERR               1
#define XDEBUG_LOG_WARN              3
#define XDEBUG_LOG_COM               5
#define XDEBUG_LOG_INFO              7
#define XDEBUG_LOG_DEBUG            10
#define XDEBUG_LOG_DEFAULT          "7" /* as a string, as that's what STD_PHP_INI_ENTRY wants */

extern const char* xdebug_log_prefix[11];

#define STATUS_STARTING   0
#define STATUS_STOPPING   1
#define STATUS_STOPPED    2
#define STATUS_RUNNING    3
#define STATUS_BREAK      4

#define REASON_OK         0
#define REASON_ERROR      1
#define REASON_ABORTED    2
#define REASON_EXCEPTION  3

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
	char *class;
	char *function;
	int   type;
	int   internal;
} xdebug_func;

typedef struct _xdebug_call_entry {
	int         type; /* 0 = function call, 1 = line */
	int         user_defined;
	char       *filename;
	char       *function;
	int         lineno;
	double      time_taken;
	long        mem_used;
} xdebug_call_entry;

typedef struct xdebug_profile {
	double        time;
	double        mark;
	long          memory;
	long          mem_mark;
	xdebug_llist *call_list;
} xdebug_profile;

typedef struct _function_stack_entry {
	/* function properties */
	xdebug_func  function;
	int          user_defined;

	/* location properties */
	unsigned int level;
	char        *filename;
	int          lineno;
	char        *include_filename;
	int          function_nr;

	/* argument properties */
	int                arg_done;
	unsigned int       varc;
	xdebug_var_name   *var;
	int                is_variadic;
	zval              *return_value;
	xdebug_llist      *declared_vars;
	HashTable         *symbol_table;
	zend_execute_data *execute_data;
	zval              *This;

	/* filter properties */
	long         filtered_tracing;
	long         filtered_code_coverage;

	/* tracing properties */
	signed long  memory;
	signed long  prev_memory;
	double       time;

	/* profiling properties */
	xdebug_profile profile;
	struct {
		int   lineno;
		char *filename;
		char *funcname;
	} profiler;

	/* misc properties */
	int          refcount;
	struct _function_stack_entry *prev;
	zend_op_array *op_array;
} function_stack_entry;

function_stack_entry *xdebug_get_stack_head(void);
function_stack_entry *xdebug_get_stack_frame(int nr);
function_stack_entry *xdebug_get_stack_tail(void);


xdebug_hash* xdebug_declared_var_hash_from_llist(xdebug_llist *list);
int xdebug_trigger_enabled(int setting, const char *var_name, char *var_value);

typedef struct _xdebug_library_globals_t {
	zend_execute_data     *active_execute_data;
	function_stack_entry  *active_stack_entry;
	HashTable             *active_symbol_table;
	zval                  *active_object;
	user_opcode_handler_t  original_opcode_handlers[256];
	xdebug_set            *opcode_handlers_set;
} xdebug_library_globals_t;

typedef struct _xdebug_library_settings_t {
#if WIN32
	int dummy;
#endif
} xdebug_library_settings_t;

void xdebug_init_library_globals(xdebug_library_globals_t *xg);
void xdebug_library_minit(void);
void xdebug_library_mshutdown(void);

void xdebug_lib_set_active_data(zend_execute_data *execute_data);
void xdebug_lib_set_active_object(zval *object);
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

/* Needed for code coverage as Zend doesn't always add EXT_STMT when expected */
#define XDEBUG_SET_OPCODE_OVERRIDE_COMMON(oc) \
	xdebug_set_opcode_handler(oc, xdebug_common_override_handler);

#define XDEBUG_SET_OPCODE_OVERRIDE_ASSIGN(f,oc) \
	xdebug_set_opcode_handler(oc, xdebug_##f##_handler);

int xdebug_isset_opcode_handler(int opcode);
void xdebug_set_opcode_handler(int opcode, user_opcode_handler_t handler);
void xdebug_unset_opcode_handler(int opcode);
int xdebug_call_original_opcode_handler_if_set(int opcode, XDEBUG_OPCODE_HANDLER_ARGS);

#endif
