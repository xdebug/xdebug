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
#ifndef __XDEBUG_TRACING_PRIVATE_H__
#define __XDEBUG_TRACING_PRIVATE_H__

#include "tracing.h"

#define XG_TRACE(v)    (XG(globals.tracing.v))
#define XINI_TRACE(v)  (XG(settings.tracing.v))

#define XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(f) \
	int xdebug_##f##_handler(zend_execute_data *execute_data)

#define XDEBUG_OPCODE_OVERRIDE_ASSIGN(f,o,do_code_coverage) \
	int xdebug_##f##_handler(zend_execute_data *execute_data) \
	{ \
		return xdebug_common_assign_dim_handler((o), (do_code_coverage), execute_data); \
	}

#if PHP_VERSION_ID >= 70400
#define XDEBUG_OPCODE_OVERRIDE_ASSIGN_OP(f,do_code_coverage) \
	int xdebug_##f##_handler(zend_execute_data *execute_data) \
	{ \
		const char *op = get_assign_operation(execute_data->opline->extended_value); \
		return xdebug_common_assign_dim_handler(op, (do_code_coverage), execute_data); \
	}
#endif

XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(qm_assign);
#if PHP_VERSION_ID >= 70400
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_op);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_dim_op);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_obj_op);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_static_prop_op);
#else
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_add);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_sub);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_mul);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_div);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_mod);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_sl);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_sr);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_bw_or);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_bw_and);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_bw_xor);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_pow);
#endif
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(pre_inc);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(post_inc);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(pre_dec);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(post_dec);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(pre_inc_obj);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(post_inc_obj);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(pre_dec_obj);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(post_dec_obj);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_concat);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_dim);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_obj);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_ref);
#if PHP_VERSION_ID >= 70400
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_obj_ref);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_static_prop);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(assign_static_prop_ref);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(pre_inc_static_prop);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(pre_dec_static_prop);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(post_inc_static_prop);
XDEBUG_OPCODE_OVERRIDE_ASSIGN_DECL(post_dec_static_prop);
#endif

FILE *xdebug_trace_open_file(char *fname, char *script_filename, long options, char **used_fname);

#endif
