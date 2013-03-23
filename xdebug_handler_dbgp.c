/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans                               |
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
   |           Shane Caraveo <shanec@ActiveState.com>                     |
   +----------------------------------------------------------------------+
 */

#include <sys/types.h>

#ifndef PHP_WIN32
#include <unistd.h>
#endif

#include "php.h"
#include "SAPI.h"

#include "ext/standard/php_string.h"
#include "ext/standard/url.h"
#include "main/php_version.h"
#include "TSRM.h"
#include "php_globals.h"
#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_code_coverage.h"
#include "xdebug_com.h"
#include "xdebug_compat.h"
#include "xdebug_handler_dbgp.h"
#include "xdebug_hash.h"
#include "xdebug_llist.h"
#include "xdebug_mm.h"
#include "xdebug_var.h"
#include "xdebug_xml.h"

#include "xdebug_compat.h"

#ifdef PHP_WIN32
#include "win32/time.h"
#include <process.h>
#endif
#include <fcntl.h>

ZEND_EXTERN_MODULE_GLOBALS(xdebug)
static char *create_eval_key_id(int id);

/*****************************************************************************
** Constants and strings for statii and reasons
*/

/* Status structure */
#define DBGP_STATUS_STARTING  1
#define DBGP_STATUS_STOPPING  2
#define DBGP_STATUS_STOPPED   3
#define DBGP_STATUS_RUNNING   4
#define DBGP_STATUS_BREAK     5
#define DBGP_STATUS_DETACHED  6

char *xdebug_dbgp_status_strings[6] =
	{"", "starting", "stopping", "stopped", "running", "break"};

#define DBGP_REASON_OK        0
#define DBGP_REASON_ERROR     1
#define DBGP_REASON_ABORTED   2
#define DBGP_REASON_EXCEPTION 3

char *xdebug_dbgp_reason_strings[4] =
	{"ok", "error", "aborted", "exception"};

typedef struct {
	int   code;
	char *message;
} xdebug_error_entry;

xdebug_error_entry xdebug_error_codes[24] = {
	{   0, "no error" },
	{   1, "parse error in command" },
	{   2, "duplicate arguments in command" },
	{   3, "invalid or missing options" },
	{   4, "unimplemented command" },
	{   5, "command is not available" },
	{ 100, "can not open file" },
	{ 101, "stream redirect failed" },
	{ 200, "breakpoint could not be set" },
	{ 201, "breakpoint type is not supported" },
	{ 202, "invalid breakpoint line" },
	{ 203, "no code on breakpoint line" },
	{ 204, "invalid breakpoint state" },
	{ 205, "no such breakpoint" },
	{ 206, "error evaluating code" },
	{ 207, "invalid expression" },
	{ 300, "can not get property" },
	{ 301, "stack depth invalid" },
	{ 302, "context invalid" },
	{ 800, "profiler not started" },
	{ 900, "encoding not supported" },
	{ 998, "an internal exception in the debugger" },
	{ 999, "unknown error" },
	{  -1, NULL }
};

#define XDEBUG_STR_SWITCH_DECL       char *__switch_variable
#define XDEBUG_STR_SWITCH(s)         __switch_variable = (s);
#define XDEBUG_STR_CASE(s)           if (strcmp(__switch_variable, s) == 0) {
#define XDEBUG_STR_CASE_END          } else
#define XDEBUG_STR_CASE_DEFAULT      {
#define XDEBUG_STR_CASE_DEFAULT_END  }

#define XDEBUG_TYPES_COUNT 8
char *xdebug_dbgp_typemap[XDEBUG_TYPES_COUNT][3] = {
	/* common, lang, schema */
	{"bool",     "bool",     "xsd:boolean"},
	{"int",      "int",      "xsd:decimal"},
	{"float",    "float",    "xsd:double"},
	{"string",   "string",   "xsd:string"},
	{"null",     "null",     NULL},
	{"hash",     "array",    NULL},
	{"object",   "object",   NULL},
	{"resource", "resource", NULL}
};

/*****************************************************************************
** Prototypes for debug command handlers
*/

/* DBGP_FUNC(break); */
DBGP_FUNC(breakpoint_get);
DBGP_FUNC(breakpoint_list);
DBGP_FUNC(breakpoint_remove);
DBGP_FUNC(breakpoint_set);
DBGP_FUNC(breakpoint_update);

DBGP_FUNC(context_get);
DBGP_FUNC(context_names);

DBGP_FUNC(eval);
DBGP_FUNC(feature_get);
DBGP_FUNC(feature_set);

DBGP_FUNC(typemap_get);
DBGP_FUNC(property_get);
DBGP_FUNC(property_set);
DBGP_FUNC(property_value);

DBGP_FUNC(source);
DBGP_FUNC(stack_depth);
DBGP_FUNC(stack_get);
DBGP_FUNC(status);

DBGP_FUNC(stderr);
DBGP_FUNC(stdout);

DBGP_FUNC(stop);
DBGP_FUNC(run);
DBGP_FUNC(step_into);
DBGP_FUNC(step_out);
DBGP_FUNC(step_over);
DBGP_FUNC(detach);

/* Non standard comments */
DBGP_FUNC(xcmd_profiler_name_get);
DBGP_FUNC(xcmd_get_executable_lines);

/*****************************************************************************
** Dispatcher tables for supported debug commands
*/

