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

#ifdef PHP_WIN32
#include <process.h>
#endif

#include "php.h"
#include "TSRM.h"
#include "php_globals.h"
#include "Zend/zend_alloc.h"

#include "php_xdebug.h"
#include "profiler.h"
#include "profiler_private.h"

#include "lib/mm.h"
#include "lib/str.h"
#include "lib/var.h"
#include "lib/usefulstuff.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

void xdebug_init_profiler_globals(xdebug_profiler_globals_t *xg)
{
	xg->profiler_enabled = 0;
}

void xdebug_profiler_minit(void)
{
}

void xdebug_profiler_mshutdown(void)
{
}

void xdebug_profiler_rinit(void)
{
	XG_PROF(profile_file)  = NULL;
	XG_PROF(profile_filename) = NULL;
	XG_PROF(profile_filename_refs) = NULL;
	XG_PROF(profile_functionname_refs) = NULL;
	XG_PROF(profile_last_filename_ref) = 0;
	XG_PROF(profile_last_functionname_ref) = 0;
	XG_PROF(profiler_enabled) = 0;
}

void xdebug_profiler_post_deactivate(void)
{
	if (XG_PROF(profiler_enabled)) {
		xdebug_profiler_deinit();
	}
}

void xdebug_profiler_pcntl_exec_handler(void)
{
	if (XG_PROF(profiler_enabled)) {
		xdebug_profiler_deinit();
	}
}

void xdebug_profiler_exit_handler(void)
{
	if (XG_PROF(profiler_enabled)) {
		xdebug_profiler_deinit();
	}
}


void xdebug_profiler_init_if_requested(zend_op_array *op_array)
{
	/* Check for special GET/POST parameter to start profiling */
	if (
		!XG_PROF(profiler_enabled) &&
		(XINI_PROF(profiler_enable) || xdebug_trigger_enabled(XINI_PROF(profiler_enable_trigger), "XDEBUG_PROFILE", XINI_PROF(profiler_enable_trigger_value)))
	) {
		xdebug_profiler_init((char*) STR_NAME_VAL(op_array->filename));
	}
}

void xdebug_profiler_execute_ex(function_stack_entry *fse, zend_op_array *op_array)
{
	if (XG_PROF(profiler_enabled)) {
		/* Calculate all elements for profile entries */
		xdebug_profiler_add_function_details_user(fse, op_array);
		xdebug_profiler_function_begin(fse);
	}
}

void xdebug_profiler_execute_ex_end(function_stack_entry *fse)
{
	if (XG_PROF(profiler_enabled)) {
		xdebug_profiler_function_end(fse);
		xdebug_profiler_free_function_details(fse);
	}
}

void xdebug_profiler_execute_internal(function_stack_entry *fse)
{
	if (XG_PROF(profiler_enabled)) {
		xdebug_profiler_add_function_details_internal(fse);
		xdebug_profiler_function_begin(fse);
	}
}

void xdebug_profiler_execute_internal_end(function_stack_entry *fse)
{
	if (XG_PROF(profiler_enabled)) {
		xdebug_profiler_function_end(fse);
		xdebug_profiler_free_function_details(fse);
	}
}

void xdebug_profile_call_entry_dtor(void *dummy, void *elem)
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

static void profiler_write_header(FILE *file, char *script_name)
{
	if (XINI_PROF(profiler_append)) {
		fprintf(file, "\n==== NEW PROFILING FILE ==============================================\n");
	}
	fprintf(file, "version: 1\ncreator: xdebug %s (PHP %s)\n", XDEBUG_VERSION, PHP_VERSION);
	fprintf(file, "cmd: %s\npart: 1\npositions: line\n\n", script_name);
	fprintf(file, "events: Time Memory\n\n");
	fflush(file);
}

