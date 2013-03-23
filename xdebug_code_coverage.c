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
   | Authors: Derick Rethans <derick@xdebug.org>                          |
   +----------------------------------------------------------------------+
 */

#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_set.h"
#include "xdebug_var.h"
#include "xdebug_code_coverage.h"
#include "xdebug_compat.h"
#include "xdebug_tracing.h"

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

#define XDEBUG_OPCODE_OVERRIDE(f) \
	int xdebug_##f##_handler(ZEND_OPCODE_HANDLER_ARGS) \
	{ \
		return xdebug_common_override_handler(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU); \
	}


int xdebug_common_override_handler(ZEND_OPCODE_HANDLER_ARGS)
{
	if (XG(do_code_coverage)) {
		zend_op *cur_opcode;
		int      lineno;
		char    *file;

		zend_op_array *op_array = execute_data->op_array;

		cur_opcode = *EG(opline_ptr);
		lineno = cur_opcode->lineno;

		file = (char *)op_array->filename;

		xdebug_count_line(file, lineno, 0, 0 TSRMLS_CC);
	}
	return ZEND_USER_OPCODE_DISPATCH;
}

static char *xdebug_find_var_name(zend_execute_data *execute_data TSRMLS_DC)
{
	zend_op       *cur_opcode, *next_opcode, *prev_opcode = NULL, *opcode_ptr;
	zval          *dimval;
	int            is_var, cv_len;
	zend_op_array *op_array = execute_data->op_array;
	xdebug_str     name = {0, 0, NULL};
	int            gohungfound = 0, is_static = 0;
	char          *zval_value = NULL;
	xdebug_var_export_options *options;

	cur_opcode = *EG(opline_ptr);
	next_opcode = cur_opcode + 1;
	prev_opcode = cur_opcode - 1;

	if (cur_opcode->XDEBUG_TYPE(op1) == IS_VAR &&
			(next_opcode->XDEBUG_TYPE(op1) == IS_VAR || cur_opcode->XDEBUG_TYPE(op2) == IS_VAR) &&
			prev_opcode->opcode == ZEND_FETCH_RW &&
			prev_opcode->XDEBUG_TYPE(op1) == IS_CONST &&
#if PHP_VERSION_ID >= 50399
			Z_TYPE_P(prev_opcode->op1.zv) == IS_STRING
#else
			prev_opcode->op1.u.constant.type == IS_STRING
#endif
	) {
#if PHP_VERSION_ID >= 50399
		xdebug_str_add(&name, xdebug_sprintf("$%s", Z_STRVAL_P(prev_opcode->op1.zv)), 1);
#else
		xdebug_str_add(&name, xdebug_sprintf("$%s", prev_opcode->op1.u.constant.value.str.val), 1);
#endif
	}

	is_static = (prev_opcode->XDEBUG_TYPE(op1) == IS_CONST && prev_opcode->XDEBUG_EXTENDED_VALUE(op2) == ZEND_FETCH_STATIC_MEMBER);
	options = xdebug_var_export_options_from_ini(TSRMLS_C);
	options->no_decoration = 1;

	if (cur_opcode->XDEBUG_TYPE(op1) == IS_CV) {
		xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, cur_opcode->XDEBUG_ZNODE_ELEM(op1, var), &cv_len)), 1);
	} else if (cur_opcode->XDEBUG_TYPE(op1) == IS_VAR && cur_opcode->opcode == ZEND_ASSIGN && prev_opcode->opcode == ZEND_FETCH_W) {
		if (is_static) {
			xdebug_str_add(&name, xdebug_sprintf("self::"), 1);
		} else {
			zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, prev_opcode->XDEBUG_TYPE(op1), &prev_opcode->op1, &is_var), 0, options);
			xdebug_str_add(&name, xdebug_sprintf("$%s", zval_value), 1);
		}
	} else if (is_static) { /* todo : see if you can change this and the previous cases around */
		xdebug_str_add(&name, xdebug_sprintf("self::"), 1 );
	}
	if (cur_opcode->opcode >= ZEND_ASSIGN_ADD && cur_opcode->opcode <= ZEND_ASSIGN_BW_XOR ) {
		if (cur_opcode->extended_value == ZEND_ASSIGN_OBJ) {
			zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, cur_opcode->XDEBUG_TYPE(op2), &cur_opcode->op2, &is_var), 0, options);
			if (cur_opcode->XDEBUG_TYPE(op1) == IS_UNUSED) {
				xdebug_str_add(&name, xdebug_sprintf("$this->%s", zval_value), 1);
			} else {
				xdebug_str_add(&name, xdebug_sprintf("->%s", zval_value), 1);
			}
		} else if (cur_opcode->extended_value == ZEND_ASSIGN_DIM) {
			zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, cur_opcode->XDEBUG_TYPE(op2), &cur_opcode->op2, &is_var), 0, NULL);
			xdebug_str_add(&name,xdebug_sprintf("[%s]", zval_value), 1);
		}
	}
	if (cur_opcode->opcode >= ZEND_PRE_INC_OBJ && cur_opcode->opcode <= ZEND_POST_DEC_OBJ) {
		zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, cur_opcode->XDEBUG_TYPE(op2), &cur_opcode->op2, &is_var), 0, options);
		xdebug_str_add(&name, xdebug_sprintf("$this->%s", zval_value), 1);
	}
	if (zval_value) {
		xdfree(zval_value);
		zval_value = NULL;
	}

	/* Scroll back to start of FETCHES */
	gohungfound = 0;
	opcode_ptr = prev_opcode;
	while (opcode_ptr->opcode == ZEND_FETCH_DIM_W || opcode_ptr->opcode == ZEND_FETCH_OBJ_W || opcode_ptr->opcode == ZEND_FETCH_W || opcode_ptr->opcode == ZEND_FETCH_RW) {
		opcode_ptr = opcode_ptr - 1;
		gohungfound = 1;
	}
	opcode_ptr = opcode_ptr + 1;

	if (gohungfound) {
		do
		{
			if (opcode_ptr->XDEBUG_TYPE(op1) == IS_UNUSED && opcode_ptr->opcode == ZEND_FETCH_OBJ_W) {
				xdebug_str_add(&name, "$this", 0);
			}
			if (opcode_ptr->XDEBUG_TYPE(op1) == IS_CV) {
				xdebug_str_add(&name, xdebug_sprintf("$%s", zend_get_compiled_variable_name(op_array, opcode_ptr->XDEBUG_ZNODE_ELEM(op1, var), &cv_len)), 1);
			}
			if (opcode_ptr->opcode == ZEND_FETCH_W) {
				zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->XDEBUG_TYPE(op1), &opcode_ptr->op1, &is_var), 0, options);
				xdebug_str_add(&name, xdebug_sprintf("%s", zval_value), 1);
			}
			if (is_static && opcode_ptr->opcode == ZEND_FETCH_RW) {
				zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->XDEBUG_TYPE(op1), &opcode_ptr->op1, &is_var), 0, options);
				xdebug_str_add(&name, xdebug_sprintf("%s", zval_value), 1);
			}
			if (opcode_ptr->opcode == ZEND_FETCH_DIM_W) {
				if (opcode_ptr->XDEBUG_TYPE(op2) != IS_VAR) {
					zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->XDEBUG_TYPE(op2), &opcode_ptr->op2, &is_var), 0, NULL);
					xdebug_str_add(&name, xdebug_sprintf("[%s]", zval_value), 1);
				} else {
					xdebug_str_add(&name, xdebug_sprintf("[???]") , 1);
				}
			} else if (opcode_ptr->opcode == ZEND_FETCH_OBJ_W) {
				zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->XDEBUG_TYPE(op2), &opcode_ptr->op2, &is_var), 0, options);
				xdebug_str_add(&name, xdebug_sprintf("->%s", zval_value), 1);
			}
			opcode_ptr = opcode_ptr + 1;
			if (zval_value) {
				xdfree(zval_value);
				zval_value = NULL;
			}
		} while (opcode_ptr->opcode == ZEND_FETCH_DIM_W || opcode_ptr->opcode == ZEND_FETCH_OBJ_W || opcode_ptr->opcode == ZEND_FETCH_W || opcode_ptr->opcode == ZEND_FETCH_RW);
	}

	if (cur_opcode->opcode == ZEND_ASSIGN_OBJ) {
		if (cur_opcode->XDEBUG_TYPE(op1) == IS_UNUSED) {
			xdebug_str_add(&name, "$this", 0);
		}
		dimval = xdebug_get_zval(execute_data, cur_opcode->XDEBUG_TYPE(op2), &cur_opcode->op2, &is_var);
		xdebug_str_add(&name, xdebug_sprintf("->%s", Z_STRVAL_P(dimval)), 1);
	}

	if (cur_opcode->opcode == ZEND_ASSIGN_DIM) {
		if (next_opcode->opcode == ZEND_OP_DATA && cur_opcode->XDEBUG_TYPE(op2) == IS_UNUSED) {
			xdebug_str_add(&name, "[]", 0);
		} else {
			zval_value = xdebug_get_zval_value(xdebug_get_zval(execute_data, opcode_ptr->XDEBUG_TYPE(op2), &opcode_ptr->op2, &is_var), 0, NULL);
			xdebug_str_add(&name, xdebug_sprintf("[%s]", zval_value), 1);
			xdfree(zval_value);
		}
	}

	xdfree(options->runtime);
	xdfree(options);

	return name.d;
}