static xdebug_dbgp_cmd dbgp_commands[] = {
	/* DBGP_FUNC_ENTRY(break) */
	DBGP_FUNC_ENTRY(breakpoint_get,    XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(breakpoint_list,   XDEBUG_DBGP_POST_MORTEM)
	DBGP_FUNC_ENTRY(breakpoint_remove, XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(breakpoint_set,    XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(breakpoint_update, XDEBUG_DBGP_NONE)

	DBGP_FUNC_ENTRY(context_get,       XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(context_names,     XDEBUG_DBGP_POST_MORTEM)

	DBGP_FUNC_ENTRY(eval,              XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(feature_get,       XDEBUG_DBGP_POST_MORTEM)
	DBGP_FUNC_ENTRY(feature_set,       XDEBUG_DBGP_NONE)

	DBGP_FUNC_ENTRY(typemap_get,       XDEBUG_DBGP_POST_MORTEM)
	DBGP_FUNC_ENTRY(property_get,      XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(property_set,      XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(property_value,    XDEBUG_DBGP_NONE)

	DBGP_FUNC_ENTRY(source,            XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(stack_depth,       XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(stack_get,         XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(status,            XDEBUG_DBGP_POST_MORTEM)

	DBGP_FUNC_ENTRY(stderr,            XDEBUG_DBGP_NONE)
	DBGP_FUNC_ENTRY(stdout,            XDEBUG_DBGP_NONE)

	DBGP_CONT_FUNC_ENTRY(run,          XDEBUG_DBGP_NONE)
	DBGP_CONT_FUNC_ENTRY(step_into,    XDEBUG_DBGP_NONE)
	DBGP_CONT_FUNC_ENTRY(step_out,     XDEBUG_DBGP_NONE)
	DBGP_CONT_FUNC_ENTRY(step_over,    XDEBUG_DBGP_NONE)

	DBGP_STOP_FUNC_ENTRY(stop,         XDEBUG_DBGP_POST_MORTEM)
	DBGP_STOP_FUNC_ENTRY(detach,       XDEBUG_DBGP_NONE)

	/* Non standard functions */
	DBGP_FUNC_ENTRY(xcmd_profiler_name_get,    XDEBUG_DBGP_POST_MORTEM)
	DBGP_FUNC_ENTRY(xcmd_get_executable_lines, XDEBUG_DBGP_NONE)
	{ NULL, NULL }
};

/*****************************************************************************
** Utility functions
*/

static xdebug_dbgp_cmd* lookup_cmd(char *cmd)
{
	xdebug_dbgp_cmd *ptr = dbgp_commands;

	while (ptr->name) {
		if (strcmp(ptr->name, cmd) == 0) {
			return ptr;
		}
		ptr++;
	}
	return NULL;
}

static xdebug_str *make_message(xdebug_con *context, xdebug_xml_node *message TSRMLS_DC)
{
	xdebug_str  xml_message = {0, 0, NULL};
	xdebug_str *ret;

	xdebug_str_ptr_init(ret);

	xdebug_xml_return_node(message, &xml_message);
	if (XG(remote_log_file)) {
		fprintf(XG(remote_log_file), "-> %s\n\n", xml_message.d);
		fflush(XG(remote_log_file));
	}

	xdebug_str_add(ret, xdebug_sprintf("%d", xml_message.l + sizeof("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n") - 1), 1);
	xdebug_str_addl(ret, "\0", 1, 0);
	xdebug_str_add(ret, "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n", 0);
	xdebug_str_add(ret, xml_message.d, 0);
	xdebug_str_addl(ret, "\0", 1, 0);
	xdebug_str_dtor(xml_message);

	return ret;
}

static void send_message(xdebug_con *context, xdebug_xml_node *message TSRMLS_DC)
{
	xdebug_str *tmp;

	tmp = make_message(context, message TSRMLS_CC);
	SSENDL(context->socket, tmp->d, tmp->l);
	xdebug_str_ptr_dtor(tmp);
}


/*****************************************************************************
** Data returning functions
*/
#define XF_ST_ROOT               0
#define XF_ST_ARRAY_INDEX_NUM    1
#define XF_ST_ARRAY_INDEX_ASSOC  2
#define XF_ST_OBJ_PROPERTY       3
#define XF_ST_STATIC_ROOT        4
#define XF_ST_STATIC_PROPERTY    5

inline static HashTable *fetch_ht_from_zval(zval *z TSRMLS_DC)
{
	switch (Z_TYPE_P(z)) {
		case IS_ARRAY:
			return Z_ARRVAL_P(z);
			break;
		case IS_OBJECT:
			return Z_OBJPROP_P(z);
			break;
	}
	return NULL;
}

inline static char *fetch_classname_from_zval(zval *z, int *length, zend_class_entry **ce TSRMLS_DC)
{
	char *name;
	zend_uint name_len;
	zend_class_entry *tmp_ce;

	if (Z_TYPE_P(z) != IS_OBJECT) {
		return NULL;
	}

	tmp_ce = zend_get_class_entry(z TSRMLS_CC);
	if (Z_OBJ_HT_P(z)->get_class_name == NULL ||
		Z_OBJ_HT_P(z)->get_class_name(z, (const char **) &name, &name_len, 0 TSRMLS_CC) != SUCCESS) {

		if (!tmp_ce) {
			return NULL;
		}

		*length = tmp_ce->name_length;
		*ce = tmp_ce;
		return estrdup(tmp_ce->name);
	} else {
		*ce = tmp_ce;
	}

	*length = name_len;
	return name;
}

static char* prepare_search_key(char *name, int *name_length, char *prefix, int prefix_length)
{
	char *element;
	int   extra_length = 0;

	if (prefix_length) {
		if (prefix[0] == '*') {
			extra_length = 3;
		} else {
			extra_length = 2 + prefix_length;
		}
	}

	element = malloc(*name_length + 1 + extra_length);
	memset(element, 0, *name_length + 1 + extra_length);
	if (extra_length) {
		memcpy(element + 1, prefix, extra_length - 2);
	}
	memcpy(element + extra_length, name, *name_length);
	*name_length += extra_length;

	return element;
}

static zval* fetch_zval_from_symbol_table(HashTable *ht, char* name, int name_length, int type, char* ccn, int ccnl, zend_class_entry *cce TSRMLS_DC)
{
	zval **retval_pp = NULL, *retval_p = NULL;
	char  *element = NULL;
	int    element_length = name_length;
#if PHP_VERSION_ID >= 50400
	zend_property_info *zpp;
#endif

	switch (type) {
		case XF_ST_STATIC_ROOT:
		case XF_ST_STATIC_PROPERTY:
#if PHP_VERSION_ID < 50400
			ht = CE_STATIC_MEMBERS(cce);
			goto continue_from_static_root;
#else
			/* First we try a public,private,protected property */
			element = prepare_search_key(name, &element_length, "", 0);
			if (cce && &cce->properties_info && zend_hash_find(&cce->properties_info, element, element_length + 1, (void **) &zpp) == SUCCESS) {
				retval_p = cce->static_members_table[zpp->offset];
				goto cleanup;
			}
			element_length = name_length;

			/* Then we try to see whether the first char is * and use the part between * and * as class name for the private property */
			if (name[0] == '*') {
				char *secondStar;
				
				secondStar = strstr(name + 1, "*");
				if (secondStar) {
					free(element);
					element_length = name_length - (secondStar + 1 - name);
					element = prepare_search_key(secondStar + 1, &element_length, "", 0);
					if (cce && &cce->properties_info && zend_hash_find(&cce->properties_info, element, element_length + 1, (void **) &zpp) == SUCCESS) {
						retval_p = cce->static_members_table[zpp->offset];
						goto cleanup;
					}
				}
			}

			break;
#endif

		case XF_ST_ROOT:
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || PHP_MAJOR_VERSION >= 6
			/* Check for compiled vars */
			element = prepare_search_key(name, &element_length, "", 0);
			if (XG(active_execute_data) && XG(active_op_array)) {
				int i = 0;
				ulong hash_value = zend_inline_hash_func(element, element_length + 1);
				zend_op_array *opa = XG(active_op_array);
				zval **CV;

				while (i < opa->last_var) {
					if (opa->vars[i].hash_value == hash_value &&
						opa->vars[i].name_len == element_length &&
						strcmp(opa->vars[i].name, element) == 0)
					{
#if PHP_VERSION_ID >= 50500
						CV = (*EX_CV_NUM(XG(active_execute_data), i));
#else
						CV = XG(active_execute_data)->CVs[i];
#endif
						if (CV) {
							retval_p = *CV;
							goto cleanup;
						}
					}
					i++;
				}
			}
			free(element);
			ht = XG(active_symbol_table);
#else
			ht = XG(active_symbol_table);
			/* break intentionally missing */
#endif
		case XF_ST_ARRAY_INDEX_ASSOC:
			element = prepare_search_key(name, &name_length, "", 0);

			/* Handle "this" in a different way */
			if (type == XF_ST_ROOT && strcmp("this", element) == 0) {
				if (XG(This)) {
					retval_p = XG(This);
				} else {
					retval_p = NULL;
				}
				goto cleanup;
			}

			if (ht && zend_hash_find(ht, element, name_length + 1, (void **) &retval_pp) == SUCCESS) {
				retval_p = *retval_pp;
				goto cleanup;
			}
			break;
		case XF_ST_ARRAY_INDEX_NUM:
			element = prepare_search_key(name, &name_length, "", 0);
			if (ht && zend_hash_index_find(ht, strtoul(element, NULL, 10), (void **) &retval_pp) == SUCCESS) {
				retval_p = *retval_pp;
				goto cleanup;
			}
			break;
		case XF_ST_OBJ_PROPERTY:
#if PHP_VERSION_ID <= 50400
continue_from_static_root:
#endif

			/* First we try a public property */
			element = prepare_search_key(name, &element_length, "", 0);
			if (ht && zend_hash_find(ht, element, element_length + 1, (void **) &retval_pp) == SUCCESS) {
				retval_p = *retval_pp;
				goto cleanup;
			}
			element_length = name_length;

			/* Then we try it again as protected property */
			free(element);
			element = prepare_search_key(name, &element_length, "*", 1);
			if (ht && zend_hash_find(ht, element, element_length + 1, (void **) &retval_pp) == SUCCESS) {
				retval_p = *retval_pp;
				goto cleanup;
			}
			element_length = name_length;

			/* Then we try it again as private property */
			free(element);
			element = prepare_search_key(name, &element_length, ccn, ccnl);
			if (ht && zend_hash_find(ht, element, element_length + 1, (void **) &retval_pp) == SUCCESS) {
				retval_p = *retval_pp;
				goto cleanup;
			}
			element_length = name_length;

			/* Then we try to see whether the first char is * and use the part between * and * as class name for the private property */
			if (name[0] == '*') {
				char *secondStar;
				
				secondStar = strstr(name + 1, "*");
				if (secondStar) {
					free(element);
					element_length = name_length - (secondStar + 1 - name);
					element = prepare_search_key(secondStar + 1, &element_length, name + 1, secondStar - name - 1);
					if (ht && zend_hash_find(ht, element, element_length + 1, (void **) &retval_pp) == SUCCESS) {
						retval_p = *retval_pp;
						goto cleanup;
					}
				}
			}

			break;
	}
cleanup:
	if (element) {
		free(element);
	}
	return retval_p;
}

static zval* get_symbol_contents_zval(char* name, int name_length TSRMLS_DC)
{
	HashTable *st = NULL;
	int        found = -1;
	int        state = 0;
	char     **p = &name;
	char      *keyword = NULL, *keyword_end = NULL;
	int        type = XF_ST_ROOT;
	zval      *retval = NULL;
	char      *current_classname = NULL;
	zend_class_entry *current_ce = NULL;
	int        cc_length = 0;
	char       quotechar = 0;

	do {
		if (*p[0] == '\0') {
			found = 0;
		} else {
			switch (state) {
				case 0:
					if (*p[0] == '$') {
						keyword = *p + 1;
						break;
					}
					if (*p[0] == ':') { /* special tricks */
						keyword = *p;
						state = 7;
						break;
					}
					keyword = *p;
					state = 1;
					/* break intentionally missing */
				case 1:
					if (*p[0] == '[') {
						keyword_end = *p;
						if (keyword) {
							retval = fetch_zval_from_symbol_table(st, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce TSRMLS_CC);
							if (current_classname) {
								efree(current_classname);
							}
							current_classname = NULL;
							current_ce = NULL;
							if (retval) {
								st = fetch_ht_from_zval(retval TSRMLS_CC);
							}
							keyword = NULL;
						}
						state = 3;
					} else if (*p[0] == '-') {
						keyword_end = *p;
						if (keyword) {
							retval = fetch_zval_from_symbol_table(st, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce TSRMLS_CC);
							if (current_classname) {
								efree(current_classname);
							}
							current_classname = NULL;
							current_ce = NULL;
							if (retval) {
								current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce TSRMLS_CC);
								st = fetch_ht_from_zval(retval TSRMLS_CC);
							}
							keyword = NULL;
						}
						state = 2;
						type = XF_ST_OBJ_PROPERTY;
					} else if (*p[0] == ':') {
						keyword_end = *p;
						if (keyword) {
							retval = fetch_zval_from_symbol_table(st, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce TSRMLS_CC);
							if (current_classname) {
								efree(current_classname);
							}
							current_classname = NULL;
							if (retval) {
								current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce TSRMLS_CC);
								st = NULL;
							}
							keyword = NULL;
						}
						state = 8;
						type = XF_ST_STATIC_PROPERTY;
					}
					break;
				case 2:
					if (*p[0] != '>') {
						keyword = *p;
						state = 1;
					}
					break;
				case 8:
					if (*p[0] != ':') {
						keyword = *p;
						state = 1;
					}
					break;
				case 3:
					/* Associative arrays */
					if (*p[0] == '\'' || *p[0] == '"') {
						state = 4;
						keyword = *p + 1;
						quotechar = *p[0];
						type = XF_ST_ARRAY_INDEX_ASSOC;
					}
					/* Numerical index */
					if (*p[0] >= '0' && *p[0] <= '9') {
						state = 6;
						keyword = *p;
						type = XF_ST_ARRAY_INDEX_NUM;
					}
					break;
				case 4:
					if (*p[0] == quotechar) {
						quotechar = 0;
						state = 5;
						keyword_end = *p;
						retval = fetch_zval_from_symbol_table(st, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce TSRMLS_CC);
						if (current_classname) {
							efree(current_classname);
						}
						current_classname = NULL;
						if (retval) {
							current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce TSRMLS_CC);
							st = fetch_ht_from_zval(retval TSRMLS_CC);
						}
						keyword = NULL;
					}
					break;
				case 5:
					if (*p[0] == ']') {
						state = 1;
					}
					break;
				case 6:
					if (*p[0] == ']') {
						state = 1;
						keyword_end = *p;
						retval = fetch_zval_from_symbol_table(st, keyword, keyword_end - keyword, type, current_classname, cc_length, current_ce TSRMLS_CC);
						if (current_classname) {
							efree(current_classname);
						}
						current_classname = NULL;
						if (retval) {
							current_classname = fetch_classname_from_zval(retval, &cc_length, &current_ce TSRMLS_CC);
							st = fetch_ht_from_zval(retval TSRMLS_CC);
						}
						keyword = NULL;
					}
					break;
				case 7: /* special cases, started with a ":" */
					if (*p[0] == ':') {
						state = 1;
						keyword_end = *p;

						if (strncmp(keyword, "::", 2) == 0) { /* static class properties */
							zend_class_entry *ce = zend_fetch_class(XG(active_fse)->function.class, strlen(XG(active_fse)->function.class), ZEND_FETCH_CLASS_SELF TSRMLS_CC);

							st = NULL;

							current_classname = estrdup(ce->name);
							cc_length = strlen(ce->name);
							current_ce = ce;
							keyword = *p + 1;

							type = XF_ST_STATIC_ROOT;
						} else {
							keyword = NULL;
						}
					}
					break;
			}
			(*p)++;
		}
	} while (found < 0);
	if (keyword != NULL) {
		retval = fetch_zval_from_symbol_table(st, keyword, *p - keyword, type, current_classname, cc_length, current_ce TSRMLS_CC);
	}
	if (current_classname) {
		efree(current_classname);
	}
	return retval;
}

static xdebug_xml_node* get_symbol(char* name, int name_length, xdebug_var_export_options *options TSRMLS_DC)
{
	zval                *retval;

	retval = get_symbol_contents_zval(name, name_length TSRMLS_CC);
	if (retval) {
		return xdebug_get_zval_value_xml_node(name, retval, options TSRMLS_CC);
	}

	return NULL;
}

static int get_symbol_contents(char* name, int name_length, xdebug_xml_node *node, xdebug_var_export_options *options TSRMLS_DC)
{
	zval                *retval;

	retval = get_symbol_contents_zval(name, name_length TSRMLS_CC);
	if (retval) {
		xdebug_var_export_xml_node(&retval, name, node, options, 1 TSRMLS_CC);
		return 1;
	}

	return 0;
}

static char* return_file_source(char *filename, int begin, int end TSRMLS_DC)
{
	php_stream *stream;
	int    i = begin;
	char  *line = NULL;
	xdebug_str source = { 0, 0, NULL };

	if (i < 0) {
		begin = 0;
		i = 0;
	}

	filename = xdebug_path_from_url(filename TSRMLS_CC);
	stream = php_stream_open_wrapper(filename, "rb",
			USE_PATH | ENFORCE_SAFE_MODE | REPORT_ERRORS,
			NULL);
	xdfree(filename);

	/* Read until the "begin" line has been read */
	if (!stream) {
		return NULL;
	}

	/* skip to the first requested line */
	while (i > 0 && !php_stream_eof(stream)) {
		if (line) {
			efree(line);
			line = NULL;
		}
		line = php_stream_gets(stream, NULL, 1024);
		i--;
	}
	/* Read until the "end" line has been read */
	do {
		if (line) {
			xdebug_str_add(&source, line, 0);
			efree(line);
			line = NULL;
			if (php_stream_eof(stream)) break;
		}
		line = php_stream_gets(stream, NULL, 1024);
		i++;
	} while (i < end + 1 - begin);

	/* Print last line */
	if (line) {
		efree(line);
		line = NULL;
	}
	php_stream_close(stream);
	return source.d;
}

static char* return_eval_source(char *id, int begin, int end TSRMLS_DC)
{
	char             *key, *joined;
	xdebug_eval_info *ei;
	xdebug_arg       *parts = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));

	if (begin < 0) {
		begin = 0;
	}
	key = create_eval_key_id(atoi(id));
	if (xdebug_hash_find(XG(context).eval_id_lookup, key, strlen(key), (void *) &ei)) {
		xdebug_arg_init(parts);
		xdebug_explode("\n", ei->contents, parts, end + 2);
		joined = xdebug_join("\n", parts, begin, end);
		xdebug_arg_dtor(parts);
		return joined;
	}
	return NULL;
}

static char* return_source(char *filename, int begin, int end TSRMLS_DC)
{
	if (strncmp(filename, "dbgp://", 7) == 0) {
		return return_eval_source(filename + 7, begin, end TSRMLS_CC);
	} else {
		return return_file_source(filename, begin, end TSRMLS_CC);
	}
}


static int check_evaled_code(function_stack_entry *fse, char **filename, int *lineno, int use_fse TSRMLS_DC)
{
	char *end_marker;
	xdebug_eval_info *ei;
	char *filename_to_use;

	filename_to_use = use_fse ? fse->filename : *filename;

	end_marker = filename_to_use + strlen(filename_to_use) - strlen("eval()'d code");
	if (end_marker >= filename_to_use && strcmp("eval()'d code", end_marker) == 0) {
		if (xdebug_hash_find(XG(context).eval_id_lookup, filename_to_use, strlen(filename_to_use), (void *) &ei)) {
			*filename = xdebug_sprintf("dbgp://%lu", ei->id);
		}
		return 1;
	}
	return 0;
}

static xdebug_xml_node* return_stackframe(int nr TSRMLS_DC)
{
	function_stack_entry *fse, *fse_prev;
	char                 *tmp_fname;
	char                 *tmp_filename;
	int                   tmp_lineno;
	xdebug_xml_node      *tmp;

	fse = xdebug_get_stack_frame(nr TSRMLS_CC);
	fse_prev = xdebug_get_stack_frame(nr - 1 TSRMLS_CC);

	tmp_fname = xdebug_show_fname(fse->function, 0, 0 TSRMLS_CC);

	tmp = xdebug_xml_node_init("stack");
	xdebug_xml_add_attribute_ex(tmp, "where", xdstrdup(tmp_fname), 0, 1);
	xdebug_xml_add_attribute_ex(tmp, "level", xdebug_sprintf("%ld", nr), 0, 1);
	if (fse_prev) {
		if (check_evaled_code(fse_prev, &tmp_filename, &tmp_lineno, 1 TSRMLS_CC)) {
			xdebug_xml_add_attribute_ex(tmp, "type",     xdstrdup("eval"), 0, 1);
			xdebug_xml_add_attribute_ex(tmp, "filename", tmp_filename, 0, 0);
		} else {
			xdebug_xml_add_attribute_ex(tmp, "type",     xdstrdup("file"), 0, 1);
			xdebug_xml_add_attribute_ex(tmp, "filename", xdebug_path_to_url(fse_prev->filename TSRMLS_CC), 0, 1);
		}
		xdebug_xml_add_attribute_ex(tmp, "lineno",   xdebug_sprintf("%lu", fse_prev->lineno TSRMLS_CC), 0, 1);
	} else {
		tmp_filename = (char *) zend_get_executed_filename(TSRMLS_C);
		tmp_lineno = zend_get_executed_lineno(TSRMLS_C);
		if (check_evaled_code(fse, &tmp_filename, &tmp_lineno, 0 TSRMLS_CC)) {
			xdebug_xml_add_attribute_ex(tmp, "type", xdstrdup("eval"), 0, 1);
			xdebug_xml_add_attribute_ex(tmp, "filename", tmp_filename, 0, 0);
		} else {
			xdebug_xml_add_attribute_ex(tmp, "type", xdstrdup("file"), 0, 1);
			xdebug_xml_add_attribute_ex(tmp, "filename", xdebug_path_to_url(tmp_filename TSRMLS_CC), 0, 1);
		}
		xdebug_xml_add_attribute_ex(tmp, "lineno",   xdebug_sprintf("%lu", tmp_lineno), 0, 1);
	}

	xdfree(tmp_fname);
	return tmp;
}

/*****************************************************************************
** Client command handlers - Breakpoints
*/

/* Helper functions */
void xdebug_hash_admin_dtor(xdebug_brk_admin *admin)
{
	xdfree(admin->key);
	xdfree(admin);
}

static int breakpoint_admin_add(xdebug_con *context, int type, char *key)
{
	xdebug_brk_admin *admin = xdmalloc(sizeof(xdebug_brk_admin));
	char             *hkey;
	TSRMLS_FETCH();

	XG(breakpoint_count)++;
	admin->id   = getpid() * 10000 + XG(breakpoint_count);
	admin->type = type;
	admin->key  = xdstrdup(key);

	hkey = xdebug_sprintf("%lu", admin->id);
	xdebug_hash_add(context->breakpoint_list, hkey, strlen(hkey), (void*) admin);
	xdfree(hkey);

	return admin->id;
}

static int breakpoint_admin_fetch(xdebug_con *context, char *hkey, int *type, char **key)
{
	xdebug_brk_admin *admin;

	if (xdebug_hash_find(context->breakpoint_list, hkey, strlen(hkey), (void *) &admin)) {
		*type = admin->type;
		*key  = admin->key;
		return SUCCESS;
	} else {
		return FAILURE;
	}

}

static int breakpoint_admin_remove(xdebug_con *context, char *hkey)
{
	if (xdebug_hash_delete(context->breakpoint_list, hkey, strlen(hkey))) {
		return SUCCESS;
	} else {
		return FAILURE;
	}
}

static void breakpoint_brk_info_add(xdebug_xml_node *xml, xdebug_brk_info *brk)
{
	TSRMLS_FETCH();

	if (brk->type) {
		xdebug_xml_add_attribute_ex(xml, "type", xdstrdup(brk->type), 0, 1);
	}
	if (brk->file) {
		xdebug_xml_add_attribute_ex(xml, "filename", xdebug_path_to_url(brk->file TSRMLS_CC), 0, 1);
	}
	if (brk->lineno) {
		xdebug_xml_add_attribute_ex(xml, "lineno", xdebug_sprintf("%lu", brk->lineno), 0, 1);
	}
	if (brk->functionname) {
		xdebug_xml_add_attribute_ex(xml, "function", xdstrdup(brk->functionname), 0, 1);
	}
	if (brk->classname) {
		xdebug_xml_add_attribute_ex(xml, "class", xdstrdup(brk->classname), 0, 1);
	}
	if (brk->temporary) {
		xdebug_xml_add_attribute(xml, "state", "temporary");
	} else if (brk->disabled) {
		xdebug_xml_add_attribute(xml, "state", "disabled");
	} else {
		xdebug_xml_add_attribute(xml, "state", "enabled");
	}
	xdebug_xml_add_attribute_ex(xml, "hit_count", xdebug_sprintf("%lu", brk->hit_count), 0, 1);
	switch (brk->hit_condition) {
		case XDEBUG_HIT_GREATER_EQUAL:
			xdebug_xml_add_attribute(xml, "hit_condition", ">=");
			break;
		case XDEBUG_HIT_EQUAL:
			xdebug_xml_add_attribute(xml, "hit_condition", "==");
			break;
		case XDEBUG_HIT_MOD:
			xdebug_xml_add_attribute(xml, "hit_condition", "%");
			break;
	}
	if (brk->condition) {
		xdebug_xml_node *condition = xdebug_xml_node_init("expression");
		xdebug_xml_add_text_ex(condition, brk->condition, strlen(brk->condition), 0, 1);
		xdebug_xml_add_child(xml, condition);
	}
	xdebug_xml_add_attribute_ex(xml, "hit_value", xdebug_sprintf("%lu", brk->hit_value), 0, 1);
}

static xdebug_brk_info* breakpoint_brk_info_fetch(int type, char *hkey)
{
	xdebug_llist_element *le;
	xdebug_brk_info      *brk = NULL;
	xdebug_arg           *parts = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));

	TSRMLS_FETCH();

	switch (type) {
		case BREAKPOINT_TYPE_LINE:
			/* First we split the key into filename and linenumber */
			xdebug_arg_init(parts);
			xdebug_explode("$", hkey, parts, -1);

			/* Second we loop through the list of file/line breakpoints to
			 * look for our thingy */
			for (le = XDEBUG_LLIST_HEAD(XG(context).line_breakpoints); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
				brk = XDEBUG_LLIST_VALP(le);

				if (atoi(parts->args[1]) == brk->lineno && memcmp(brk->file, parts->args[0], brk->file_len) == 0) {
					xdebug_arg_dtor(parts);
					return brk;
				}
			}

			/* Cleaning up */
			xdebug_arg_dtor(parts);
			break;

		case BREAKPOINT_TYPE_FUNCTION:
			if (xdebug_hash_find(XG(context).function_breakpoints, hkey, strlen(hkey), (void *) &brk)) {
				return brk;
			}
			break;

		case BREAKPOINT_TYPE_EXCEPTION:
			if (xdebug_hash_find(XG(context).exception_breakpoints, hkey, strlen(hkey), (void *) &brk)) {
				return brk;
			}
			break;
	}
	return brk;
}

static int breakpoint_remove(int type, char *hkey)
{
	xdebug_llist_element *le;
	xdebug_brk_info      *brk = NULL;
	xdebug_arg           *parts = (xdebug_arg*) xdmalloc(sizeof(xdebug_arg));
	int                   retval = FAILURE;
	TSRMLS_FETCH();

	switch (type) {
		case BREAKPOINT_TYPE_LINE:
			/* First we split the key into filename and linenumber */
			xdebug_arg_init(parts);
			xdebug_explode("$", hkey, parts, -1);

			/* Second we loop through the list of file/line breakpoints to
			 * look for our thingy */
			for (le = XDEBUG_LLIST_HEAD(XG(context).line_breakpoints); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
				brk = XDEBUG_LLIST_VALP(le);

				if (atoi(parts->args[1]) == brk->lineno && memcmp(brk->file, parts->args[0], brk->file_len) == 0) {
					xdebug_llist_remove(XG(context).line_breakpoints, le, NULL);
					retval = SUCCESS;
					break;
				}
			}

			/* Cleaning up */
			xdebug_arg_dtor(parts);
			break;

		case BREAKPOINT_TYPE_FUNCTION:
			if (xdebug_hash_delete(XG(context).function_breakpoints, hkey, strlen(hkey))) {
				retval = SUCCESS;
			}
			break;

		case BREAKPOINT_TYPE_EXCEPTION:
			if (xdebug_hash_delete(XG(context).exception_breakpoints, hkey, strlen(hkey))) {
				retval = SUCCESS;
			}
			break;
	}
	return retval;
}

#define BREAKPOINT_ACTION_GET       1
#define BREAKPOINT_ACTION_REMOVE    2
#define BREAKPOINT_ACTION_UPDATE    3

#define BREAKPOINT_CHANGE_STATE() \
	XDEBUG_STR_SWITCH(CMD_OPTION('s')) { \
		XDEBUG_STR_CASE("enabled") \
			brk_info->disabled = 0; \
		XDEBUG_STR_CASE_END \
 \
		XDEBUG_STR_CASE("disabled") \
			brk_info->disabled = 1; \
		XDEBUG_STR_CASE_END \
 \
		XDEBUG_STR_CASE_DEFAULT \
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS); \
		XDEBUG_STR_CASE_DEFAULT_END \
	}

#define BREAKPOINT_CHANGE_OPERATOR() \
	XDEBUG_STR_SWITCH(CMD_OPTION('o')) { \
		XDEBUG_STR_CASE(">=") \
			brk_info->hit_condition = XDEBUG_HIT_GREATER_EQUAL; \
		XDEBUG_STR_CASE_END \
 \
		XDEBUG_STR_CASE("==") \
			brk_info->hit_condition = XDEBUG_HIT_EQUAL; \
		XDEBUG_STR_CASE_END \
 \
		XDEBUG_STR_CASE("%") \
			brk_info->hit_condition = XDEBUG_HIT_MOD; \
		XDEBUG_STR_CASE_END \
 \
		XDEBUG_STR_CASE_DEFAULT \
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS); \
		XDEBUG_STR_CASE_DEFAULT_END \
	}



static void breakpoint_do_action(DBGP_FUNC_PARAMETERS, int action)
{
	int                   type;
	char                 *hkey;
	xdebug_brk_info      *brk_info;
	xdebug_xml_node      *breakpoint_node;
	XDEBUG_STR_SWITCH_DECL;

	if (!CMD_OPTION('d')) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}
	/* Lets check if it exists */
	if (breakpoint_admin_fetch(context, CMD_OPTION('d'), &type, (char**) &hkey) == SUCCESS) {
		/* so it exists, now we're going to find it in the correct hash/list
		 * and return the info we have on it */
		brk_info = breakpoint_brk_info_fetch(type, hkey);

		if (action == BREAKPOINT_ACTION_UPDATE) {
			if (CMD_OPTION('s')) {
				BREAKPOINT_CHANGE_STATE();
			}
			if (CMD_OPTION('n')) {
				brk_info->lineno = strtol(CMD_OPTION('n'), NULL, 10);
			}
			if (CMD_OPTION('h')) {
				brk_info->hit_value = strtol(CMD_OPTION('h'), NULL, 10);
			}
			if (CMD_OPTION('o')) {
				BREAKPOINT_CHANGE_OPERATOR();
			}
		}

		breakpoint_node = xdebug_xml_node_init("breakpoint");
		breakpoint_brk_info_add(breakpoint_node, brk_info);
		xdebug_xml_add_attribute_ex(breakpoint_node, "id", xdstrdup(CMD_OPTION('d')), 0, 1);
		xdebug_xml_add_child(*retval, breakpoint_node);

		if (action == BREAKPOINT_ACTION_REMOVE) {
			/* Now we remove the crap */
			breakpoint_remove(type, hkey);
			breakpoint_admin_remove(context, CMD_OPTION('d'));
		}
	} else {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_NO_SUCH_BREAKPOINT)
	}
}