void xdebug_profiler_init(char *script_name)
{
	char *filename = NULL, *fname = NULL;

	if (XG_PROF(profiler_enabled)) {
		return;
	}

	if (!strlen(XINI_PROF(profiler_output_name)) ||
		xdebug_format_output_filename(&fname, XINI_PROF(profiler_output_name), script_name) <= 0
	) {
		/* Invalid or empty xdebug.profiler_output_name */
		return;
	}

	/* Add a slash if none is present in the profiler_output_dir setting */
	if (IS_SLASH(XINI_PROF(profiler_output_dir)[strlen(XINI_PROF(profiler_output_dir)) - 1])) {
		filename = xdebug_sprintf("%s%s", XINI_PROF(profiler_output_dir), fname);
	} else {
		filename = xdebug_sprintf("%s%c%s", XINI_PROF(profiler_output_dir), DEFAULT_SLASH, fname);
	}
	xdfree(fname);

	if (XINI_PROF(profiler_append)) {
		XG_PROF(profile_file) = xdebug_fopen(filename, "a", NULL, &XG_PROF(profile_filename));
	} else {
		XG_PROF(profile_file) = xdebug_fopen(filename, "w", NULL, &XG_PROF(profile_filename));
	}
	xdfree(filename);

	if (!XG_PROF(profile_file)) {
		return;
	}

	profiler_write_header(XG_PROF(profile_file), script_name);

	if (!SG(headers_sent)) {
		sapi_header_line ctr = {0};

		ctr.line = xdebug_sprintf("X-Xdebug-Profile-Filename: %s", XG_PROF(profile_filename));
		ctr.line_len = strlen(ctr.line);
		sapi_header_op(SAPI_HEADER_REPLACE, &ctr);
		xdfree(ctr.line);
	}

	XG_PROF(profiler_start_time) = xdebug_get_utime();

	XG_PROF(profiler_enabled) = 1;
	XG_PROF(profile_filename_refs) = xdebug_hash_alloc(128, NULL);
	XG_PROF(profile_functionname_refs) = xdebug_hash_alloc(128, NULL);
	XG_PROF(profile_last_filename_ref) = 0;
	XG_PROF(profile_last_functionname_ref) = 0;
	return;
}

void xdebug_profiler_deinit()
{
	function_stack_entry *fse;
	xdebug_llist_element *le;

	for (le = XDEBUG_LLIST_TAIL(XG_BASE(stack)); le != NULL; le = XDEBUG_LLIST_PREV(le)) {
		fse = XDEBUG_LLIST_VALP(le);
		xdebug_profiler_function_end(fse);
	}

	fprintf(
		XG_PROF(profile_file),
		"summary: %lu %zd\n\n",
		(unsigned long) ((xdebug_get_utime() - (XG_PROF(profiler_start_time))) * 1000000),
		zend_memory_peak_usage(0)
	);

	XG_PROF(profiler_enabled) = 0;

	fflush(XG_PROF(profile_file));

	if (XG_PROF(profile_file)) {
		fclose(XG_PROF(profile_file));
		XG_PROF(profile_file) = NULL;
	}

	if (XG_PROF(profile_filename)) {
		xdfree(XG_PROF(profile_filename));
	}

	xdebug_hash_destroy(XG_PROF(profile_filename_refs));
	xdebug_hash_destroy(XG_PROF(profile_functionname_refs));
	XG_PROF(profile_filename_refs) = NULL;
	XG_PROF(profile_functionname_refs) = NULL;
}

static inline void xdebug_profiler_function_push(function_stack_entry *fse)
{
	fse->profile.time += xdebug_get_utime();
	fse->profile.time -= fse->profile.mark;
	fse->profile.mark = 0;
	fse->profile.memory += zend_memory_usage(0);
	fse->profile.memory -= fse->profile.mem_mark;
	fse->profile.mem_mark = 0;
}

void xdebug_profiler_function_continue(function_stack_entry *fse)
{
	fse->profile.mark = xdebug_get_utime();
}

void xdebug_profiler_function_pause(function_stack_entry *fse)
{
	xdebug_profiler_function_push(fse);
}

static char* get_filename_ref(char *name)
{
	long nr;

	if (xdebug_hash_find(XG_PROF(profile_filename_refs), name, strlen(name), (void*) &nr)) {
		return xdebug_sprintf("(%d)", nr);
	} else {
		XG_PROF(profile_last_filename_ref)++;
		xdebug_hash_add(XG_PROF(profile_filename_refs), name, strlen(name), (void*) (size_t) XG_PROF(profile_last_filename_ref));
		return xdebug_sprintf("(%d) %s", XG_PROF(profile_last_filename_ref), name);
	}
}

