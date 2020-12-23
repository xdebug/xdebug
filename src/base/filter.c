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
#include "php_xdebug.h"

#include "filter.h"

#include "lib/lib.h"
#include "lib/log.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

int xdebug_is_stack_frame_filtered(int filter_type, function_stack_entry *fse)
{
	switch (filter_type) {
		case XDEBUG_FILTER_CODE_COVERAGE:
			return fse->filtered_code_coverage;

		case XDEBUG_FILTER_STACK:
			return fse->filtered_stack;

		case XDEBUG_FILTER_TRACING:
			return fse->filtered_tracing;
	}

	return 0;
}

int xdebug_is_top_stack_frame_filtered(int filter_type)
{
	function_stack_entry *fse;
	fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));
	return xdebug_is_stack_frame_filtered(filter_type, fse);
}

void xdebug_filter_register_constants(INIT_FUNC_ARGS)
{
	REGISTER_LONG_CONSTANT("XDEBUG_FILTER_CODE_COVERAGE", XDEBUG_FILTER_CODE_COVERAGE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_FILTER_STACK", XDEBUG_FILTER_STACK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_FILTER_TRACING", XDEBUG_FILTER_TRACING, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("XDEBUG_FILTER_NONE", XDEBUG_FILTER_NONE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_PATH_INCLUDE", XDEBUG_PATH_INCLUDE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_PATH_EXCLUDE", XDEBUG_PATH_EXCLUDE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_NAMESPACE_INCLUDE", XDEBUG_NAMESPACE_INCLUDE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XDEBUG_NAMESPACE_EXCLUDE", XDEBUG_NAMESPACE_EXCLUDE, CONST_CS | CONST_PERSISTENT);
}

static int xdebug_filter_match_path_include(function_stack_entry *fse, unsigned char *filtered_flag, char *filter)
{
	if (strncasecmp(filter, ZSTR_VAL(fse->filename), strlen(filter)) == 0) {
		*filtered_flag = 0;
		return 1;
	}
	return 0;
}

static int xdebug_filter_match_path_exclude(function_stack_entry *fse, unsigned char *filtered_flag, char *filter)
{
	if (strncasecmp(filter, ZSTR_VAL(fse->filename), strlen(filter)) == 0) {
		*filtered_flag = 1;
		return 1;
	}
	return 0;
}

static int xdebug_filter_match_namespace_include(function_stack_entry *fse, unsigned char *filtered_flag, char *filter)
{
	if (!fse->function.class_name && filter[0] == '\0') {
		*filtered_flag = 0;
		return 1;
	}
	if (fse->function.class_name && filter[0] != '\0' && strncasecmp(filter, ZSTR_VAL(fse->function.class_name), strlen(filter)) == 0) {
		*filtered_flag = 0;
		return 1;
	}
	return 0;
}

static int xdebug_filter_match_namespace_exclude(function_stack_entry *fse, unsigned char *filtered_flag, char *filter)
{
	if (!fse->function.class_name && filter[0] == '\0') {
		*filtered_flag = 1;
		return 1;
	}
	if (fse->function.class_name && filter[0] != '\0' && strncasecmp(filter, ZSTR_VAL(fse->function.class_name), strlen(filter)) == 0) {
		*filtered_flag = 1;
		return 1;
	}
	return 0;
}


void xdebug_filter_run_internal(function_stack_entry *fse, int group, unsigned char *filtered_flag, int type, xdebug_llist *filters)
{
	xdebug_llist_element *le;
	unsigned int          k;
	function_stack_entry  tmp_fse;
	int (*filter_to_run)(function_stack_entry *fse, unsigned char *filtered_flag, char *filter);

	le = XDEBUG_LLIST_HEAD(filters);

	switch (type) {
		case XDEBUG_PATH_INCLUDE:
			*filtered_flag = 1;
			if (group == XDEBUG_FILTER_CODE_COVERAGE && fse->function.type & XFUNC_INCLUDES) {
				tmp_fse.filename = fse->include_filename;
				fse = &tmp_fse;
			}

			filter_to_run = xdebug_filter_match_path_include;
			break;

		case XDEBUG_PATH_EXCLUDE:
			*filtered_flag = 0;
			if (group == XDEBUG_FILTER_CODE_COVERAGE && fse->function.type & XFUNC_INCLUDES) {
				tmp_fse.filename = fse->include_filename;
				fse = &tmp_fse;
			}

			filter_to_run = xdebug_filter_match_path_exclude;
			break;

		case XDEBUG_NAMESPACE_INCLUDE:
			*filtered_flag = 1;
			filter_to_run = xdebug_filter_match_namespace_include;
			break;

		case XDEBUG_NAMESPACE_EXCLUDE:
			*filtered_flag = 0;
			filter_to_run = xdebug_filter_match_namespace_exclude;
			break;

		default:
			/* Logically can't happen, but compilers can't detect that */
			return;
	}

	for (k = 0; k < filters->size; k++, le = XDEBUG_LLIST_NEXT(le)) {
		char *filter = XDEBUG_LLIST_VALP(le);

		/* If the filter matched once, we're done */
		if (filter_to_run(fse, filtered_flag, filter)) {
			break;
		}
	}
}

void xdebug_filter_run(function_stack_entry *fse)
{
	fse->filtered_stack   = 0;
	fse->filtered_tracing = 0;

	if (XG_BASE(filter_type_stack) != XDEBUG_FILTER_NONE) {
		xdebug_filter_run_internal(fse, XDEBUG_FILTER_STACK, &fse->filtered_stack, XG_BASE(filter_type_stack), XG_BASE(filters_stack));
	}
	if (XG_BASE(filter_type_tracing) != XDEBUG_FILTER_NONE) {
		xdebug_filter_run_internal(fse, XDEBUG_FILTER_TRACING, &fse->filtered_tracing, XG_BASE(filter_type_tracing), XG_BASE(filters_tracing));
	}
}

/* {{{ proto void xdebug_set_filter(int group, int type, array filters)
   This function configures filters for tracing and code coverage */
PHP_FUNCTION(xdebug_set_filter)
{
	zend_long      filter_group;
	zend_long      filter_type;
	xdebug_llist **filter_list;
	zval          *filters, *item;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lla", &filter_group, &filter_type, &filters) == FAILURE) {
		return;
	}

	switch (filter_group) {
		case XDEBUG_FILTER_CODE_COVERAGE:
			if (!XDEBUG_MODE_IS(XDEBUG_MODE_COVERAGE)) {
				xdebug_log_ex(XLOG_CHAN_BASE, XLOG_WARN, "COV-FILTER", "Can not set a filter for code coverage, because Xdebug mode does not include 'coverage'");
				return;
			}

			filter_list = &XG_BASE(filters_code_coverage);
			XG_BASE(filter_type_code_coverage) = XDEBUG_FILTER_NONE;
			if (filter_type == XDEBUG_NAMESPACE_INCLUDE || filter_type == XDEBUG_NAMESPACE_EXCLUDE) {
				php_error(E_WARNING, "The code coverage filter (XDEBUG_FILTER_CODE_COVERAGE) only supports the XDEBUG_PATH_INCLUDE, XDEBUG_PATH_EXCLUDE, and XDEBUG_FILTER_NONE filter types");
				return;
			}
			break;

		case XDEBUG_FILTER_STACK:
			if (!XDEBUG_MODE_IS(XDEBUG_MODE_DEVELOP)) {
				xdebug_log_ex(XLOG_CHAN_BASE, XLOG_WARN, "DEV-FILTER", "Can not set a stack filter, because Xdebug mode does not include 'develop'");
				return;
			}

			filter_list = &XG_BASE(filters_stack);
			XG_BASE(filter_type_stack) = XDEBUG_FILTER_NONE;
			break;

		case XDEBUG_FILTER_TRACING:
			if (!XDEBUG_MODE_IS(XDEBUG_MODE_TRACING)) {
				xdebug_log_ex(XLOG_CHAN_BASE, XLOG_WARN, "TRACE-FILTER", "Can not set a filter for tracing, because Xdebug mode does not include 'trace'");
				return;
			}

			filter_list = &XG_BASE(filters_tracing);
			XG_BASE(filter_type_tracing) = XDEBUG_FILTER_NONE;
			break;

		default:
			php_error(E_WARNING, "Filter group needs to be one of XDEBUG_FILTER_CODE_COVERAGE, XDEBUG_FILTER_STACK, or XDEBUG_FILTER_TRACING");
			return;
	}

	if (
		filter_type == XDEBUG_PATH_INCLUDE ||
		filter_type == XDEBUG_PATH_EXCLUDE ||
		filter_type == XDEBUG_NAMESPACE_INCLUDE ||
		filter_type == XDEBUG_NAMESPACE_EXCLUDE ||
		filter_type == XDEBUG_FILTER_NONE
	) {
		switch (filter_group) {
			case XDEBUG_FILTER_CODE_COVERAGE:
				XG_BASE(filter_type_code_coverage) = filter_type;
				break;

			case XDEBUG_FILTER_STACK:
				XG_BASE(filter_type_stack) = filter_type;
				break;

			case XDEBUG_FILTER_TRACING:
				XG_BASE(filter_type_tracing) = filter_type;
				break;
		}
	} else {
		php_error(E_WARNING, "Filter type needs to be one of XDEBUG_PATH_INCLUDE, XDEBUG_PATH_EXCLUDE, XDEBUG_NAMESPACE_INCLUDE, XDEBUG_NAMESPACE_EXCLUDE, or XDEBUG_FILTER_NONE");
		return;
	}

	xdebug_llist_empty(*filter_list, NULL);

	if (filter_type == XDEBUG_FILTER_NONE) {
		return;
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(filters), item) {
		zend_string *str = zval_get_string(item);
		char *filter = ZSTR_VAL(str);

		/* If we are a namespace filter, and the filter name starts with \, we
		 * need to strip the \ from the matcher */
		xdebug_llist_insert_next(*filter_list, XDEBUG_LLIST_TAIL(*filter_list), xdstrdup(filter[0] == '\\' ? &filter[1] : filter));

		zend_string_release(str);
	} ZEND_HASH_FOREACH_END();
}
/* }}} */