DBGP_FUNC(breakpoint_get)
{
	breakpoint_do_action(DBGP_FUNC_PASS_PARAMETERS, BREAKPOINT_ACTION_GET);
}

DBGP_FUNC(breakpoint_remove)
{
	breakpoint_do_action(DBGP_FUNC_PASS_PARAMETERS, BREAKPOINT_ACTION_REMOVE);
}

DBGP_FUNC(breakpoint_update)
{
	breakpoint_do_action(DBGP_FUNC_PASS_PARAMETERS, BREAKPOINT_ACTION_UPDATE);
}


static void breakpoint_list_helper(void *xml, xdebug_hash_element *he)
{
	xdebug_xml_node  *xml_node = (xdebug_xml_node*) xml;
	xdebug_xml_node  *child;
	xdebug_brk_admin *admin = (xdebug_brk_admin*) he->ptr;
	xdebug_brk_info  *brk;

	child = xdebug_xml_node_init("breakpoint");
	brk = breakpoint_brk_info_fetch(admin->type, admin->key);
	breakpoint_brk_info_add(child, brk);
	xdebug_xml_add_attribute_ex(child, "id", xdebug_sprintf("%lu", admin->id), 0, 1);
	xdebug_xml_add_child(xml_node, child);
}

DBGP_FUNC(breakpoint_list)
{
	xdebug_hash_apply(context->breakpoint_list, (void *) *retval, breakpoint_list_helper);
}

