/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007 Derick Rethans      |
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
#include "xdebug_set.h"
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

void xdebug_count_line(char *filename, int lineno, int executable, int deadcode TSRMLS_DC)
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
		if (line->executable != 1 && deadcode) {
			line->executable = 2;
		} else {
			line->executable = 1;
		}
	} else {
		line->count++;
	}

	xdfree(sline);
}

static void prefill_from_opcode(char *fn, zend_op opcode, int deadcode TSRMLS_DC)
{
	if (
		opcode.opcode != ZEND_NOP &&
		opcode.opcode != ZEND_EXT_NOP &&
		opcode.opcode != ZEND_RECV &&
		opcode.opcode != ZEND_RECV_INIT
#ifdef ZEND_ENGINE_2
		&& opcode.opcode != ZEND_VERIFY_ABSTRACT_CLASS
		&& opcode.opcode != ZEND_OP_DATA
		&& opcode.opcode != ZEND_ADD_INTERFACE
#endif
	) {
		xdebug_count_line(fn, opcode.lineno, 1, deadcode TSRMLS_CC);
	}
}

static zend_brk_cont_element* xdebug_find_brk_cont(zval *nest_levels_zval, int array_offset, zend_op_array *op_array)
{
	int nest_levels;
	zend_brk_cont_element *jmp_to;

	nest_levels = nest_levels_zval->value.lval;

	do {
		jmp_to = &op_array->brk_cont_array[array_offset];
		array_offset = jmp_to->parent;
	} while (--nest_levels > 0);
	return jmp_to;
}

static int xdebug_find_jump(zend_op_array *opa, unsigned int position, long *jmp1, long *jmp2)
{
	zend_op *base_address = &(opa->opcodes[0]);

	zend_op opcode = opa->opcodes[position];
	if (opcode.opcode == ZEND_JMP) {
#ifdef ZEND_ENGINE_2
		*jmp1 = ((long) opcode.op1.u.jmp_addr - (long) base_address) / sizeof(zend_op);
#else
		*jmp1 = opcode.op1.u.opline_num;
#endif
		return 1;
	} else if (
		opcode.opcode == ZEND_JMPZ ||
		opcode.opcode == ZEND_JMPNZ ||
		opcode.opcode == ZEND_JMPZ_EX ||
		opcode.opcode == ZEND_JMPNZ_EX
	) {
		*jmp1 = position + 1;
#ifdef ZEND_ENGINE_2
		*jmp2 = ((long) opcode.op2.u.jmp_addr - (long) base_address) / sizeof(zend_op);
#else
		*jmp2 = opcode.op1.u.opline_num;
#endif
		return 1;
	} else if (opcode.opcode == ZEND_JMPZNZ) {
		*jmp1 = opcode.op2.u.opline_num;
		*jmp2 = opcode.extended_value;
		return 1;
	} else if (opcode.opcode == ZEND_BRK || opcode.opcode == ZEND_CONT) {
		zend_brk_cont_element *el;

		if (opcode.op2.op_type == IS_CONST && opcode.op1.u.jmp_addr != (zend_op*) 0xFFFFFFFF) {
			el = xdebug_find_brk_cont(&opcode.op2.u.constant, opcode.op1.u.opline_num, opa);
			*jmp1 = opcode.opcode == ZEND_BRK ? el->brk : el->cont;
			return 1;
		}
	} else if (opcode.opcode == ZEND_FE_RESET || opcode.opcode == ZEND_FE_FETCH) {
		*jmp1 = position + 1;
		*jmp2 = opcode.op2.u.opline_num;
		return 1;
	}
	return 0;
}

static void xdebug_analyse_branch(zend_op_array *opa, unsigned int position, xdebug_set *set)
{
	long jump_pos1 = -1;
	long jump_pos2 = -1;

	/*(fprintf(stderr, "Branch analysis from position: %d\n", position);)*/
	/* First we see if the branch has been visited, if so we bail out. */
	if (xdebug_set_in(set, position)) {
		return;
	}
	/* Loop over the opcodes until the end of the array, or until a jump point has been found */
	xdebug_set_add(set, position);
	/*(fprintf(stderr, "XDEBUG Adding %d\n", position);)*/
	while (position < opa->size) {

		/* See if we have a jump instruction */
		if (xdebug_find_jump(opa, position, &jump_pos1, &jump_pos2)) {
			/*(fprintf(stderr, "XDEBUG Jump found. Position 1 = %d", jump_pos1);)*/
			if (jump_pos2 != -1) {
				/*(fprintf(stderr, ", Position 2 = %d\n", jump_pos2);)*/
			} else {
				/*(fprintf(stderr, "\n");)*/
			}
			xdebug_analyse_branch(opa, jump_pos1, set);
			if (jump_pos2 != -1) {
				xdebug_analyse_branch(opa, jump_pos2, set);
			}
			break;
		}
#ifdef ZEND_ENGINE_2
		/* See if we have a throw instruction */
		if (opa->opcodes[position].opcode == ZEND_THROW) {
			/* fprintf(stderr, "Throw found at %d\n", position); */
			/* Now we need to go forward to the first
			 * zend_fetch_class/zend_catch combo */
			while (position < opa->size) {
				if (opa->opcodes[position].opcode == ZEND_CATCH) {
					/* fprintf(stderr, "Found catch at %d\n", position); */
					position--;
					break;
				}
				position++;
				/* fprintf(stderr, "Skipping %d\n", position); */
			}
			position--;
		}
#endif
		/* See if we have an exit instruction */
		if (opa->opcodes[position].opcode == ZEND_EXIT) {
			/* fprintf(stderr, "X* Return found\n"); */
			break;
		}
		/* See if we have a return instruction */
		if (opa->opcodes[position].opcode == ZEND_RETURN) {
			/*(fprintf(stderr, "XDEBUG Return found\n");)*/
			break;
		}

		position++;
		/*(fprintf(stderr, "XDEBUG Adding %d\n", position);)*/
		xdebug_set_add(set, position);
	}
}