static int xdebug_common_assign_dim_handler(char *op, int do_cc, ZEND_OPCODE_HANDLER_ARGS)
{
	char    *file;
	zend_op_array *op_array = execute_data->op_array;
	int            lineno;
	zend_op       *cur_opcode, *next_opcode;
	char          *full_varname;
	zval          *val = NULL;
	char          *t;
	int            is_var;
	function_stack_entry *fse;

	cur_opcode = *EG(opline_ptr);
	next_opcode = cur_opcode + 1;
	file = (char *) op_array->filename;
	lineno = cur_opcode->lineno;

	if (do_cc && XG(do_code_coverage)) {
		xdebug_count_line(file, lineno, 0, 0 TSRMLS_CC);
	}
	if (XG(do_trace) && XG(trace_file) && XG(collect_assignments)) {
		full_varname = xdebug_find_var_name(execute_data TSRMLS_CC);

		if (cur_opcode->opcode >= ZEND_PRE_INC && cur_opcode->opcode <= ZEND_POST_DEC) {
			char *tmp_varname;

			switch (cur_opcode->opcode) {
				case ZEND_PRE_INC:  tmp_varname = xdebug_sprintf("++%s", full_varname); break;
				case ZEND_POST_INC: tmp_varname = xdebug_sprintf("%s++", full_varname); break;
				case ZEND_PRE_DEC:  tmp_varname = xdebug_sprintf("--%s", full_varname); break;
				case ZEND_POST_DEC: tmp_varname = xdebug_sprintf("%s--", full_varname); break;
			}
			xdfree(full_varname);
			full_varname = tmp_varname;

			val = xdebug_get_zval(execute_data, cur_opcode->XDEBUG_TYPE(op1), &cur_opcode->op1, &is_var);
		} else if (cur_opcode->opcode >= ZEND_PRE_INC_OBJ && cur_opcode->opcode <= ZEND_POST_DEC_OBJ) {
			char *tmp_varname;

			switch (cur_opcode->opcode) {
				case ZEND_PRE_INC_OBJ:  tmp_varname = xdebug_sprintf("++%s", full_varname); break;
				case ZEND_POST_INC_OBJ: tmp_varname = xdebug_sprintf("%s++", full_varname); break;
				case ZEND_PRE_DEC_OBJ:  tmp_varname = xdebug_sprintf("--%s", full_varname); break;
				case ZEND_POST_DEC_OBJ: tmp_varname = xdebug_sprintf("%s--", full_varname); break;
			}
			xdfree(full_varname);
			full_varname = tmp_varname;

			val = xdebug_get_zval(execute_data, cur_opcode->XDEBUG_TYPE(op2), &cur_opcode->op2, &is_var);
		} else if (next_opcode->opcode == ZEND_OP_DATA) {
			val = xdebug_get_zval(execute_data, next_opcode->XDEBUG_TYPE(op1), &next_opcode->op1, &is_var);
		} else {
			val = xdebug_get_zval(execute_data, cur_opcode->XDEBUG_TYPE(op2), &cur_opcode->op2, &is_var);
		}

		fse = XDEBUG_LLIST_VALP(XDEBUG_LLIST_TAIL(XG(stack)));
		t = xdebug_return_trace_assignment(fse, full_varname, val, op, file, lineno TSRMLS_CC);
		xdfree(full_varname);
		fprintf(XG(trace_file), "%s", t);
		fflush(XG(trace_file));
		xdfree(t);
	}
	return ZEND_USER_OPCODE_DISPATCH;
}

