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

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

void profile_call_entry_dtor(void *dummy, void *elem)
{
	xdebug_call_entry *ce = elem;

	if (ce->function) {
		xdfree(ce->function);
	}
	xdfree(ce);
}

int xdebug_profiler_init(char *script_name)
{
	char *filename = xdebug_sprintf("%s/cachegrind.out.%ld", XG(profiler_output_dir), getpid());

	XG(profile_file) = fopen(filename, "w");
	if (!XG(profile_file)) {
		return FAILURE;
	} 
	fprintf(XG(profile_file), "version: 0.9.6\ncmd: %s\npart: 1\n\nevents: Time\nsummary: 900\n\n", script_name);
	return SUCCESS;
}

static inline void xdebug_spaces(function_stack_entry *fse)
{
	fprintf(XG(profile_file), "%*s", fse->level * 2, "");
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

void xdebug_profiler_function_user_begin(function_stack_entry *fse)
{
	fse->profile.time = 0;
	fse->profile.mark = xdebug_get_utime();
}


void xdebug_profiler_function_user_end(function_stack_entry *fse)
{
	xdebug_llist_element *le;
    char                 *tmp_fname, *tmp_name;

	xdebug_profiler_function_push(fse);
	tmp_name = show_fname(fse->function, 0, 0 TSRMLS_CC);
	switch (fse->function.type) {
		case XFUNC_INCLUDE:
		case XFUNC_INCLUDE_ONCE:
		case XFUNC_REQUIRE:
		case XFUNC_REQUIRE_ONCE:
			tmp_fname = xdebug_sprintf("%s::%s", tmp_name, get_zval_value(fse->vars[0].addr));
			xdfree(tmp_name);
			tmp_name = tmp_fname;
	}

	if (fse->prev) {
		xdebug_call_entry *ce = xdmalloc(sizeof(xdebug_call_entry));
		ce->function = xdstrdup(tmp_name);
		ce->time_taken = fse->profile.time;

		xdebug_llist_insert_next(fse->prev->profile.call_list, NULL, ce);
	}

	fprintf(XG(profile_file), "fl=%s\n", fse->filename);
	fprintf(XG(profile_file), "fn=%s\n", tmp_name);
	xdfree(tmp_name);

	if (fse->function.function && strcmp(fse->function.function, "{main}") == 0) {
		fprintf(XG(profile_file), "\nsummary: %ld\n\n", (long) (fse->profile.time * 10000000));
	}

	/* Subtract time in calledfunction from time here */
	for (le = XDEBUG_LLIST_HEAD(fse->profile.call_list); le != NULL; le = XDEBUG_LLIST_NEXT(le))
	{
		xdebug_call_entry *call_entry = XDEBUG_LLIST_VALP(le);
		fse->profile.time -= call_entry->time_taken;
	}
	fprintf(XG(profile_file), "0 %ld\n", (long) (fse->profile.time * 10000000));

	/* dump call list */
	for (le = XDEBUG_LLIST_HEAD(fse->profile.call_list); le != NULL; le = XDEBUG_LLIST_NEXT(le))
	{
		xdebug_call_entry *call_entry = XDEBUG_LLIST_VALP(le);

		fprintf(XG(profile_file), "cfn=%s\n", call_entry->function);
		
		fprintf(XG(profile_file), "calls=1 0\n");
		fprintf(XG(profile_file), "0 %ld\n", (long) (call_entry->time_taken * 10000000));
	}
	fprintf(XG(profile_file), "\n");

}


void xdebug_profiler_function_internal_begin(function_stack_entry *fse)
{
	xdebug_profiler_function_user_begin(fse);
}


void xdebug_profiler_function_internal_end(function_stack_entry *fse)
{
	xdebug_profiler_function_user_end(fse);
}