DBGP_FUNC(breakpoint_set)
{
	xdebug_brk_info      *brk_info;
	char                 *tmp_name;
	int                   brk_id = 0;
	int                   new_length = 0;
	function_stack_entry *fse;
	XDEBUG_STR_SWITCH_DECL;

	brk_info = xdmalloc(sizeof(xdebug_brk_info));
	brk_info->type = NULL;
	brk_info->file = NULL;
	brk_info->file_len = 0;
	brk_info->lineno = 0;
	brk_info->classname = NULL;
	brk_info->functionname = NULL;
	brk_info->function_break_type = 0;
	brk_info->exceptionname = NULL;
	brk_info->condition = NULL;
	brk_info->disabled = 0;
	brk_info->temporary = 0;
	brk_info->hit_count = 0;
	brk_info->hit_value = 0;
	brk_info->hit_condition = XDEBUG_HIT_DISABLED;

	if (!CMD_OPTION('t')) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
	} else {
		brk_info->type = xdstrdup(CMD_OPTION('t'));
	}

	if (CMD_OPTION('s')) {
		BREAKPOINT_CHANGE_STATE();
		xdebug_xml_add_attribute_ex(*retval, "state", xdstrdup(CMD_OPTION('s')), 0, 1);
	}
	if (CMD_OPTION('o') && CMD_OPTION('h')) {
		BREAKPOINT_CHANGE_OPERATOR();
		brk_info->hit_value = strtol(CMD_OPTION('h'), NULL, 10);
	}
	if (CMD_OPTION('r')) {
		brk_info->temporary = strtol(CMD_OPTION('r'), NULL, 10);
	}

	if ((strcmp(CMD_OPTION('t'), "line") == 0) || (strcmp(CMD_OPTION('t'), "conditional") == 0)) {
		if (!CMD_OPTION('n')) {
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
		}
		brk_info->lineno = strtol(CMD_OPTION('n'), NULL, 10);

		/* If no filename is given, we use the current one */
		if (!CMD_OPTION('f')) {
			fse = xdebug_get_stack_tail(TSRMLS_C);
			if (!fse) {
				RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
			} else {
				brk_info->file = xdebug_path_from_url(fse->filename TSRMLS_CC);
				brk_info->file_len = strlen(brk_info->file);
			}
		} else {
			char realpath_file[MAXPATHLEN];

			brk_info->file = xdebug_path_from_url(CMD_OPTION('f') TSRMLS_CC);

			/* Now we do some real path checks to resolve symlinks. */
			if (VCWD_REALPATH(brk_info->file, realpath_file)) {
				xdfree(brk_info->file);
				brk_info->file = xdstrdup(realpath_file);
			}

			brk_info->file_len = strlen(brk_info->file);
		}

		/* Perhaps we have a break condition */
		if (CMD_OPTION('-')) {
			brk_info->condition = (char*) xdebug_base64_decode((unsigned char*) CMD_OPTION('-'), strlen(CMD_OPTION('-')), &new_length); 
		}

		tmp_name = xdebug_sprintf("%s$%lu", brk_info->file, brk_info->lineno);
		brk_id = breakpoint_admin_add(context, BREAKPOINT_TYPE_LINE, tmp_name);
		xdfree(tmp_name);
		xdebug_llist_insert_next(context->line_breakpoints, XDEBUG_LLIST_TAIL(context->line_breakpoints), (void*) brk_info);
	} else 

	if ((strcmp(CMD_OPTION('t'), "call") == 0) || (strcmp(CMD_OPTION('t'), "return") == 0)) {
		if (strcmp(CMD_OPTION('t'), "call") == 0) {
			brk_info->function_break_type = XDEBUG_BRK_FUNC_CALL;
		} else {
			brk_info->function_break_type = XDEBUG_BRK_FUNC_RETURN;
		}

		if (!CMD_OPTION('m')) {
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
		}
		brk_info->functionname = xdstrdup(CMD_OPTION('m'));
		if (CMD_OPTION('a')) {
			int   res;

			brk_info->classname = xdstrdup(CMD_OPTION('a'));
			tmp_name = xdebug_sprintf("%s::%s", CMD_OPTION('a'), CMD_OPTION('m'));
			res = xdebug_hash_add(context->function_breakpoints, tmp_name, strlen(tmp_name), (void*) brk_info);
			brk_id = breakpoint_admin_add(context, BREAKPOINT_TYPE_FUNCTION, tmp_name);
			xdfree(tmp_name);

			if (!res) {
				RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
			}
		} else {
			if (!xdebug_hash_add(context->function_breakpoints, CMD_OPTION('m'), strlen(CMD_OPTION('m')), (void*) brk_info)) {
				RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_BREAKPOINT_NOT_SET);
			} else {
				brk_id = breakpoint_admin_add(context, BREAKPOINT_TYPE_FUNCTION, CMD_OPTION('m'));
			}
		}
	} else

	if (strcmp(CMD_OPTION('t'), "exception") == 0) {
		if (!CMD_OPTION('x')) {
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
		}
		brk_info->exceptionname = xdstrdup(CMD_OPTION('x'));
		if (!xdebug_hash_add(context->exception_breakpoints, CMD_OPTION('x'), strlen(CMD_OPTION('x')), (void*) brk_info)) {
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_BREAKPOINT_NOT_SET);
		} else {
			brk_id = breakpoint_admin_add(context, BREAKPOINT_TYPE_EXCEPTION, CMD_OPTION('x'));
		}
	} else

	if (strcmp(CMD_OPTION('t'), "watch") == 0) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_BREAKPOINT_TYPE_NOT_SUPPORTED);
	}

	xdebug_xml_add_attribute_ex(*retval, "id", xdebug_sprintf("%d", brk_id), 0, 1);
}

static int xdebug_do_eval(char *eval_string, zval *ret_zval TSRMLS_DC)
{
	int                old_error_reporting;
	int                res = FAILURE;
	zval             **original_return_value_ptr_ptr = EG(return_value_ptr_ptr);
	zend_op          **original_opline_ptr = EG(opline_ptr);
	zend_op_array     *original_active_op_array = EG(active_op_array);
	zend_execute_data *original_execute_data = EG(current_execute_data);
	int                original_no_extensions = EG(no_extensions);
#if PHP_VERSION_ID < 50200
	zend_bool          original_bailout_set = EG(bailout_set);
	jmp_buf            original_bailout;
#else
	jmp_buf           *original_bailout = EG(bailout);
#endif
#if PHP_VERSION_ID >= 50300
	void             **original_argument_stack_top = EG(argument_stack)->top;
	void             **original_argument_stack_end = EG(argument_stack)->end;
#endif

#if PHP_VERSION_ID < 50200
	memcpy(&original_bailout, &EG(bailout), sizeof(jmp_buf));
#endif

	/* Remember error reporting level */
	old_error_reporting = EG(error_reporting);
	EG(error_reporting) = 0;

	/* Do evaluation */
	XG(breakpoints_allowed) = 0;

	zend_first_try {
		res = zend_eval_string(eval_string, ret_zval, "xdebug://debug-eval" TSRMLS_CC);
	} zend_end_try();

	/* Clean up */
	EG(error_reporting) = old_error_reporting;
	XG(breakpoints_allowed) = 1;

	EG(return_value_ptr_ptr) = original_return_value_ptr_ptr;
	EG(opline_ptr) = original_opline_ptr;
	EG(active_op_array) = original_active_op_array;
	EG(current_execute_data) = original_execute_data;
	EG(no_extensions) = original_no_extensions;
#if PHP_VERSION_ID < 50200
	EG(bailout_set) = original_bailout_set;
	memcpy(&EG(bailout), &original_bailout, sizeof(jmp_buf));
#else
	EG(bailout) = original_bailout;
#endif
#if PHP_VERSION_ID >= 50300
	EG(argument_stack)->top = original_argument_stack_top;
	EG(argument_stack)->end = original_argument_stack_end;
#endif

	return res;
}

DBGP_FUNC(eval)
{
	char            *eval_string;
	xdebug_xml_node *ret_xml;
	zval             ret_zval;
	int              new_length;
	int              res;
	xdebug_var_export_options *options;

	if (!CMD_OPTION('-')) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	options = (xdebug_var_export_options*) context->options;
	
	if (CMD_OPTION('p')) {
		options->runtime[0].page = strtol(CMD_OPTION('p'), NULL, 10);
	} else {
		options->runtime[0].page = 0;
	}

	/* base64 decode eval string */
	eval_string = (char*) xdebug_base64_decode((unsigned char*) CMD_OPTION('-'), strlen(CMD_OPTION('-')), &new_length);

	res = xdebug_do_eval(eval_string, &ret_zval TSRMLS_CC);

	efree(eval_string);

	/* Handle result */
	if (res == FAILURE) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_EVALUATING_CODE);
	} else {
		ret_xml = xdebug_get_zval_value_xml_node(NULL, &ret_zval, options TSRMLS_CC);
		xdebug_xml_add_child(*retval, ret_xml);
		zval_dtor(&ret_zval);
	}
}

/* these functions interupt PHP's output functions, so we can
   redirect to our remote debugger! */
static int xdebug_send_stream(const char *name, const char *str, uint str_length TSRMLS_DC)
{
	/* create an xml document to send as the stream */
	xdebug_xml_node *message;

	message = xdebug_xml_node_init("stream");
	xdebug_xml_add_attribute(message, "xmlns", "urn:debugger_protocol_v1");
	xdebug_xml_add_attribute(message, "xmlns:xdebug", "http://xdebug.org/dbgp/xdebug");
	xdebug_xml_add_attribute_ex(message, "type", (char *)name, 0, 0);
	xdebug_xml_add_text_encodel(message, xdstrndup(str, str_length), str_length);
	send_message(&XG(context), message TSRMLS_CC);
	xdebug_xml_node_dtor(message);

	return 0;
}


DBGP_FUNC(stderr)
{
	xdebug_xml_add_attribute(*retval, "success", "0");
}

