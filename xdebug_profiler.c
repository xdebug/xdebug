/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003, 2004 Derick Rethans                        |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors: Derick Rethans <derick@xdebug.org>                          |
   +----------------------------------------------------------------------+
 */

#include "php.h"
#include "TSRM.h"
#include "php_globals.h"
#include "php_xdebug.h"
#include "xdebug_mm.h"
#include "xdebug_profiler.h"
#include "xdebug_str.h"
#include "xdebug_var.h"
#include "usefulstuff.h"
#ifdef PHP_WIN32
#include <process.h>
#endif

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

void profile_call_entry_dtor(void *dummy, void *elem)
{
	xdebug_call_entry *ce = elem;

	if (ce->function) {
		xdfree(ce->function);
	}
	if (ce->filename) {
		xdfree(ce->filename);
	}
	xdfree(ce);
}

int xdebug_profiler_init(char *script_name TSRMLS_DC)
{
	char *filename;

	if (strcmp(XG(profiler_output_name), "crc32") == 0) {
		filename = xdebug_sprintf("%s/cachegrind.out.%lu", XG(profiler_output_dir), xdebug_crc32(script_name, strlen(script_name)));
	} else {
		filename = xdebug_sprintf("%s/cachegrind.out.%ld", XG(profiler_output_dir), getpid());
	}

	XG(profile_file) = fopen(filename, "w");
	if (!XG(profile_file)) {
		return FAILURE;
	} 
	fprintf(XG(profile_file), "version: 0.9.6\ncmd: %s\npart: 1\n\nevents: Time Memory\n\n", script_name);
	return SUCCESS;
}

void xdebug_profiler_deinit(TSRMLS_D)
{
	function_stack_entry *fse;
	xdebug_llist_element *le;

	for (le = XDEBUG_LLIST_TAIL(XG(stack)); le != NULL; le = XDEBUG_LLIST_PREV(le)) {
		fse = XDEBUG_LLIST_VALP(le);
		if (fse->user_defined == XDEBUG_INTERNAL) {
			xdebug_profiler_function_internal_end(fse TSRMLS_CC);
		} else {
			xdebug_profiler_function_user_end(fse, fse->op_array TSRMLS_CC);
		}
	}
}

static inline void xdebug_profiler_function_push(function_stack_entry *fse)
{
	fse->profile.time += xdebug_get_utime();
	fse->profile.time -= fse->profile.mark;
	fse->profile.mark = 0;
}

void xdebug_profiler_function_continue(function_stack_entry *fse)
{
	fse->profile.mark = xdebug_get_utime();
}

void xdebug_profiler_function_pause(function_stack_entry *fse)
{
	xdebug_profiler_function_push(fse);
}

void xdebug_profiler_function_user_begin(function_stack_entry *fse TSRMLS_DC)
{
	fse->profile.time = 0;
	fse->profile.mark = xdebug_get_utime();
#if MEMORY_LIMIT
	fse->profile.memory = AG(allocated_memory);
#endif
}


void xdebug_profiler_function_user_end(function_stack_entry *fse, zend_op_array* op_array TSRMLS_DC)
{
	xdebug_llist_element *le;
    char                 *tmp_fname, *tmp_name;
	int                   default_lineno = 0;

	xdebug_profiler_function_push(fse);
	tmp_name = show_fname(fse->function, 0, 0 TSRMLS_CC);
	switch (fse->function.type) {
		case XFUNC_INCLUDE:
		case XFUNC_INCLUDE_ONCE:
		case XFUNC_REQUIRE:
		case XFUNC_REQUIRE_ONCE:
			tmp_fname = xdebug_sprintf("%s::%s", tmp_name, fse->include_filename);
			xdfree(tmp_name);
			tmp_name = tmp_fname;
			default_lineno = 1;
	}

	if (fse->prev) {
		xdebug_call_entry *ce = xdmalloc(sizeof(xdebug_call_entry));
		ce->filename = xdstrdup(fse->filename);
		ce->function = xdstrdup(tmp_name);
		ce->time_taken = fse->profile.time;
		ce->lineno = fse->lineno;
		ce->user_defined = fse->user_defined;
#if MEMORY_LIMIT
		ce->mem_used = fse->profile.memory - AG(allocated_memory);
#endif

		xdebug_llist_insert_next(fse->prev->profile.call_list, NULL, ce);
	}

	if (op_array) {
		fprintf(XG(profile_file), "fl=%s\n", op_array->filename);
	} else {
		fprintf(XG(profile_file), "fl=php:internal\n");
	}
	if (fse->user_defined == XDEBUG_EXTERNAL) {
		fprintf(XG(profile_file), "fn=%s\n", tmp_name);
	} else {
		fprintf(XG(profile_file), "fn=php::%s\n", tmp_name);
	}
	xdfree(tmp_name);

	if (fse->function.function && strcmp(fse->function.function, "{main}") == 0) {
#if MEMORY_LIMIT
		fprintf(XG(profile_file), "\nsummary: %lu %u\n\n", (unsigned long) (fse->profile.time * 10000000), AG(allocated_memory));
#else
		fprintf(XG(profile_file), "\nsummary: %lu\n\n", (unsigned long) (fse->profile.time * 10000000));
#endif
	}

	/* Subtract time in calledfunction from time here */
	for (le = XDEBUG_LLIST_HEAD(fse->profile.call_list); le != NULL; le = XDEBUG_LLIST_NEXT(le))
	{
		xdebug_call_entry *call_entry = XDEBUG_LLIST_VALP(le);
		fse->profile.time -= call_entry->time_taken;
#if MEMORY_LIMIT
		fse->memory -= call_entry->mem_used;
#endif
	}
#if MEMORY_LIMIT
	fprintf(XG(profile_file), "%d %lu %ld\n", default_lineno, (unsigned long) (fse->profile.time * 10000000), (AG(allocated_memory) - fse->profile.memory) < 0 ? 0 : (AG(allocated_memory) - fse->profile.memory));
#else
	fprintf(XG(profile_file), "%d %lu\n", default_lineno, (unsigned long) (fse->profile.time * 10000000));
#endif

	/* dump call list */
	for (le = XDEBUG_LLIST_HEAD(fse->profile.call_list); le != NULL; le = XDEBUG_LLIST_NEXT(le))
	{
		xdebug_call_entry *call_entry = XDEBUG_LLIST_VALP(le);

		if (call_entry->user_defined == XDEBUG_EXTERNAL) {
			fprintf(XG(profile_file), "cfn=%s\n", call_entry->function);
		} else {
			fprintf(XG(profile_file), "cfn=php::%s\n", call_entry->function);
		}
		
		fprintf(XG(profile_file), "calls=1 0 0\n");
#if MEMORY_LIMIT
		fprintf(XG(profile_file), "%d %lu %ld\n", call_entry->lineno, (unsigned long) (call_entry->time_taken * 10000000), call_entry->mem_used < 0 ? 0 : call_entry->mem_used);
#else
		fprintf(XG(profile_file), "%d %lu\n", call_entry->lineno, (unsigned long) (call_entry->time_taken * 10000000));
#endif
	}
	fprintf(XG(profile_file), "\n");

}


void xdebug_profiler_function_internal_begin(function_stack_entry *fse TSRMLS_DC)
{
	xdebug_profiler_function_user_begin(fse TSRMLS_CC);
}


void xdebug_profiler_function_internal_end(function_stack_entry *fse TSRMLS_DC)
{
	xdebug_profiler_function_user_end(fse, NULL TSRMLS_CC);
}