static char* get_functionname_ref(char *name)
{
	long nr;

	if (xdebug_hash_find(XG_PROF(profile_functionname_refs), name, strlen(name), (void*) &nr)) {
		return xdebug_sprintf("(%d)", nr);
	} else {
		XG_PROF(profile_last_functionname_ref)++;
		xdebug_hash_add(XG_PROF(profile_functionname_refs), name, strlen(name), (void*) (size_t) XG_PROF(profile_last_functionname_ref));
		return xdebug_sprintf("(%d) %s", XG_PROF(profile_last_functionname_ref), name);
	}
}

void xdebug_profiler_add_function_details_user(function_stack_entry *fse, zend_op_array *op_array)
{
	char *tmp_fname, *tmp_name;

	tmp_name = xdebug_show_fname(fse->function, 0, 0);
	switch (fse->function.type) {
		case XFUNC_INCLUDE:
		case XFUNC_INCLUDE_ONCE:
		case XFUNC_REQUIRE:
		case XFUNC_REQUIRE_ONCE:
			tmp_fname = xdebug_sprintf("%s::%s", tmp_name, fse->include_filename);
			xdfree(tmp_name);
			tmp_name = tmp_fname;
			fse->profiler.lineno = 1;
			break;

		default:
			if (op_array/* && op_array->function_name*/) {
				fse->profiler.lineno = fse->op_array->line_start;
			} else {
				fse->profiler.lineno = fse->lineno;
			}
			break;
	}
	if (fse->profiler.lineno == 0) {
		fse->profiler.lineno = 1;
	}

	if (op_array && op_array->filename) {
		fse->profiler.filename = xdstrdup((char*) STR_NAME_VAL(op_array->filename));
	} else {
		fse->profiler.filename = xdstrdup(fse->filename);
	}
	fse->profiler.funcname = xdstrdup(tmp_name);
	xdfree(tmp_name);
}

void xdebug_profiler_add_function_details_internal(function_stack_entry *fse)
{
	char *tmp_fname, *tmp_name;

	tmp_name = xdebug_show_fname(fse->function, 0, 0);
	switch (fse->function.type) {
		case XFUNC_INCLUDE:
		case XFUNC_INCLUDE_ONCE:
		case XFUNC_REQUIRE:
		case XFUNC_REQUIRE_ONCE:
			tmp_fname = xdebug_sprintf("%s::%s", tmp_name, fse->include_filename);
			xdfree(tmp_name);
			tmp_name = tmp_fname;
			fse->profiler.lineno = 1;
			break;

		default:
			fse->profiler.lineno = fse->lineno;
			break;
	}
	if (fse->profiler.lineno == 0) {
		fse->profiler.lineno = 1;
	}

	fse->profiler.filename = xdstrdup(fse->filename);
	fse->profiler.funcname = xdstrdup(tmp_name);

	xdfree(tmp_name);
}

void xdebug_profiler_function_begin(function_stack_entry *fse)
{
	fse->profile.time = 0;
	fse->profile.mark = xdebug_get_utime();
	fse->profile.memory = 0;
	fse->profile.mem_mark = zend_memory_usage(0);
}