DBGP_FUNC(stdout)
{
	int mode = 0;
	char *success = "0";

	if (!CMD_OPTION('c')) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	mode = strtol(CMD_OPTION('c'), NULL, 10);
	XG(stdout_mode) = mode;
	success = "1";

	xdebug_xml_add_attribute_ex(*retval, "success", xdstrdup(success), 0, 1);
}

DBGP_FUNC(stop)
{
	XG(status) = DBGP_STATUS_STOPPED;
	xdebug_xml_add_attribute(*retval, "status", xdebug_dbgp_status_strings[XG(status)]);
	xdebug_xml_add_attribute(*retval, "reason", xdebug_dbgp_reason_strings[XG(reason)]);
}

DBGP_FUNC(run)
{
	xdebug_xml_add_attribute_ex(*retval, "filename", xdstrdup(context->program_name), 0, 1);
}

DBGP_FUNC(step_into)
{
	XG(context).do_next   = 0;
	XG(context).do_step   = 1;
	XG(context).do_finish = 0;
}

DBGP_FUNC(step_out)
{
	function_stack_entry *fse;

	XG(context).do_next   = 0;
	XG(context).do_step   = 0;
	XG(context).do_finish = 1;

	if ((fse = xdebug_get_stack_tail(TSRMLS_C))) {
		XG(context).next_level = fse->level - 1;
	} else {
		XG(context).next_level = -1;
	}
}

DBGP_FUNC(step_over)
{
	function_stack_entry *fse;

	XG(context).do_next   = 1;
	XG(context).do_step   = 0;
	XG(context).do_finish = 0;

	if ((fse = xdebug_get_stack_tail(TSRMLS_C))) {
		XG(context).next_level = fse->level;
	} else {
		XG(context).next_level = 0;
	}
}

DBGP_FUNC(detach)
{
	XG(status) = DBGP_STATUS_DETACHED;
	xdebug_xml_add_attribute(*retval, "status", xdebug_dbgp_status_strings[DBGP_STATUS_STOPPED]);
	xdebug_xml_add_attribute(*retval, "reason", xdebug_dbgp_reason_strings[XG(reason)]);
	XG(context).handler->remote_deinit(&(XG(context)));
	XG(remote_enabled) = 0;
	XG(stdout_mode) = 0;
}


DBGP_FUNC(source)
{
	char *source;
	int   begin = 0, end = 999999;
	char *filename;
	function_stack_entry *fse;

	if (!CMD_OPTION('f')) {
		if ((fse = xdebug_get_stack_tail(TSRMLS_C))) {
			filename = fse->filename;
		} else {
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
		}
	} else {
		filename = CMD_OPTION('f');
	}

	if (CMD_OPTION('b')) {
		begin = strtol(CMD_OPTION('b'), NULL, 10);
	}
	if (CMD_OPTION('e')) {
		end = strtol(CMD_OPTION('e'), NULL, 10);
	}

	/* return_source allocates memory for source */
	XG(breakpoints_allowed) = 0;
	source = return_source(filename, begin, end TSRMLS_CC);
	XG(breakpoints_allowed) = 1;

	if (!source) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_CANT_OPEN_FILE);
	} else {
		xdebug_xml_add_text_encode(*retval, source);
	}
}

