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
   |          Michael Voříšek <mvorisek@mvorisek.cz>                      |
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

#include "lib/log.h"
#include "lib/mm.h"
#include "lib/str.h"
#include "lib/var.h"
#include "lib/usefulstuff.h"

ZEND_EXTERN_MODULE_GLOBALS(xdebug)

int xdebug_profiler_exit_handler(XDEBUG_OPCODE_HANDLER_ARGS);

void xdebug_init_profiler_globals(xdebug_profiler_globals_t *xg)
{
	xg->active = 0;
}

void xdebug_profiler_minit(void)
{
	/* Overload the "exit" opcode */
	xdebug_set_opcode_handler(ZEND_EXIT, xdebug_profiler_exit_handler);
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
	XG_PROF(php_internal_seen_before) = 0;
	XG_PROF(profile_last_functionname_ref) = 0;
	XG_PROF(active) = 0;
}

static void deinit_if_active(void)
{
	if (!XG_PROF(active)) {
		return;
	}

	xdebug_profiler_deinit();
}

void xdebug_profiler_post_deactivate(void)
{
	deinit_if_active();
}

void xdebug_profiler_pcntl_exec_handler(void)
{
	deinit_if_active();
}

int xdebug_profiler_exit_handler(XDEBUG_OPCODE_HANDLER_ARGS)
{
	const zend_op *cur_opcode = execute_data->opline;

	deinit_if_active();

	return xdebug_call_original_opcode_handler_if_set(cur_opcode->opcode, XDEBUG_OPCODE_HANDLER_ARGS_PASSTHRU);
}


void xdebug_profiler_init_if_requested(zend_op_array *op_array)
{
	if (XG_PROF(active)) {
		return;
	}

	if (EG(flags) & EG_FLAGS_IN_SHUTDOWN) {
		return;
	}

	if (xdebug_lib_start_with_request(XDEBUG_MODE_PROFILING) || xdebug_lib_start_with_trigger(XDEBUG_MODE_PROFILING, NULL)) {
		xdebug_profiler_init((char*) STR_NAME_VAL(op_array->filename));
	}
}

void xdebug_profiler_execute_ex(function_stack_entry *fse, zend_op_array *op_array)
{
	if (!XG_PROF(active)) {
		return;
	}

	/* Calculate all elements for profile entries */
	xdebug_profiler_add_function_details_user(fse, op_array);
	xdebug_profiler_function_begin(fse);
}

void xdebug_profiler_execute_ex_end(function_stack_entry *fse)
{
	xdebug_profiler_function_end(fse);
	xdebug_profiler_free_function_details(fse);
}

void xdebug_profiler_execute_internal(function_stack_entry *fse)
{
	if (!XG_PROF(active)) {
		return;
	}

	xdebug_profiler_add_function_details_internal(fse);
	xdebug_profiler_function_begin(fse);
}

void xdebug_profiler_execute_internal_end(function_stack_entry *fse)
{
	if (!XG_PROF(active)) {
		return;
	}

	xdebug_profiler_function_end(fse);
	xdebug_profiler_free_function_details(fse);
}

