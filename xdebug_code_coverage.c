/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003 Derick Rethans                              |
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

#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_var.h"
#include "xdebug_code_coverage.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

void xdebug_coverage_line_dtor(void *data)
{
	xdebug_coverage_line *line = (xdebug_coverage_line *) data;

	xdfree(line);
}

void xdebug_coverage_file_dtor(void *data)
{
	xdebug_coverage_file *file = (xdebug_coverage_file *) data;

	xdebug_hash_destroy(file->lines);
	xdfree(file->name);
	xdfree(file);
}

void xdebug_count_line(char *filename, int lineno, int executable TSRMLS_DC)
{
	xdebug_coverage_file *file;
	xdebug_coverage_line *line;
	char *sline;

	sline = xdebug_sprintf("%d", lineno);

	/* Check if the file already exists in the hash */
	if (!xdebug_hash_find(XG(code_coverage), filename, strlen(filename), (void *) &file)) {
		/* The file does not exist, so we add it to the hash, and
		 *  add a line element to the file */
		file = xdmalloc(sizeof(xdebug_coverage_file));
		file->name = xdstrdup(filename);
		file->lines = xdebug_hash_alloc(128, xdebug_coverage_line_dtor);
		
		xdebug_hash_add(XG(code_coverage), filename, strlen(filename), file);
	}

	/* Check if the line already exists in the hash */
	if (!xdebug_hash_find(file->lines, sline, strlen(sline), (void *) &line)) {
		line = xdmalloc(sizeof(xdebug_coverage_line));
		line->lineno = lineno;
		line->count = 0;
		line->executable = 0;

		xdebug_hash_add(file->lines, sline, strlen(sline), line);
	}

	if (executable) {
		line->executable = 1;
	} else {
		line->count++;
	}

	xdfree(sline);
}

static void prefil_from_opcode(function_stack_entry *fse, char *fn, zend_op opcode TSRMLS_DC)
{
	xdebug_count_line(fn, opcode.lineno, 1 TSRMLS_CC);
}

static int prefil_from_function_table(zend_op_array *opa, int num_args, va_list args, zend_hash_key *hash_key)
{
	char *new_filename;
	unsigned int i;
	TSRMLS_FETCH();

	new_filename = va_arg(args, char*);
	if (opa->type == ZEND_USER_FUNCTION) {
		if (opa->filename && strcmp(opa->filename, new_filename) == 0) {
			for (i = 0; i < opa->size; i++) {
				prefil_from_opcode(NULL, new_filename, opa->opcodes[i] TSRMLS_CC);
			}
		}
	}

	return ZEND_HASH_APPLY_KEEP;
}

#ifdef ZEND_ENGINE_2
static int prefil_from_class_table(zend_class_entry **class_entry, int num_args, va_list args, zend_hash_key *hash_key)
#else
static int prefil_from_class_table(zend_class_entry *class_entry, int num_args, va_list args, zend_hash_key *hash_key)
#endif
{
	char *new_filename;
	zend_class_entry *ce;

#ifdef ZEND_ENGINE_2
	ce = *class_entry;
#else
	ce = class_entry;
#endif

	new_filename = va_arg(args, char*);
	if (ce->type == ZEND_USER_CLASS) {
		zend_hash_apply_with_arguments(&ce->function_table, (apply_func_args_t) prefil_from_function_table, 1, new_filename);
	}

	return ZEND_HASH_APPLY_KEEP;
}

void xdebug_prefil_code_coverage(function_stack_entry *fse, zend_op_array *op_array TSRMLS_DC)
{
	unsigned int i;

	for (i = 0; i < op_array->size; i++) {
		prefil_from_opcode(fse, op_array->filename, op_array->opcodes[i] TSRMLS_CC);
	}

	zend_hash_apply_with_arguments(CG(function_table), (apply_func_args_t) prefil_from_function_table, 1, op_array->filename);
	zend_hash_apply_with_arguments(CG(class_table), (apply_func_args_t) prefil_from_class_table, 1, op_array->filename);
}

PHP_FUNCTION(xdebug_start_code_coverage)
{
	long options = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &options) == FAILURE) {
		return;
	}
	XG(code_coverage_unused) = (options == XDEBUG_CC_OPTION_UNUSED);

	if (XG(extended_info)) {
		XG(do_code_coverage) = 1;
	} else {
		php_error(E_WARNING, "You can only use code coverage when you leave the setting of 'xdebug.extended_info' to the default '1'.");
	}
}

PHP_FUNCTION(xdebug_stop_code_coverage)
{
	long cleanup = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &cleanup) == FAILURE) {
		return;
	}
	if (XG(do_code_coverage)) {
		if (cleanup) {
			xdebug_hash_destroy(XG(code_coverage));
			XG(code_coverage) = xdebug_hash_alloc(32, xdebug_coverage_file_dtor);
		}
		XG(do_code_coverage) = 0;
	}
}


static int xdebug_lineno_cmp(const void *a, const void *b TSRMLS_DC)
{
	Bucket *f = *((Bucket **) a);
	Bucket *s = *((Bucket **) b);

	if (f->h < s->h) {
		return -1;
	} else if (f->h > s->h) {
		return 1;
	} else {
		return 0;
	}
}


static void add_line(void *ret, xdebug_hash_element *e)
{
	xdebug_coverage_line *line = (xdebug_coverage_line*) e->ptr;
	zval                 *retval = (zval*) ret;

	if (line->executable && (line->count == 0)) {
		add_index_long(retval, line->lineno, -1);
	} else {
		add_index_long(retval, line->lineno, 1);
	}
}

static void add_file(void *ret, xdebug_hash_element *e)
{
	xdebug_coverage_file *file = (xdebug_coverage_file*) e->ptr;
	zval                 *retval = (zval*) ret;
	zval                 *lines;
	HashTable            *target_hash;
	TSRMLS_FETCH();

	MAKE_STD_ZVAL(lines);
	array_init(lines);

	/* Add all the lines */
	xdebug_hash_apply(file->lines, (void *) lines, add_line);

	/* Sort on linenumber */
	target_hash = HASH_OF(lines);
	zend_hash_sort(target_hash, zend_qsort, xdebug_lineno_cmp, 0 TSRMLS_CC);

	add_assoc_zval_ex(retval, file->name, strlen(file->name) + 1, lines);
}

PHP_FUNCTION(xdebug_get_code_coverage)
{
	array_init(return_value);
	xdebug_hash_apply(XG(code_coverage), (void *) return_value, add_file);
}

PHP_FUNCTION(xdebug_get_function_count)
{
	RETURN_LONG(XG(function_count));
}