DBGP_FUNC(feature_get)
{
	xdebug_var_export_options *options;
	XDEBUG_STR_SWITCH_DECL;

	options = (xdebug_var_export_options*) context->options;

	if (!CMD_OPTION('n')) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}
	xdebug_xml_add_attribute_ex(*retval, "feature_name", xdstrdup(CMD_OPTION('n')), 0, 1);

	XDEBUG_STR_SWITCH(CMD_OPTION('n')) {
		XDEBUG_STR_CASE("breakpoint_languages")
			xdebug_xml_add_attribute(*retval, "supported", "0");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("breakpoint_types")
			xdebug_xml_add_text(*retval, xdstrdup("line conditional call return exception"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("data_encoding")
			xdebug_xml_add_attribute(*retval, "supported", "0");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("encoding")
			xdebug_xml_add_text(*retval, xdstrdup("iso-8859-1"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("language_name")
			xdebug_xml_add_text(*retval, xdstrdup("PHP"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("language_supports_threads")
			xdebug_xml_add_text(*retval, xdstrdup("0"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END
		
		XDEBUG_STR_CASE("language_version")
			xdebug_xml_add_text(*retval, xdstrdup(PHP_VERSION));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("max_children")
			xdebug_xml_add_text(*retval, xdebug_sprintf("%ld", options->max_children));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("max_data")
			xdebug_xml_add_text(*retval, xdebug_sprintf("%ld", options->max_data));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("max_depth")
			xdebug_xml_add_text(*retval, xdebug_sprintf("%ld", options->max_depth));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("protocol_version")
			xdebug_xml_add_text(*retval, xdstrdup(DBGP_VERSION));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("supported_encodings")
			xdebug_xml_add_text(*retval, xdstrdup("iso-8859-1"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("supports_async")
			xdebug_xml_add_text(*retval, xdstrdup("0"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("supports_postmortem")
			xdebug_xml_add_text(*retval, xdstrdup("1"));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("show_hidden")
			xdebug_xml_add_text(*retval, xdebug_sprintf("%ld", options->show_hidden));
			xdebug_xml_add_attribute(*retval, "supported", "1");
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE_DEFAULT
			xdebug_xml_add_text(*retval, xdstrdup(lookup_cmd(CMD_OPTION('n')) ? "1" : "0"));
			xdebug_xml_add_attribute(*retval, "supported", lookup_cmd(CMD_OPTION('n')) ? "1" : "0");
		XDEBUG_STR_CASE_DEFAULT_END
	}
}

DBGP_FUNC(feature_set)
{
	xdebug_var_export_options *options;
	XDEBUG_STR_SWITCH_DECL;

	options = (xdebug_var_export_options*) context->options;

	if (!CMD_OPTION('n') || !CMD_OPTION('v')) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	XDEBUG_STR_SWITCH(CMD_OPTION('n')) {

		XDEBUG_STR_CASE("encoding")
			if (strcmp(CMD_OPTION('v'), "iso-8859-1") != 0) {
				RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_ENCODING_NOT_SUPPORTED);
			}
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("max_children")
			options->max_children = strtol(CMD_OPTION('v'), NULL, 10);
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("max_data")
			options->max_data = strtol(CMD_OPTION('v'), NULL, 10);
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("max_depth")
			int i;
			options->max_depth = strtol(CMD_OPTION('v'), NULL, 10);

			/* Reallocating page structure */
			xdfree(options->runtime);
			options->runtime = (xdebug_var_runtime_page*) xdmalloc(options->max_depth * sizeof(xdebug_var_runtime_page));
			for (i = 0; i < options->max_depth; i++) {
				options->runtime[i].page = 0;
				options->runtime[i].current_element_nr = 0;
			}
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("show_hidden")
			options->show_hidden = strtol(CMD_OPTION('v'), NULL, 10);
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE("multiple_sessions")
			/* FIXME: Add new boolean option check / struct field for this */
		XDEBUG_STR_CASE_END

		XDEBUG_STR_CASE_DEFAULT
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
		XDEBUG_STR_CASE_DEFAULT_END
	}
	xdebug_xml_add_attribute_ex(*retval, "feature", xdstrdup(CMD_OPTION('n')), 0, 1);
	xdebug_xml_add_attribute_ex(*retval, "success", "1", 0, 0);
}

DBGP_FUNC(typemap_get)
{
	int              i;
	xdebug_xml_node *type;

	xdebug_xml_add_attribute(*retval, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	xdebug_xml_add_attribute(*retval, "xmlns:xsd", "http://www.w3.org/2001/XMLSchema");

	/* Add our basic types */
	for (i = 0; i < XDEBUG_TYPES_COUNT; i++) {
		type = xdebug_xml_node_init("map");
		xdebug_xml_add_attribute(type, "name", xdebug_dbgp_typemap[i][1]);
		xdebug_xml_add_attribute(type, "type", xdebug_dbgp_typemap[i][0]);
		if (xdebug_dbgp_typemap[i][2]) {
			xdebug_xml_add_attribute(type, "xsi:type", xdebug_dbgp_typemap[i][2]);
		}
		xdebug_xml_add_child(*retval, type);
	}
}

static int add_variable_node(xdebug_xml_node *node, char *name, int name_length, int var_only, int non_null, int no_eval, xdebug_var_export_options *options TSRMLS_DC)
{
	xdebug_xml_node *contents;

	contents = get_symbol(name, name_length, options TSRMLS_CC);
	if (contents) {
		xdebug_xml_add_child(node, contents);
		return SUCCESS;
	}
	return FAILURE;
}


DBGP_FUNC(property_get)
{
	int                        depth = 0;
	int                        context_nr = 0;
	function_stack_entry      *fse;
	int                        old_max_data;
	xdebug_var_export_options *options = (xdebug_var_export_options*) context->options;

	if (!CMD_OPTION('n')) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	if (CMD_OPTION('d')) {
		depth = strtol(CMD_OPTION('d'), NULL, 10);
	}

	if (CMD_OPTION('c')) {
		context_nr = strtol(CMD_OPTION('c'), NULL, 10);
	}

	/* Set the symbol table corresponding with the requested stack depth */
	if (context_nr == 0) { /* locals */
		if ((fse = xdebug_get_stack_frame(depth TSRMLS_CC))) {
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || PHP_MAJOR_VERSION >= 6
			function_stack_entry *old_fse = xdebug_get_stack_frame(depth - 1 TSRMLS_CC);

			if (depth > 0) {
				XG(active_execute_data) = old_fse->execute_data;
			} else {
				XG(active_execute_data) = EG(current_execute_data);
			}
#else
			XG(active_execute_data) = fse->execute_data;
#endif
			XG(active_symbol_table) = fse->symbol_table;
			XG(active_op_array)     = fse->op_array;
			XG(This)                = fse->This;
			XG(active_fse)          = fse;
		} else {
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
		}
	} else { /* superglobals */
		XG(active_symbol_table) = &EG(symbol_table);
	}

	if (CMD_OPTION('p')) {
		options->runtime[0].page = strtol(CMD_OPTION('p'), NULL, 10);
	} else {
		options->runtime[0].page = 0;
	}

	/* Override max data size if necessary */
	old_max_data = options->max_data;
	if (CMD_OPTION('m')) {
		options->max_data= strtol(CMD_OPTION('m'), NULL, 10);
	}
	if (add_variable_node(*retval, CMD_OPTION('n'), strlen(CMD_OPTION('n')) + 1, 1, 0, 0, options TSRMLS_CC) == FAILURE) {
		options->max_data = old_max_data;
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_PROPERTY_NON_EXISTANT);
	}
	XG(active_op_array) = NULL;
}

DBGP_FUNC(property_set)
{
	char                      *data = CMD_OPTION('-');
	char                      *new_value;
	int                        new_length;
	int                        depth = 0;
	int                        context_nr = 0;
	int                        res;
	char                      *eval_string;
	zval                       ret_zval;
	function_stack_entry      *fse;
	xdebug_var_export_options *options = (xdebug_var_export_options*) context->options;
	zval                      *symbol;
	XDEBUG_STR_SWITCH_DECL;

	if (!CMD_OPTION('n')) { /* name */
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	if (!data) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	if (CMD_OPTION('d')) { /* depth */
		depth = strtol(CMD_OPTION('d'), NULL, 10);
	}

	if (CMD_OPTION('c')) { /* context_id */
		context_nr = strtol(CMD_OPTION('c'), NULL, 10);
	}

	/* Set the symbol table corresponding with the requested stack depth */
	if (context_nr == 0) { /* locals */
		if ((fse = xdebug_get_stack_frame(depth TSRMLS_CC))) {
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || PHP_MAJOR_VERSION >= 6
			function_stack_entry *old_fse = xdebug_get_stack_frame(depth - 1 TSRMLS_CC);

			if (depth > 0) {
				XG(active_execute_data) = old_fse->execute_data;
			} else {
				XG(active_execute_data) = EG(current_execute_data);
			}
#else
			XG(active_execute_data) = fse->execute_data;
#endif
			XG(active_symbol_table) = fse->symbol_table;
			XG(active_op_array)     = fse->op_array;
			XG(This)                = fse->This;
			XG(active_fse)          = fse;
		} else {
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
		}
	} else { /* superglobals */
		XG(active_symbol_table) = &EG(symbol_table);
	}

	if (CMD_OPTION('p')) {
		options->runtime[0].page = strtol(CMD_OPTION('p'), NULL, 10);
	} else {
		options->runtime[0].page = 0;
	}

	new_value = (char*) xdebug_base64_decode((unsigned char*) data, strlen(data), &new_length);

	if (CMD_OPTION('t')) {
		symbol = get_symbol_contents_zval(CMD_OPTION('n'), strlen(CMD_OPTION('n')) + 1 TSRMLS_CC);

		/* Handle result */
		if (!symbol) {
			efree(new_value);
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_PROPERTY_NON_EXISTANT);
		} else {
			zval_dtor(symbol);
			Z_TYPE_P(symbol) = IS_STRING;
			Z_STRVAL_P(symbol) = new_value;
			Z_STRLEN_P(symbol) = new_length;
			xdebug_xml_add_attribute(*retval, "success", "1");

			XDEBUG_STR_SWITCH(CMD_OPTION('t')) {
				XDEBUG_STR_CASE("bool")
					convert_to_boolean(symbol);
				XDEBUG_STR_CASE_END

				XDEBUG_STR_CASE("int")
					convert_to_long(symbol);
				XDEBUG_STR_CASE_END

				XDEBUG_STR_CASE("float")
					convert_to_double(symbol);
				XDEBUG_STR_CASE_END

				XDEBUG_STR_CASE("string")
					/* do nothing */
				XDEBUG_STR_CASE_END

				XDEBUG_STR_CASE_DEFAULT
					xdebug_xml_add_attribute(*retval, "success", "0");
				XDEBUG_STR_CASE_DEFAULT_END
			}
		}
	} else {
		/* Do the eval */
		eval_string = xdebug_sprintf("%s = %s", CMD_OPTION('n'), new_value);
		res = xdebug_do_eval(eval_string, &ret_zval TSRMLS_CC);

		/* Free data */
		xdfree(eval_string);
		efree(new_value);

		/* Handle result */
		if (res == FAILURE) {
			/* don't send an error, send success = zero */
			xdebug_xml_add_attribute(*retval, "success", "0");
		} else {
			zval_dtor(&ret_zval);
			xdebug_xml_add_attribute(*retval, "success", "1");
		}
	}
}

static int add_variable_contents_node(xdebug_xml_node *node, char *name, int name_length, int var_only, int non_null, int no_eval, xdebug_var_export_options *options TSRMLS_DC)
{
	int contents_found;

	contents_found = get_symbol_contents(name, name_length, node, options TSRMLS_CC);
	if (contents_found) {
		return SUCCESS;
	}
	return FAILURE;
}

DBGP_FUNC(property_value)
{
	int                        depth = 0;
	function_stack_entry      *fse;
	int                        old_max_data;
	xdebug_var_export_options *options = (xdebug_var_export_options*) context->options;

	if (!CMD_OPTION('n')) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	if (CMD_OPTION('d')) {
		depth = strtol(CMD_OPTION('d'), NULL, 10);
	}

	/* Set the symbol table corresponding with the requested stack depth */
	if ((fse = xdebug_get_stack_frame(depth TSRMLS_CC))) {
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || PHP_MAJOR_VERSION >= 6
		function_stack_entry *old_fse = xdebug_get_stack_frame(depth - 1 TSRMLS_CC);

		if (depth > 0) {
			XG(active_execute_data) = old_fse->execute_data;
		} else {
			XG(active_execute_data) = EG(current_execute_data);
		}
#else
		XG(active_execute_data) = fse->execute_data;
#endif
		XG(active_symbol_table) = fse->symbol_table;
		XG(active_op_array)     = fse->op_array;
		XG(This)                = fse->This;
		XG(active_fse)          = fse;
	} else {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
	}

	if (CMD_OPTION('p')) {
		options->runtime[0].page = strtol(CMD_OPTION('p'), NULL, 10);
	} else {
		options->runtime[0].page = 0;
	}

	/* Override max data size if necessary */
	old_max_data = options->max_data;
	if (CMD_OPTION('m')) {
		options->max_data = strtol(CMD_OPTION('m'), NULL, 10);
	}
	if (add_variable_contents_node(*retval, CMD_OPTION('n'), strlen(CMD_OPTION('n')) + 1, 1, 0, 0, options TSRMLS_CC) == FAILURE) {
		options->max_data = old_max_data;
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_PROPERTY_NON_EXISTANT);
	}
}

static void attach_used_var_with_contents(void *xml, xdebug_hash_element* he, void *options)
{
	char               *name = (char*) he->ptr;
	xdebug_xml_node    *node = (xdebug_xml_node *) xml;
	xdebug_xml_node    *contents;
	TSRMLS_FETCH();

	contents = get_symbol(name, strlen(name), options TSRMLS_CC);
	if (contents) {
		xdebug_xml_add_child(node, contents);
	} else {
		xdebug_attach_uninitialized_var(node, name);
	}
}

static int xdebug_add_filtered_symboltable_var(zval *symbol XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	xdebug_hash *tmp_hash;

	tmp_hash = va_arg(args, xdebug_hash *);

	if (strcmp("argc", hash_key->arKey) == 0) { return 0; }
	if (strcmp("argv", hash_key->arKey) == 0) { return 0; }
	if (hash_key->arKey[0] == '_') {
		if (strcmp("_COOKIE", hash_key->arKey) == 0) { return 0; }
		if (strcmp("_ENV", hash_key->arKey) == 0) { return 0; }
		if (strcmp("_FILES", hash_key->arKey) == 0) { return 0; }
		if (strcmp("_GET", hash_key->arKey) == 0) { return 0; }
		if (strcmp("_POST", hash_key->arKey) == 0) { return 0; }
		if (strcmp("_REQUEST", hash_key->arKey) == 0) { return 0; }
		if (strcmp("_SERVER", hash_key->arKey) == 0) { return 0; }
		if (strcmp("_SESSION", hash_key->arKey) == 0) { return 0; }
	}
	if (hash_key->arKey[0] == 'H') {
		if (strcmp("HTTP_COOKIE_VARS", hash_key->arKey) == 0) { return 0; }
		if (strcmp("HTTP_ENV_VARS", hash_key->arKey) == 0) { return 0; }
		if (strcmp("HTTP_GET_VARS", hash_key->arKey) == 0) { return 0; }
		if (strcmp("HTTP_POST_VARS", hash_key->arKey) == 0) { return 0; }
		if (strcmp("HTTP_POST_FILES", hash_key->arKey) == 0) { return 0; }
		if (strcmp("HTTP_RAW_POST_DATA", hash_key->arKey) == 0) { return 0; }
		if (strcmp("HTTP_SERVER_VARS", hash_key->arKey) == 0) { return 0; }
		if (strcmp("HTTP_SESSION_VARS", hash_key->arKey) == 0) { return 0; }
	}
	if (strcmp("GLOBALS", hash_key->arKey) == 0) { return 0; }

	xdebug_hash_add(tmp_hash, hash_key->arKey, strlen(hash_key->arKey), hash_key->arKey);	

	return 0;
}

static int attach_context_vars(xdebug_xml_node *node, xdebug_var_export_options *options, long context_id, long depth, void (*func)(void *, xdebug_hash_element*, void*) TSRMLS_DC)
{
	function_stack_entry *fse;
	char                 *var_name;

	/* right now, we only have zero or one, one being globals, which is
	 * always the head of the stack */
	if (context_id == 1) {
		/* add super globals */
		XG(active_symbol_table) = &EG(symbol_table);
		XG(active_execute_data) = NULL;
		add_variable_node(node, "_COOKIE", sizeof("_COOKIE"), 1, 1, 0, options TSRMLS_CC);
		add_variable_node(node, "_ENV", sizeof("_ENV"), 1, 1, 0, options TSRMLS_CC);
		add_variable_node(node, "_FILES", sizeof("_FILES"), 1, 1, 0, options TSRMLS_CC);
		add_variable_node(node, "_GET", sizeof("_GET"), 1, 1, 0, options TSRMLS_CC);
		add_variable_node(node, "_POST", sizeof("_POST"), 1, 1, 0, options TSRMLS_CC);
		add_variable_node(node, "_REQUEST", sizeof("_REQUEST"), 1, 1, 0, options TSRMLS_CC);
		add_variable_node(node, "_SERVER", sizeof("_SERVER"), 1, 1, 0, options TSRMLS_CC);
		add_variable_node(node, "_SESSION", sizeof("_SESSION"), 1, 1, 0, options TSRMLS_CC);
		add_variable_node(node, "GLOBALS", sizeof("GLOBALS"), 1, 1, 0, options TSRMLS_CC);
		XG(active_symbol_table) = NULL;
		return 0;
	}

	/* Here the context_id is 0 */
	if ((fse = xdebug_get_stack_frame(depth TSRMLS_CC))) {
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3) || PHP_MAJOR_VERSION >= 6
		function_stack_entry *old_fse = xdebug_get_stack_frame(depth - 1 TSRMLS_CC);

		if (depth > 0) {
			XG(active_execute_data) = old_fse->execute_data;
		} else {
			XG(active_execute_data) = EG(current_execute_data);
		}
#else
		XG(active_execute_data) = fse->execute_data;
#endif
		XG(active_symbol_table) = fse->symbol_table;
		XG(active_op_array)     = fse->op_array;
		XG(This)                = fse->This;

		/* Only show vars when they are scanned */
		if (fse->used_vars) {
			xdebug_hash *tmp_hash;

			/* Get a hash from all the used vars (which can have duplicates) */
			tmp_hash = xdebug_used_var_hash_from_llist(fse->used_vars);

			/* Check for dynamically defined variables, but make sure we don't already
			 * have them. Also blacklist superglobals and argv/argc */
			if (XG(active_symbol_table)) {
				zend_hash_apply_with_arguments(XG(active_symbol_table) XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) xdebug_add_filtered_symboltable_var, 1, tmp_hash);
			}

			/* Add all the found variables to the node */
			xdebug_hash_apply_with_argument(tmp_hash, (void *) node, func, (void *) options);

			/* Zend engine 2 does not give us $this, eval so we can get it */
			if (!xdebug_hash_find(tmp_hash, "this", 4, (void *) &var_name)) {
				add_variable_node(node, "this", sizeof("this"), 1, 1, 0, options TSRMLS_CC);
			}

			xdebug_hash_destroy(tmp_hash);
		}

		/* Check for static variables and constants, but only if it's a static
		 * method call as we attach constants and static properties to "this"
		 * too normally. */
		if (fse->function.type == XFUNC_STATIC_MEMBER) {
			zend_class_entry *ce = zend_fetch_class(fse->function.class, strlen(fse->function.class), ZEND_FETCH_CLASS_SELF TSRMLS_CC);

			xdebug_attach_static_vars(node, options, ce TSRMLS_CC);
		}

		XG(active_symbol_table) = NULL;
		XG(active_execute_data) = NULL;
		XG(active_op_array)     = NULL;
		XG(This)                = NULL;
		return 0;
	}
	
	return 1;
}


DBGP_FUNC(stack_depth)
{
	xdebug_xml_add_attribute_ex(*retval, "depth", xdebug_sprintf("%lu", XG(level)), 0, 1);
}

DBGP_FUNC(stack_get)
{
	xdebug_xml_node      *stackframe;
	xdebug_llist_element *le;
	int                   counter = 0, depth;

	if (CMD_OPTION('d')) {
		depth = strtol(CMD_OPTION('d'), NULL, 10);
		if (depth >= 0 && depth < XG(level)) {
			stackframe = return_stackframe(depth TSRMLS_CC);
			xdebug_xml_add_child(*retval, stackframe);
		} else {
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
		}
	} else {
		counter = 0;
		for (le = XDEBUG_LLIST_TAIL(XG(stack)); le != NULL; le = XDEBUG_LLIST_PREV(le)) {
			stackframe = return_stackframe(counter TSRMLS_CC);
			xdebug_xml_add_child(*retval, stackframe);
			counter++;
		}
	}
}

DBGP_FUNC(status)
{
	xdebug_xml_add_attribute(*retval, "status", xdebug_dbgp_status_strings[XG(status)]);
	xdebug_xml_add_attribute(*retval, "reason", xdebug_dbgp_reason_strings[XG(reason)]);
}


DBGP_FUNC(context_names)
{
	xdebug_xml_node *child;

	child = xdebug_xml_node_init("context");
	xdebug_xml_add_attribute(child, "name", "Locals");
	xdebug_xml_add_attribute(child, "id", "0");
	xdebug_xml_add_child(*retval, child);
	child = xdebug_xml_node_init("context");
	xdebug_xml_add_attribute(child, "name", "Superglobals");
	xdebug_xml_add_attribute(child, "id", "1");
	xdebug_xml_add_child(*retval, child);
}

DBGP_FUNC(context_get)
{
	int                        res;
	int                        context_id = 0;
	int                        depth = 0;
	xdebug_var_export_options *options = (xdebug_var_export_options*) context->options;
	
	if (CMD_OPTION('c')) {
		context_id = atol(CMD_OPTION('c'));
	}
	if (CMD_OPTION('d')) {
		depth = atol(CMD_OPTION('d'));
	}
	/* Always reset to page = 0, as it might have been modified by property_get or property_value */
	options->runtime[0].page = 0;
	
	res = attach_context_vars(*retval, options, context_id, depth, attach_used_var_with_contents TSRMLS_CC);
	switch (res) {
		case 1:
			RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
			break;
	}

	xdebug_xml_add_attribute_ex(*retval, "context", xdebug_sprintf("%d", context_id), 0, 1);
}

DBGP_FUNC(xcmd_profiler_name_get)
{
	if (XG(profiler_enabled) && XG(profile_filename)) {
		xdebug_xml_add_text(*retval, xdstrdup(XG(profile_filename)));
	} else {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_PROFILING_NOT_STARTED);
	}
}

DBGP_FUNC(xcmd_get_executable_lines)
{
	function_stack_entry *fse;
	int                   i, depth;
	xdebug_xml_node      *lines, *line;

	if (!CMD_OPTION('d')) {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_INVALID_ARGS);
	}

	depth = strtol(CMD_OPTION('d'), NULL, 10);
	if (depth >= 0 && depth < XG(level)) {
		fse = xdebug_get_stack_frame(depth TSRMLS_CC);
	} else {
		RETURN_RESULT(XG(status), XG(reason), XDEBUG_ERROR_STACK_DEPTH_INVALID);
	}

	lines = xdebug_xml_node_init("xdebug:lines");
	for (i = 0; i < fse->op_array->last; i++ ) {
		if (fse->op_array->opcodes[i].opcode == ZEND_EXT_STMT ) {
			line = xdebug_xml_node_init("xdebug:line");
			xdebug_xml_add_attribute_ex(line, "lineno", xdebug_sprintf("%lu", fse->op_array->opcodes[i].lineno), 0, 1);
			xdebug_xml_add_child(lines, line);
		}
	}
	xdebug_xml_add_child(*retval, lines);
}


/*****************************************************************************
** Parsing functions
*/

/* {{{ Constants for state machine */
#define STATE_NORMAL                   0
#define STATE_QUOTED                   1
#define STATE_OPT_FOLLOWS              2
#define STATE_SEP_FOLLOWS              3
#define STATE_VALUE_FOLLOWS_FIRST_CHAR 4
#define STATE_VALUE_FOLLOWS            5
#define STATE_SKIP_CHAR                6
/* }}} */

static void xdebug_dbgp_arg_dtor(xdebug_dbgp_arg *arg)
{
	int i;

	for (i = 0; i < 27; i++) {
		if (arg->value[i]) {
			xdfree(arg->value[i]);
		}
	}
	xdfree(arg);
}

static int xdebug_dbgp_parse_cmd(char *line, char **cmd, xdebug_dbgp_arg **ret_args)
{
	xdebug_dbgp_arg *args = NULL;
	char *ptr;
	int   state;
	int   charescaped = 0;
	char  opt = ' ', *value_begin = NULL;

	args = xdmalloc(sizeof (xdebug_dbgp_arg));
	memset(args->value, 0, sizeof(args->value));
	*cmd = NULL;

	/* Find the end of the command, this is always on the first space */
	ptr = strchr(line, ' ');
	if (!ptr) {
		/* No space found. If the line is not empty, return the line
		 * and assume it only consists of the command name. If the line
		 * is 0 chars long, we return a failure. */
		if (strlen(line)) {
			*cmd = strdup(line);
			*ret_args = args;
			return XDEBUG_ERROR_OK;
		} else {
			goto parse_error;
		}
	} else {
		/* A space was found, so we copy everything before it
		 * into the cmd parameter. */
		*cmd = xdcalloc(1, ptr - line + 1);
		memcpy(*cmd, line, ptr - line);
	}
	/* Now we loop until we find the end of the string, which is the \0
	 * character */
	state = STATE_NORMAL;
	do {
		ptr++;
		switch (state) {
			case STATE_NORMAL:
				if (*ptr != '-') {
					goto parse_error;
				} else {
					state = STATE_OPT_FOLLOWS;
				}
				break;
			case STATE_OPT_FOLLOWS:
				opt = *ptr;
				state = STATE_SEP_FOLLOWS;
				break;
			case STATE_SEP_FOLLOWS:
				if (*ptr != ' ') {
					goto parse_error;
				} else {
					state = STATE_VALUE_FOLLOWS_FIRST_CHAR;
					value_begin = ptr + 1;
				}
				break;
			case STATE_VALUE_FOLLOWS_FIRST_CHAR:
				if (*ptr == '"' && opt != '-') {
					value_begin = ptr + 1;
					state = STATE_QUOTED;
				} else {
					state = STATE_VALUE_FOLLOWS;
				}
				break;
			case STATE_VALUE_FOLLOWS:
				if ((*ptr == ' ' && opt != '-') || *ptr == '\0') {
					int index = opt - 'a';

					if (opt == '-') {
						index = 26;
					}

					if (!args->value[index]) {
						args->value[index] = xdcalloc(1, ptr - value_begin + 1);
						memcpy(args->value[index], value_begin, ptr - value_begin);
						state = STATE_NORMAL;
					} else {
						goto duplicate_opts;
					}
				}
				break;
			case STATE_QUOTED:
				/* if the quote is escaped, remain in STATE_QUOTED.  This
				   will also handle other escaped chars, or an instance of
				   an escaped slash followed by a quote: \\"
				*/
				if (*ptr == '\\') {
					charescaped = !charescaped;
				} else
				if (*ptr == '"') {
					int index = opt - 'a';

					if (charescaped) {
						charescaped = 0;
						break;
					}
					if (opt == '-') {
						index = 26;
					}

					if (!args->value[index]) {
						int len = ptr - value_begin;
						args->value[index] = xdcalloc(1, len + 1);
						memcpy(args->value[index], value_begin, len);
						php_stripcslashes(args->value[index], &len);
						state = STATE_SKIP_CHAR;
					} else {
						goto duplicate_opts;
					}
				}
				break;
			case STATE_SKIP_CHAR:
				state = STATE_NORMAL;
				break;
				
		}
	} while (*ptr);
	*ret_args = args;
	return XDEBUG_ERROR_OK;

parse_error:
	*ret_args = args;
	return XDEBUG_ERROR_PARSE;

duplicate_opts:
	*ret_args = args;
	return XDEBUG_ERROR_DUP_ARG;
}

static int xdebug_dbgp_parse_option(xdebug_con *context, char* line, int flags, xdebug_xml_node *retval TSRMLS_DC)
{
	char *cmd = NULL;
	int res, ret = 0;
	xdebug_dbgp_arg *args;
	xdebug_dbgp_cmd *command;
	xdebug_xml_node *error;

	if (XG(remote_log_file)) {
		fprintf(XG(remote_log_file), "<- %s\n", line);
		fflush(XG(remote_log_file));
	}
	res = xdebug_dbgp_parse_cmd(line, (char**) &cmd, (xdebug_dbgp_arg**) &args);

	/* Add command name to return packet */
	if (cmd) {
		/* if no cmd res will be XDEBUG_ERROR_PARSE */
		xdebug_xml_add_attribute_ex(retval, "command", xdstrdup(cmd), 0, 1);
	}

	/* Handle missing transaction ID, and if it exist add it to the result */
	if (!CMD_OPTION('i')) {
		/* we need the transaction_id even for errors in parse_cmd, but if
		   we error out here, just force the error to happen below */
		res = XDEBUG_ERROR_INVALID_ARGS;
	} else {
		xdebug_xml_add_attribute_ex(retval, "transaction_id", xdstrdup(CMD_OPTION('i')), 0, 1);
	}

	/* Handle parse errors */
	/* FIXME: use RETURN_RESULT here too */
	if (res != XDEBUG_ERROR_OK) {
		error = xdebug_xml_node_init("error");
		xdebug_xml_add_attribute_ex(error, "code", xdebug_sprintf("%lu", res), 0, 1);
		xdebug_xml_add_child(retval, error);
		ADD_REASON_MESSAGE(res);
	} else {

		/* Execute commands and stuff */
		command = lookup_cmd(cmd);

		if (command) {
			if (command->cont) {
				XG(status) = DBGP_STATUS_RUNNING;
				XG(reason) = DBGP_REASON_OK;
			}
			XG(lastcmd) = command->name;
			if (XG(lasttransid)) {
				xdfree(XG(lasttransid));
			}
			XG(lasttransid) = xdstrdup(CMD_OPTION('i'));
			if (XG(status) != DBGP_STATUS_STOPPING || (XG(status) == DBGP_STATUS_STOPPING && command->flags & XDEBUG_DBGP_POST_MORTEM)) {
				command->handler((xdebug_xml_node**) &retval, context, args TSRMLS_CC);
				ret = command->cont;
			} else {
				error = xdebug_xml_node_init("error");
				xdebug_xml_add_attribute_ex(error, "code", xdebug_sprintf("%lu", XDEBUG_ERROR_COMMAND_UNAVAILABLE), 0, 1);
				ADD_REASON_MESSAGE(XDEBUG_ERROR_COMMAND_UNAVAILABLE);
				xdebug_xml_add_child(retval, error);

				ret = -1;
			}
		} else {
			error = xdebug_xml_node_init("error");
			xdebug_xml_add_attribute_ex(error, "code", xdebug_sprintf("%lu", XDEBUG_ERROR_UNIMPLEMENTED), 0, 1);
			ADD_REASON_MESSAGE(XDEBUG_ERROR_UNIMPLEMENTED);
			xdebug_xml_add_child(retval, error);

			ret = -1;
		}
	}

	xdfree(cmd);
	xdebug_dbgp_arg_dtor(args);
	return ret;
}

/*****************************************************************************
** Handlers for debug functions
*/

char *xdebug_dbgp_get_revision(void)
{
	return "$Revision: 1.145 $";
}

static int xdebug_dbgp_cmdloop(xdebug_con *context, int bail TSRMLS_DC)
{
	char *option;
	int   ret;
	xdebug_xml_node *response;
	
	do {
		option = xdebug_fd_read_line_delim(context->socket, context->buffer, FD_RL_SOCKET, '\0', NULL);
		if (!option) {
			return 0;
		}

		response = xdebug_xml_node_init("response");
		xdebug_xml_add_attribute(response, "xmlns", "urn:debugger_protocol_v1");
		xdebug_xml_add_attribute(response, "xmlns:xdebug", "http://xdebug.org/dbgp/xdebug");
		ret = xdebug_dbgp_parse_option(context, option, 0, response TSRMLS_CC);
		if (ret != 1) {
			send_message(context, response TSRMLS_CC);
		}
		xdebug_xml_node_dtor(response);

		free(option);
	} while (0 == ret);
	
	if (bail && XG(status) == DBGP_STATUS_STOPPED) {
		zend_bailout();
	}
	return ret;

}

int xdebug_dbgp_init(xdebug_con *context, int mode)
{
	xdebug_var_export_options *options;
	xdebug_xml_node *response, *child;
	int i;
	TSRMLS_FETCH();

	/* initialize our status information */
	if (mode == XDEBUG_REQ) {
		XG(status) = DBGP_STATUS_STARTING;
		XG(reason) = DBGP_REASON_OK;
	} else if (mode == XDEBUG_JIT) {
		XG(status) = DBGP_STATUS_BREAK;
		XG(reason) = DBGP_REASON_ERROR;
	}
	XG(lastcmd) = NULL;
	XG(lasttransid) = NULL;

	response = xdebug_xml_node_init("init");
	xdebug_xml_add_attribute(response, "xmlns", "urn:debugger_protocol_v1");
	xdebug_xml_add_attribute(response, "xmlns:xdebug", "http://xdebug.org/dbgp/xdebug");

/* {{{ XML Init Stuff*/
	child = xdebug_xml_node_init("engine");
	xdebug_xml_add_attribute(child, "version", XDEBUG_VERSION);
	xdebug_xml_add_text(child, xdstrdup(XDEBUG_NAME));
	xdebug_xml_add_child(response, child);

	child = xdebug_xml_node_init("author");
	xdebug_xml_add_text(child, xdstrdup(XDEBUG_AUTHOR));
	xdebug_xml_add_child(response, child);

	child = xdebug_xml_node_init("url");
	xdebug_xml_add_text(child, xdstrdup(XDEBUG_URL));
	xdebug_xml_add_child(response, child);

	child = xdebug_xml_node_init("copyright");
	xdebug_xml_add_text(child, xdstrdup(XDEBUG_COPYRIGHT));
	xdebug_xml_add_child(response, child);

	if (strcmp(context->program_name, "-") == 0) {
		xdebug_xml_add_attribute_ex(response, "fileuri", xdstrdup("dbgp://stdin"), 0, 1);
	} else {
		xdebug_xml_add_attribute_ex(response, "fileuri", xdebug_path_to_url(context->program_name TSRMLS_CC), 0, 1);
	}
	xdebug_xml_add_attribute_ex(response, "language", "PHP", 0, 0);
	xdebug_xml_add_attribute_ex(response, "protocol_version", DBGP_VERSION, 0, 0);
	xdebug_xml_add_attribute_ex(response, "appid", xdebug_sprintf("%d", getpid()), 0, 1);

	if (getenv("DBGP_COOKIE")) {
		xdebug_xml_add_attribute_ex(response, "session", xdstrdup(getenv("DBGP_COOKIE")), 0, 1);
	}

	if (XG(ide_key) && *XG(ide_key)) {
		xdebug_xml_add_attribute_ex(response, "idekey", xdstrdup(XG(ide_key)), 0, 1);
	}

	context->buffer = xdmalloc(sizeof(fd_buf));
	context->buffer->buffer = NULL;
	context->buffer->buffer_size = 0;

	send_message(context, response TSRMLS_CC);
	xdebug_xml_node_dtor(response);
/* }}} */

	context->options = xdmalloc(sizeof(xdebug_var_export_options));
	options = (xdebug_var_export_options*) context->options;
	options->max_children = 32;
	options->max_data     = 1024;
	options->max_depth    = 1;
	options->show_hidden  = 0;
	options->runtime = (xdebug_var_runtime_page*) xdmalloc((options->max_depth + 1) * sizeof(xdebug_var_runtime_page));
	for (i = 0; i < options->max_depth; i++) {
		options->runtime[i].page = 0;
		options->runtime[i].current_element_nr = 0;
	}

	context->breakpoint_list = xdebug_hash_alloc(64, (xdebug_hash_dtor) xdebug_hash_admin_dtor);
	context->function_breakpoints = xdebug_hash_alloc(64, (xdebug_hash_dtor) xdebug_hash_brk_dtor);
	context->exception_breakpoints = xdebug_hash_alloc(64, (xdebug_hash_dtor) xdebug_hash_brk_dtor);
	context->line_breakpoints = xdebug_llist_alloc((xdebug_llist_dtor) xdebug_llist_brk_dtor);
	context->eval_id_lookup = xdebug_hash_alloc(64, (xdebug_hash_dtor) xdebug_hash_eval_info_dtor);
	context->eval_id_sequence = 0;

	xdebug_dbgp_cmdloop(context, 1 TSRMLS_CC);

	return 1;
}

int xdebug_dbgp_deinit(xdebug_con *context)
{
	xdebug_xml_node           *response;
	xdebug_var_export_options *options;
	TSRMLS_FETCH();

	if (XG(remote_enabled)) {
		XG(status) = DBGP_STATUS_STOPPING;
		XG(reason) = DBGP_REASON_OK;
		response = xdebug_xml_node_init("response");
		xdebug_xml_add_attribute(response, "xmlns", "urn:debugger_protocol_v1");
		xdebug_xml_add_attribute(response, "xmlns:xdebug", "http://xdebug.org/dbgp/xdebug");
		/* lastcmd and lasttransid are not always set (for example when the
		 * connection is severed before the first command is send) */
		if (XG(lastcmd) && XG(lasttransid)) {
			xdebug_xml_add_attribute_ex(response, "command", XG(lastcmd), 0, 0);
			xdebug_xml_add_attribute_ex(response, "transaction_id", XG(lasttransid), 0, 0);
		}
		xdebug_xml_add_attribute_ex(response, "status", xdebug_dbgp_status_strings[XG(status)], 0, 0);
		xdebug_xml_add_attribute_ex(response, "reason", xdebug_dbgp_reason_strings[XG(reason)], 0, 0);
	
		send_message(context, response TSRMLS_CC);
		xdebug_xml_node_dtor(response);
	
		xdebug_dbgp_cmdloop(context, 0 TSRMLS_CC);
	}

	if (XG(remote_enabled)) {
		options = (xdebug_var_export_options*) context->options;
		xdfree(options->runtime);
		xdfree(context->options);
		xdebug_hash_destroy(context->function_breakpoints);
		xdebug_hash_destroy(context->exception_breakpoints);
		xdebug_hash_destroy(context->eval_id_lookup);
		xdebug_llist_destroy(context->line_breakpoints, NULL);
		xdebug_hash_destroy(context->breakpoint_list);
		xdfree(context->buffer);
	}

	xdebug_close_log(TSRMLS_C);
	XG(remote_enabled) = 0;
	return 1;
}

int xdebug_dbgp_error(xdebug_con *context, int type, char *exception_type, char *message, const char *location, const uint line, xdebug_llist *stack)
{
	char               *errortype;
	xdebug_xml_node     *response, *error;
	TSRMLS_FETCH();

	if (exception_type) {
		errortype = exception_type;
	} else {
		errortype = xdebug_error_type(type);
	}

	if (exception_type) {
		XG(status) = DBGP_STATUS_BREAK;
		XG(reason) = DBGP_REASON_EXCEPTION;
	} else {
		switch (type) {
			case E_CORE_ERROR:
			/* no break - intentionally */
			case E_ERROR:
			/*case E_PARSE: the parser would return 1 (failure), we can bail out nicely */
			case E_COMPILE_ERROR:
			case E_USER_ERROR:
				XG(status) = DBGP_STATUS_STOPPING;
				XG(reason) = DBGP_REASON_ABORTED;
				break;
			default:
				XG(status) = DBGP_STATUS_BREAK;
				XG(reason) = DBGP_REASON_ERROR;
		}
	}
/*
	runtime_allowed = (
		(type != E_ERROR) && 
		(type != E_CORE_ERROR) &&
		(type != E_COMPILE_ERROR) &&
		(type != E_USER_ERROR)
	) ? XDEBUG_BREAKPOINT | XDEBUG_RUNTIME : 0;
*/

	response = xdebug_xml_node_init("response");
	xdebug_xml_add_attribute(response, "xmlns", "urn:debugger_protocol_v1");
	xdebug_xml_add_attribute(response, "xmlns:xdebug", "http://xdebug.org/dbgp/xdebug");
	/* lastcmd and lasttransid are not always set (for example when the
	 * connection is severed before the first command is send) */
	if (XG(lastcmd) && XG(lasttransid)) {
		xdebug_xml_add_attribute_ex(response, "command", XG(lastcmd), 0, 0);
		xdebug_xml_add_attribute_ex(response, "transaction_id", XG(lasttransid), 0, 0);
	}
	xdebug_xml_add_attribute(response, "status", xdebug_dbgp_status_strings[XG(status)]);
	xdebug_xml_add_attribute(response, "reason", xdebug_dbgp_reason_strings[XG(reason)]);

	error = xdebug_xml_node_init("error");
	xdebug_xml_add_attribute_ex(error, "code", xdebug_sprintf("%lu", type), 0, 1);
	xdebug_xml_add_attribute_ex(error, "exception", xdstrdup(errortype), 0, 1);
	xdebug_xml_add_text(error, xdstrdup(message));
	xdebug_xml_add_child(response, error);

	send_message(context, response TSRMLS_CC);
	xdebug_xml_node_dtor(response);
	if (!exception_type) {
		xdfree(errortype);
	}

	xdebug_dbgp_cmdloop(context, 1 TSRMLS_CC);

	return 1;
}

int xdebug_dbgp_breakpoint(xdebug_con *context, xdebug_llist *stack, char *file, long lineno, int type, char *exception, char *message)
{
	xdebug_xml_node *response, *error_container;
	TSRMLS_FETCH();

	XG(status) = DBGP_STATUS_BREAK;
	XG(reason) = DBGP_REASON_OK;

	response = xdebug_xml_node_init("response");
	xdebug_xml_add_attribute(response, "xmlns", "urn:debugger_protocol_v1");
	xdebug_xml_add_attribute(response, "xmlns:xdebug", "http://xdebug.org/dbgp/xdebug");
	/* lastcmd and lasttransid are not always set (for example when the
	 * connection is severed before the first command is send) */
	if (XG(lastcmd) && XG(lasttransid)) {
		xdebug_xml_add_attribute_ex(response, "command", XG(lastcmd), 0, 0);
		xdebug_xml_add_attribute_ex(response, "transaction_id", XG(lasttransid), 0, 0);
	}
	xdebug_xml_add_attribute(response, "status", xdebug_dbgp_status_strings[XG(status)]);
	xdebug_xml_add_attribute(response, "reason", xdebug_dbgp_reason_strings[XG(reason)]);

	error_container = xdebug_xml_node_init("xdebug:message");
	if (file) {
		char *tmp_filename = file;
		int tmp_lineno = lineno;
		if (check_evaled_code(NULL, &tmp_filename, &tmp_lineno, 0 TSRMLS_CC)) {
			xdebug_xml_add_attribute_ex(error_container, "filename", xdstrdup(tmp_filename), 0, 1);
		} else {
			xdebug_xml_add_attribute_ex(error_container, "filename", xdebug_path_to_url(file TSRMLS_CC), 0, 1);
		}
	}
	if (lineno) {
		xdebug_xml_add_attribute_ex(error_container, "lineno", xdebug_sprintf("%lu", lineno), 0, 1);
	}
	if (exception) {
		xdebug_xml_add_attribute_ex(error_container, "exception", xdstrdup(exception), 0, 1);
	}
	if (message) {
		xdebug_xml_add_text(error_container, xdstrdup(message));
	}
	xdebug_xml_add_child(response, error_container);

	send_message(context, response TSRMLS_CC);
	xdebug_xml_node_dtor(response);

	XG(lastcmd) = NULL;
	if (XG(lasttransid)) {
		xdfree(XG(lasttransid));
		XG(lasttransid) = NULL;
	}

	xdebug_dbgp_cmdloop(context, 1 TSRMLS_CC);

	return 1;
}

int xdebug_dbgp_stream_output(const char *string, unsigned int length TSRMLS_DC)
{
	if ((XG(stdout_mode) == 1 || XG(stdout_mode) == 2) && length) {
		xdebug_send_stream("stdout", string, length TSRMLS_CC);
	}

	if (XG(stdout_mode) == 0 || XG(stdout_mode) == 1) {
		return 0;
	}
	return -1;
}

static char *create_eval_key_file(char *filename, int lineno)
{
	return xdebug_sprintf("%s(%d) : eval()'d code", filename, lineno);
}

static char *create_eval_key_id(int id)
{
	return xdebug_sprintf("%04x", id);
}

int xdebug_dbgp_register_eval_id(xdebug_con *context, function_stack_entry *fse)
{
	char             *key;
	xdebug_eval_info *ei;

	context->eval_id_sequence++;

	ei = xdcalloc(sizeof(xdebug_eval_info), 1);
	ei->id = context->eval_id_sequence;
	ei->contents = xdstrndup(fse->include_filename, strlen(fse->include_filename));
	ei->refcount = 2;

	key = create_eval_key_file(fse->filename, fse->lineno);
	xdebug_hash_add(context->eval_id_lookup, key, strlen(key), (void*) ei);

	key = create_eval_key_id(ei->id);
	xdebug_hash_add(context->eval_id_lookup, key, strlen(key), (void*) ei);

	return ei->id;
}