#define XDEBUG_OPCODE_OVERRIDE_ASSIGN(f,o,cc) \
	int xdebug_##f##_handler(ZEND_OPCODE_HANDLER_ARGS) \
	{ \
		return xdebug_common_assign_dim_handler((o), (cc), ZEND_OPCODE_HANDLER_ARGS_PASSTHRU); \
	}

XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign,"=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_add,"+=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_sub,"-=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_mul,"*=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_div,"/=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_mod,"%=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_sl,"<<=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_sr,">>=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_inc,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_inc,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_dec,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_dec,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_inc_obj,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_inc_obj,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(pre_dec_obj,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(post_dec_obj,"",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_concat,".=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_bw_or,"|=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_bw_and,"&=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_bw_xor,"^=",0)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_dim,"=",1)
XDEBUG_OPCODE_OVERRIDE_ASSIGN(assign_obj,"=",1)

void xdebug_count_line(char *filename, int lineno, int executable, int deadcode TSRMLS_DC)
{
	xdebug_coverage_file *file;
	xdebug_coverage_line *line;

	if (strcmp(XG(previous_filename), filename) == 0) {
		file = XG(previous_file);
	} else {
		/* Check if the file already exists in the hash */
		if (!xdebug_hash_find(XG(code_coverage), filename, strlen(filename), (void *) &file)) {
			/* The file does not exist, so we add it to the hash, and
			 *  add a line element to the file */
			file = xdmalloc(sizeof(xdebug_coverage_file));
			file->name = xdstrdup(filename);
			file->lines = xdebug_hash_alloc(128, xdebug_coverage_line_dtor);
		
			xdebug_hash_add(XG(code_coverage), filename, strlen(filename), file);
		}
		XG(previous_filename) = file->name;
		XG(previous_file) = file;
	}

	/* Check if the line already exists in the hash */
	if (!xdebug_hash_index_find(file->lines, lineno, (void *) &line)) {
		line = xdmalloc(sizeof(xdebug_coverage_line));
		line->lineno = lineno;
		line->count = 0;
		line->executable = 0;

		xdebug_hash_index_add(file->lines, lineno, line);
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
}

static void prefill_from_opcode(char *fn, zend_op opcode, int deadcode TSRMLS_DC)
{
	if (
		opcode.opcode != ZEND_NOP &&
		opcode.opcode != ZEND_EXT_NOP &&
		opcode.opcode != ZEND_RECV &&
		opcode.opcode != ZEND_RECV_INIT
		&& opcode.opcode != ZEND_VERIFY_ABSTRACT_CLASS
		&& opcode.opcode != ZEND_OP_DATA
		&& opcode.opcode != ZEND_ADD_INTERFACE
		&& opcode.opcode != ZEND_TICKS
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
		if (array_offset == -1) {
			/* broken break/continue in code */
			return NULL;
		}
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
		*jmp1 = ((long) opcode.XDEBUG_ZNODE_ELEM(op1, jmp_addr) - (long) base_address) / sizeof(zend_op);
		return 1;
	} else if (
		opcode.opcode == ZEND_JMPZ ||
		opcode.opcode == ZEND_JMPNZ ||
		opcode.opcode == ZEND_JMPZ_EX ||
		opcode.opcode == ZEND_JMPNZ_EX
	) {
		*jmp1 = position + 1;
		*jmp2 = ((long) opcode.XDEBUG_ZNODE_ELEM(op2, jmp_addr) - (long) base_address) / sizeof(zend_op);
		return 1;
	} else if (opcode.opcode == ZEND_JMPZNZ) {
		*jmp1 = opcode.XDEBUG_ZNODE_ELEM(op2, opline_num);
		*jmp2 = opcode.extended_value;
		return 1;
	} else if (opcode.opcode == ZEND_BRK || opcode.opcode == ZEND_CONT) {
		zend_brk_cont_element *el;

		if (opcode.XDEBUG_TYPE(op2) == IS_CONST
		    && opcode.XDEBUG_ZNODE_ELEM(op1, jmp_addr) != (zend_op*) 0xFFFFFFFF
		) {
#if PHP_VERSION_ID >= 50399
			el = xdebug_find_brk_cont(opcode.op2.zv, opcode.XDEBUG_ZNODE_ELEM(op1, opline_num), opa);
#else
			el = xdebug_find_brk_cont(&opcode.op2.u.constant, opcode.op1.u.opline_num, opa);
#endif
			if (el) {
				*jmp1 = opcode.opcode == ZEND_BRK ? el->brk : el->cont;
				return 1;
			} else {
				/* broken break/continue in code */
				return 0;
			}
		}
	} else if (opcode.opcode == ZEND_FE_RESET || opcode.opcode == ZEND_FE_FETCH) {
		*jmp1 = position + 1;
		*jmp2 = opcode.XDEBUG_ZNODE_ELEM(op2, opline_num);
		return 1;
#if (PHP_MAJOR_VERSION > 5) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3)
	} else if (opcode.opcode == ZEND_GOTO) {
		*jmp1 = ((long) opcode.XDEBUG_ZNODE_ELEM(op1, jmp_addr) - (long) base_address) / sizeof(zend_op);
		return 1;
#endif
	}
	return 0;
}

static void xdebug_analyse_branch(zend_op_array *opa, unsigned int position, xdebug_set *set TSRMLS_DC)
{
	long jump_pos1;
	long jump_pos2;

	/*(fprintf(stderr, "Branch analysis from position: %d\n", position);)*/
	/* First we see if the branch has been visited, if so we bail out. */
	if (xdebug_set_in(set, position)) {
		return;
	}
	/* Loop over the opcodes until the end of the array, or until a jump point has been found */
	xdebug_set_add(set, position);
	/*(fprintf(stderr, "XDEBUG Adding %d\n", position);)*/
	while (position < opa->last) {
		jump_pos1 = -1;
		jump_pos2 = -1;

		/* See if we have a jump instruction */
		if (xdebug_find_jump(opa, position, &jump_pos1, &jump_pos2)) {
			/*(fprintf(stderr, "XDEBUG Jump found. Position 1 = %d", jump_pos1);)*/
			if (jump_pos2 != -1) {
				/*(fprintf(stderr, ", Position 2 = %d\n", jump_pos2))*/;
			} else {
				/*(fprintf(stderr, "\n"))*/;
			}
			xdebug_analyse_branch(opa, jump_pos1, set TSRMLS_CC);
			if (jump_pos2 != -1 && jump_pos2 <= opa->last) {
				xdebug_analyse_branch(opa, jump_pos2, set TSRMLS_CC);
			}
			break;
		}

		/* See if we have a throw instruction */
		if (opa->opcodes[position].opcode == ZEND_THROW) {
			/* fprintf(stderr, "Throw found at %d\n", position); */
			break;
		}

		/* See if we have an exit instruction */
		if (opa->opcodes[position].opcode == ZEND_EXIT) {
			/* fprintf(stderr, "X* Return found\n"); */
			break;
		}
		/* See if we have a return instruction */
		if (
			opa->opcodes[position].opcode == ZEND_RETURN
#if PHP_VERSION_ID >= 50400
			|| opa->opcodes[position].opcode == ZEND_RETURN_BY_REF
#endif
		) {
			/*(fprintf(stderr, "XDEBUG Return found\n");)*/
			break;
		}

		position++;
		/*(fprintf(stderr, "XDEBUG Adding %d\n", position);)*/
		xdebug_set_add(set, position);
	}
}

static void xdebug_analyse_oparray(zend_op_array *opa, xdebug_set *set TSRMLS_DC)
{
	unsigned int position = 0;

	while (position < opa->last) {
		if (position == 0) {
			xdebug_analyse_branch(opa, position, set TSRMLS_CC);
		} else if (opa->opcodes[position].opcode == ZEND_CATCH) {
			xdebug_analyse_branch(opa, position, set TSRMLS_CC);
		}
		position++;
	}
}

static void prefill_from_oparray(char *fn, zend_op_array *op_array TSRMLS_DC)
{
	unsigned int i;
	xdebug_set *set = NULL;

	op_array->reserved[XG(reserved_offset)] = (void*) 1;

	/* Check for abstract methods and simply return from this function in those
	 * cases. */
#if PHP_VERSION_ID >= 50300
	if (op_array->last >= 3 && op_array->opcodes[op_array->last - 3].opcode == ZEND_RAISE_ABSTRACT_ERROR)
#else
	if (op_array->last >= 4 && op_array->opcodes[op_array->last - 4].opcode == ZEND_RAISE_ABSTRACT_ERROR)
#endif
	{
		return;
	}	

	/* Run dead code analysis if requested */
	if (XG(code_coverage_dead_code_analysis) && XDEBUG_PASS_TWO_DONE) {
		set = xdebug_set_create(op_array->last);
		xdebug_analyse_oparray(op_array, set TSRMLS_CC);
	}

	/* The normal loop then finally */
	for (i = 0; i < op_array->last; i++) {
		zend_op opcode = op_array->opcodes[i];
		prefill_from_opcode(fn, opcode, set ? !xdebug_set_in(set, i) : 0 TSRMLS_CC);
	}

	if (set) {
		xdebug_set_free(set);
	}
}

static int prefill_from_function_table(zend_op_array *opa XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	char *new_filename;
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50300
	TSRMLS_FETCH();
#endif

	new_filename = va_arg(args, char*);
	if (opa->type == ZEND_USER_FUNCTION) {
		if (opa->reserved[XG(reserved_offset)] != (void*) 1 /* && opa->filename && strcmp(opa->filename, new_filename) == 0)*/) {
			prefill_from_oparray((char *) opa->filename, opa TSRMLS_CC);
		}
	}

	return ZEND_HASH_APPLY_KEEP;
}

static int prefill_from_class_table(zend_class_entry **class_entry XDEBUG_ZEND_HASH_APPLY_TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
	char *new_filename;
	zend_class_entry *ce;

	ce = *class_entry;

	new_filename = va_arg(args, char*);
	if (ce->type == ZEND_USER_CLASS) {
		if (!(ce->ce_flags & ZEND_XDEBUG_VISITED)) {
			ce->ce_flags |= ZEND_XDEBUG_VISITED;
			zend_hash_apply_with_arguments(&ce->function_table XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) prefill_from_function_table, 1, new_filename);
		}
	}

	return ZEND_HASH_APPLY_KEEP;
}

void xdebug_prefill_code_coverage(zend_op_array *op_array TSRMLS_DC)
{
	if (op_array->reserved[XG(reserved_offset)] != (void*) 1) {
		prefill_from_oparray((char *) op_array->filename, op_array TSRMLS_CC);
	}

	zend_hash_apply_with_arguments(CG(function_table)  XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) prefill_from_function_table, 1, op_array->filename);
	zend_hash_apply_with_arguments(CG(class_table) XDEBUG_ZEND_HASH_APPLY_TSRMLS_CC, (apply_func_args_t) prefill_from_class_table, 1, op_array->filename);
}

PHP_FUNCTION(xdebug_start_code_coverage)
{
	long options = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &options) == FAILURE) {
		return;
	}
	XG(code_coverage_unused) = (options & XDEBUG_CC_OPTION_UNUSED);
	XG(code_coverage_dead_code_analysis) = (options & XDEBUG_CC_OPTION_DEAD_CODE);

	if (!XG(extended_info)) {
		php_error(E_WARNING, "You can only use code coverage when you leave the setting of 'xdebug.extended_info' to the default '1'.");
		RETURN_FALSE;
	} else if (!XG(code_coverage)) {
		php_error(E_WARNING, "Code coverage needs to be enabled in php.ini by setting 'xdebug.coverage_enable' to '1'.");
		RETURN_FALSE;
	} else {
		XG(do_code_coverage) = 1;
		RETURN_TRUE;
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
			XG(previous_filename) = "";
			XG(previous_file) = NULL;
			xdebug_hash_destroy(XG(code_coverage));
			XG(code_coverage) = xdebug_hash_alloc(32, xdebug_coverage_file_dtor);
		}
		XG(do_code_coverage) = 0;
		RETURN_TRUE;
	}
	RETURN_FALSE;
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