static void prefill_from_oparray(char *fn, zend_op_array *opa TSRMLS_DC)
{
	char cache_key[256];
	int  cache_key_len;
	void *dummy;
	unsigned int i;
	xdebug_set *set = NULL;

	opa->reserved[XG(reserved_offset)] = 1;

#ifdef ZEND_ENGINE_2
	/* Check for abstract methods and simply return from this function in those
	 * cases. */
#if PHP_VERSION_ID >= 50300
	if (opa->size >= 3 && opa->opcodes[opa->size - 3].opcode == ZEND_RAISE_ABSTRACT_ERROR)
#else
	if (opa->size >= 4 && opa->opcodes[opa->size - 4].opcode == ZEND_RAISE_ABSTRACT_ERROR)
#endif
	{
		return;
	}	
#endif

	/* Run dead code analysis if requested */
	if (XG(code_coverage_dead_code_analysis)) {
		set = xdebug_set_create(opa->size);
		xdebug_analyse_branch(opa, 0, set);
	}

	/* The normal loop then finally */
	for (i = 0; i < opa->size; i++) {
		zend_op opcode = opa->opcodes[i];
		prefill_from_opcode(fn, opcode, set ? !xdebug_set_in(set, i) : 0 TSRMLS_CC);
	}

	if (set) {
		xdebug_set_free(set);
	}
}

static int prefill_from_function_table(zend_op_array *opa, int num_args, va_list args, zend_hash_key *hash_key)
{
	char *new_filename;
	TSRMLS_FETCH();

	new_filename = va_arg(args, char*);
	if (opa->type == ZEND_USER_FUNCTION) {
		if (opa->reserved[XG(reserved_offset)] != 1/* && opa->filename && strcmp(opa->filename, new_filename) == 0)*/) {
			prefill_from_oparray(opa->filename, opa TSRMLS_CC);
		}
	}

	return ZEND_HASH_APPLY_KEEP;
}

#ifdef ZEND_ENGINE_2
static int prefill_from_class_table(zend_class_entry **class_entry, int num_args, va_list args, zend_hash_key *hash_key)
#else
static int prefill_from_class_table(zend_class_entry *class_entry, int num_args, va_list args, zend_hash_key *hash_key)
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
#if PHP_MAJOR_VERSION >= 5
		if (!(ce->ce_flags & ZEND_XDEBUG_VISITED)) {
			ce->ce_flags |= ZEND_XDEBUG_VISITED;
			zend_hash_apply_with_arguments(&ce->function_table, (apply_func_args_t) prefill_from_function_table, 1, new_filename);
		}
#else
		zend_hash_apply_with_arguments(&ce->function_table, (apply_func_args_t) prefill_from_function_table, 1, new_filename);
#endif
	}

	return ZEND_HASH_APPLY_KEEP;
}

void xdebug_prefill_code_coverage(zend_op_array *op_array TSRMLS_DC)
{
	if (op_array->reserved[XG(reserved_offset)] != 1) {
		prefill_from_oparray(op_array->filename, op_array TSRMLS_CC);
	}

	zend_hash_apply_with_arguments(CG(function_table), (apply_func_args_t) prefill_from_function_table, 1, op_array->filename);
	zend_hash_apply_with_arguments(CG(class_table), (apply_func_args_t) prefill_from_class_table, 1, op_array->filename);
}

PHP_FUNCTION(xdebug_start_code_coverage)
{
	long options = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &options) == FAILURE) {
		return;
	}
	XG(code_coverage_unused) = (options & XDEBUG_CC_OPTION_UNUSED);
	XG(code_coverage_dead_code_analysis) = (options & XDEBUG_CC_OPTION_DEAD_CODE);

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
		add_index_long(retval, line->lineno, -line->executable);
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