void xdebug_profile_call_entry_dtor(void *dummy, void *elem)
{
	xdebug_call_entry *ce = elem;

	if (ce->function) {
		xdfree(ce->function);
	}
	if (ce->filename) {
		zend_string_release(ce->filename);
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
	fprintf(file, "events: Time_(10ns) Memory_(bytes)\n\n");
	fflush(file);
}

#define NANOTIME_SCALE_10NS(nanotime) ((unsigned long)(((nanotime) + 5) / 10))

void xdebug_profiler_init(char *script_name)
{
	char *filename = NULL, *fname = NULL;
	char *output_dir = NULL;

	if (XG_PROF(active)) {
		return;
	}

	if (!strlen(XINI_PROF(profiler_output_name)) ||
		xdebug_format_output_filename(&fname, XINI_PROF(profiler_output_name), script_name) <= 0
	) {
		/* Invalid or empty xdebug.profiler_output_name */
		return;
	}

	/* Add a slash if none is present in the output_dir setting */
	output_dir = xdebug_lib_get_output_dir(); /* not duplicated */

	if (IS_SLASH(output_dir[strlen(output_dir) - 1])) {
		filename = xdebug_sprintf("%s%s", output_dir, fname);
	} else {
		filename = xdebug_sprintf("%s%c%s", output_dir, DEFAULT_SLASH, fname);
	}

	if (XINI_PROF(profiler_append)) {
		XG_PROF(profile_file) = xdebug_fopen(filename, "a", NULL, &XG_PROF(profile_filename));
	} else {
		XG_PROF(profile_file) = xdebug_fopen(filename, "w", NULL, &XG_PROF(profile_filename));
	}

	if (!XG_PROF(profile_file)) {
		xdebug_log_diagnose_permissions(XLOG_CHAN_PROFILE, output_dir, fname);
		goto return_and_free_names;
	}

	profiler_write_header(XG_PROF(profile_file), script_name);

	if (!SG(headers_sent)) {
		sapi_header_line ctr = {0};

		ctr.line = xdebug_sprintf("X-Xdebug-Profile-Filename: %s", XG_PROF(profile_filename));
		ctr.line_len = strlen(ctr.line);
		sapi_header_op(SAPI_HEADER_REPLACE, &ctr);
		xdfree((void*) ctr.line);
	}

	XG_PROF(profiler_start_nanotime) = xdebug_get_nanotime();

	XG_PROF(active) = 1;
	XG_PROF(profile_filename_refs) = xdebug_hash_alloc(128, xdfree);
	XG_PROF(profile_functionname_refs) = xdebug_hash_alloc(128, xdfree);
	XG_PROF(profile_last_filename_ref) = 1;
	XG_PROF(profile_last_functionname_ref) = 0;

return_and_free_names:
	xdfree(filename);
	xdfree(fname);
}

void xdebug_profiler_deinit()
{
	function_stack_entry *fse = XDEBUG_VECTOR_TAIL(XG_BASE(stack));
	int                   i;

	for (i = 0; i < XDEBUG_VECTOR_COUNT(XG_BASE(stack)); i++, fse--) {
		xdebug_profiler_function_end(fse);
	}

	fprintf(
		XG_PROF(profile_file),
		"summary: %lu %zd\n\n",
		NANOTIME_SCALE_10NS(xdebug_get_nanotime() - XG_PROF(profiler_start_nanotime)),
		zend_memory_peak_usage(0)
	);

	XG_PROF(active) = 0;

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
	fse->profile.nanotime += (xdebug_get_nanotime() - fse->profile.nanotime_mark);
	fse->profile.nanotime_mark = 0;
	fse->profile.memory += (zend_memory_usage(0) - fse->profile.mem_mark);
	fse->profile.mem_mark = 0;
}

void xdebug_profiler_function_continue(function_stack_entry *fse)
{
	fse->profile.nanotime_mark = xdebug_get_nanotime();
}

void xdebug_profiler_function_pause(function_stack_entry *fse)
{
	xdebug_profiler_function_push(fse);
}

static inline void add_filename_ref(xdebug_str *buffer, char *name)
{
	char *ref;

	if (xdebug_hash_find(XG_PROF(profile_filename_refs), name, strlen(name), (void*) &ref)) {
		xdebug_str_add(buffer, ref, 0);
	} else {
		XG_PROF(profile_last_filename_ref)++;
		ref = xdebug_sprintf("(%d)", XG_PROF(profile_last_filename_ref));

		xdebug_hash_add(XG_PROF(profile_filename_refs), name, strlen(name), (void*) ref);

		xdebug_str_add(buffer, ref, 0);
		xdebug_str_addc(buffer, ' ');
		xdebug_str_add(buffer, name,  0);
	}
}

static inline void add_functionname_ref(xdebug_str *buffer, char *name)
{
	char *ref;

	if (xdebug_hash_find(XG_PROF(profile_functionname_refs), name, strlen(name), (void*) &ref)) {
		xdebug_str_add(buffer, ref, 0);
	} else {
		XG_PROF(profile_last_functionname_ref)++;
		ref = xdebug_sprintf("(%d)", XG_PROF(profile_last_functionname_ref));

		xdebug_hash_add(XG_PROF(profile_functionname_refs), name, strlen(name), (void*) ref);

		xdebug_str_add(buffer, ref, 0);
		xdebug_str_addc(buffer, ' ');
		xdebug_str_add(buffer, name,  0);
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
			tmp_fname = xdebug_sprintf("%s::%s", tmp_name, ZSTR_VAL(fse->include_filename));
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
		fse->profiler.filename = zend_string_copy(op_array->filename);
	} else {
		fse->profiler.filename = zend_string_copy(fse->filename);
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

	fse->profiler.filename = zend_string_copy(fse->filename);
	fse->profiler.funcname = xdstrdup(tmp_name);

	xdfree(tmp_name);
}

void xdebug_profiler_function_begin(function_stack_entry *fse)
{
	fse->profile.nanotime = 0;
	fse->profile.nanotime_mark = xdebug_get_nanotime();
	fse->profile.memory = 0;
	fse->profile.mem_mark = zend_memory_usage(0);
}

#define TMP_KEY_BUFFER_LEN 1024
#define TMP_KEY_PREFIX     "php::"
#define TMP_KEY_PREFIX_LEN (sizeof(TMP_KEY_PREFIX)-1)
#define TMP_KEY_MAX_LEN    (TMP_KEY_BUFFER_LEN-TMP_KEY_PREFIX_LEN-1)

void xdebug_profiler_function_end(function_stack_entry *fse)
{
	xdebug_llist_element *le;
	xdebug_str file_buffer = XDEBUG_STR_INITIALIZER;
	char tmp_key[TMP_KEY_BUFFER_LEN];

	if (!XG_PROF(active)) {
		return;
	}

	/* The temporary key always starts with 'php::' */
	memcpy(tmp_key, TMP_KEY_PREFIX, TMP_KEY_PREFIX_LEN);

	if (xdebug_vector_element_is_valid(XG_BASE(stack), fse - 1) && !(fse - 1)->profile.call_list) {
		(fse - 1)->profile.call_list = xdebug_llist_alloc(xdebug_profile_call_entry_dtor);
	}
	if (!fse->profile.call_list) {
		fse->profile.call_list = xdebug_llist_alloc(xdebug_profile_call_entry_dtor);
	}
	xdebug_profiler_function_push(fse);

	if (xdebug_vector_element_is_valid(XG_BASE(stack), fse - 1)) {
		xdebug_call_entry *ce = xdmalloc(sizeof(xdebug_call_entry));

		ce->filename = zend_string_copy(fse->profiler.filename);
		ce->function = xdstrdup(fse->profiler.funcname);
		ce->nanotime_taken = fse->profile.nanotime;
		ce->lineno = fse->lineno;
		ce->user_defined = fse->user_defined;
		ce->mem_used = fse->profile.memory;

		xdebug_llist_insert_next((fse - 1)->profile.call_list, NULL, ce);
	}

	/* use previously created filename and funcname (or a reference to them) to show
	 * time spend */
	if (fse->user_defined == XDEBUG_BUILT_IN) {
		size_t tmp_key_funcname_len = strlen(fse->profiler.funcname);

		memcpy(tmp_key + TMP_KEY_PREFIX_LEN,
			fse->profiler.funcname,
			tmp_key_funcname_len > TMP_KEY_MAX_LEN ? TMP_KEY_MAX_LEN : tmp_key_funcname_len + 1
		);
		tmp_key[TMP_KEY_BUFFER_LEN - 1] = '\0';

		if (XG_PROF(php_internal_seen_before)) {
			xdebug_str_add_literal(&file_buffer, "fl=(1)\n");
		} else {
			xdebug_str_add_literal(&file_buffer, "fl=(1) php:internal\n");
			XG_PROF(php_internal_seen_before) = 1;
		}
		xdebug_str_add_literal(&file_buffer, "fn=");
		add_functionname_ref(&file_buffer, tmp_key);
		xdebug_str_addc(&file_buffer, '\n');
	} else {

		xdebug_str_add_literal(&file_buffer, "fl=");
		add_filename_ref(&file_buffer, ZSTR_VAL(fse->profiler.filename));

		xdebug_str_add_literal(&file_buffer, "\nfn=");
		add_functionname_ref(&file_buffer, fse->profiler.funcname);
		xdebug_str_addc(&file_buffer, '\n');
	}


	/* Subtract time in calledfunction from time here */
	for (le = XDEBUG_LLIST_HEAD(fse->profile.call_list); le != NULL; le = XDEBUG_LLIST_NEXT(le))
	{
		xdebug_call_entry *call_entry = XDEBUG_LLIST_VALP(le);
		fse->profile.nanotime -= call_entry->nanotime_taken;
		fse->profile.memory -= call_entry->mem_used;
	}

	/* Adds %d %lu %lu, with lineno, time, and memory */
	xdebug_str_add_uint64(&file_buffer, fse->profiler.lineno);
	xdebug_str_addc(&file_buffer, ' ');
	xdebug_str_add_uint64(&file_buffer, NANOTIME_SCALE_10NS(fse->profile.nanotime));
	xdebug_str_addc(&file_buffer, ' ');
	xdebug_str_add_uint64(&file_buffer, fse->profile.memory >= 0 ? fse->profile.memory : 0);
	xdebug_str_addc(&file_buffer, '\n');

	/* dump call list */
	for (le = XDEBUG_LLIST_HEAD(fse->profile.call_list); le != NULL; le = XDEBUG_LLIST_NEXT(le))
	{
		xdebug_call_entry *call_entry = XDEBUG_LLIST_VALP(le);

		if (call_entry->user_defined == XDEBUG_BUILT_IN) {
			size_t tmp_key_funcname_len = strlen(call_entry->function);

			memcpy(tmp_key + TMP_KEY_PREFIX_LEN,
				call_entry->function,
				tmp_key_funcname_len > TMP_KEY_MAX_LEN ? TMP_KEY_MAX_LEN : tmp_key_funcname_len + 1
			);
			tmp_key[TMP_KEY_BUFFER_LEN - 1] = '\0';

			if (XG_PROF(php_internal_seen_before)) {
				xdebug_str_add_literal(&file_buffer, "cfl=(1)\n");
			} else {
				xdebug_str_add_literal(&file_buffer, "cfl=(1) php:internal\n");
				XG_PROF(php_internal_seen_before) = 1;
			}

			xdebug_str_add_literal(&file_buffer, "cfn=");
			add_functionname_ref(&file_buffer, tmp_key);
			xdebug_str_addc(&file_buffer, '\n');
		} else {
			xdebug_str_add_literal(&file_buffer, "cfl=");
			add_filename_ref(&file_buffer, ZSTR_VAL(call_entry->filename));

			xdebug_str_add_literal(&file_buffer, "\ncfn=");
			add_functionname_ref(&file_buffer, call_entry->function);
			xdebug_str_addc(&file_buffer, '\n');
		}

		xdebug_str_add_literal(&file_buffer, "calls=1 0 0\n");

		/* Adds %d %lu %lu, with lineno, time, and memory */
		xdebug_str_add_uint64(&file_buffer, call_entry->lineno);
		xdebug_str_addc(&file_buffer, ' ');
		xdebug_str_add_uint64(&file_buffer, NANOTIME_SCALE_10NS(call_entry->nanotime_taken));
		xdebug_str_addc(&file_buffer, ' ');
		xdebug_str_add_uint64(&file_buffer, call_entry->mem_used >= 0 ? call_entry->mem_used : 0);
		xdebug_str_addc(&file_buffer, '\n');
	}
	xdebug_str_addc(&file_buffer, '\n');

	fwrite(file_buffer.d, sizeof(char), file_buffer.l, XG_PROF(profile_file));
	xdebug_str_dtor(file_buffer);
}

void xdebug_profiler_free_function_details(function_stack_entry *fse)
{
	if (fse->profiler.funcname) {
		xdfree(fse->profiler.funcname);
		fse->profiler.funcname = NULL;
	}
	if (fse->profiler.filename) {
		zend_string_release(fse->profiler.filename);
		fse->profiler.filename = NULL;
	}
}

/* Returns a *pointer* to the current profile filename, if active. NULL
 * otherwise. Calling function is responsible for duplicating immediately */
char *xdebug_get_profiler_filename()
{
	if (!XG_PROF(active)) {
		return NULL;
	}

	return XG_PROF(profile_filename);
}

PHP_FUNCTION(xdebug_get_profiler_filename)
{
	char *filename = xdebug_get_profiler_filename();

	if (!filename) {
		RETURN_FALSE;
	}

	RETURN_STRING(filename);
}