void xdebug_profiler_function_end(function_stack_entry *fse)
{
	xdebug_llist_element *le;

	if (fse->prev && !fse->prev->profile.call_list) {
		fse->prev->profile.call_list = xdebug_llist_alloc(xdebug_profile_call_entry_dtor);
	}
	if (!fse->profile.call_list) {
		fse->profile.call_list = xdebug_llist_alloc(xdebug_profile_call_entry_dtor);
	}
	xdebug_profiler_function_push(fse);

	if (fse->prev) {
		xdebug_call_entry *ce = xdmalloc(sizeof(xdebug_call_entry));
		ce->filename = xdstrdup(fse->profiler.filename);
		ce->function = xdstrdup(fse->profiler.funcname);
		ce->time_taken = fse->profile.time;
		ce->lineno = fse->lineno;
		ce->user_defined = fse->user_defined;
		ce->mem_used = fse->profile.memory;

		xdebug_llist_insert_next(fse->prev->profile.call_list, NULL, ce);
	}

	/* use previously created filename and funcname (or a reference to them) to show
	 * time spend */
	if (fse->user_defined == XDEBUG_BUILT_IN) {
		char *tmp_key = xdebug_sprintf("php::%s", fse->profiler.funcname);
		char *fl_ref = NULL, *fn_ref = NULL;

		fl_ref = get_filename_ref((char*) "php:internal");
		fn_ref = get_functionname_ref(tmp_key);

		fprintf(XG_PROF(profile_file), "fl=%s\n", fl_ref);
		fprintf(XG_PROF(profile_file), "fn=%s\n", fn_ref);

		xdfree(fl_ref);
		xdfree(fn_ref);
		xdfree(tmp_key);
	} else {
		char *fl_ref = NULL, *fn_ref = NULL;

		fl_ref = get_filename_ref(fse->profiler.filename);
		fn_ref = get_functionname_ref(fse->profiler.funcname);

		fprintf(XG_PROF(profile_file), "fl=%s\n", fl_ref);
		fprintf(XG_PROF(profile_file), "fn=%s\n", fn_ref);

		xdfree(fl_ref);
		xdfree(fn_ref);
	}


	/* Subtract time in calledfunction from time here */
	for (le = XDEBUG_LLIST_HEAD(fse->profile.call_list); le != NULL; le = XDEBUG_LLIST_NEXT(le))
	{
		xdebug_call_entry *call_entry = XDEBUG_LLIST_VALP(le);
		fse->profile.time -= call_entry->time_taken;
		fse->profile.memory -= call_entry->mem_used;
	}
	fprintf(XG_PROF(profile_file), "%d %lu %ld\n", fse->profiler.lineno, (unsigned long) (fse->profile.time * 1000000), (fse->profile.memory));

	/* dump call list */
	for (le = XDEBUG_LLIST_HEAD(fse->profile.call_list); le != NULL; le = XDEBUG_LLIST_NEXT(le))
	{
		char *fl_ref = NULL, *fn_ref = NULL;
		xdebug_call_entry *call_entry = XDEBUG_LLIST_VALP(le);

		if (call_entry->user_defined == XDEBUG_BUILT_IN) {
			char *tmp_key = xdebug_sprintf("php::%s", call_entry->function);

			fl_ref = get_filename_ref((char*) "php:internal");
			fn_ref = get_functionname_ref(tmp_key);

			xdfree(tmp_key);
		} else {
			fl_ref = get_filename_ref(call_entry->filename);
			fn_ref = get_functionname_ref(call_entry->function);
		}

		fprintf(XG_PROF(profile_file), "cfl=%s\n", fl_ref);
		fprintf(XG_PROF(profile_file), "cfn=%s\n", fn_ref);

		xdfree(fl_ref);
		xdfree(fn_ref);

		fprintf(XG_PROF(profile_file), "calls=1 0 0\n");
		fprintf(XG_PROF(profile_file), "%d %lu %ld\n", call_entry->lineno, (unsigned long) (call_entry->time_taken * 1000000), (call_entry->mem_used));
	}
	fprintf(XG_PROF(profile_file), "\n");
	fflush(XG_PROF(profile_file));
}

void xdebug_profiler_free_function_details(function_stack_entry *fse)
{
	xdfree(fse->profiler.funcname);
	xdfree(fse->profiler.filename);
	fse->profiler.funcname = NULL;
	fse->profiler.filename = NULL;
}

/* Returns a *pointer* to the current profile filename, if active. NULL
 * otherwise. Calling function is responsible for duplicating immediately */
char *xdebug_get_profiler_filename()
{
	if (XG_PROF(profiler_enabled) && XG_PROF(profile_filename)) {
		return XG_PROF(profile_filename);
	}

	return NULL;
}

PHP_FUNCTION(xdebug_get_profiler_filename)
{
	char *filename = xdebug_get_profiler_filename();

	if (filename) {
		RETURN_STRING(filename);
	} else {
		RETURN_FALSE;
	}
}
