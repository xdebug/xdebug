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
 */
#ifndef __XDEBUG_TRACING_PRIVATE_H__
#define __XDEBUG_TRACING_PRIVATE_H__

#include "tracing.h"

#define XG_TRACE(v)    (XG(globals.tracing.v))
#define XINI_TRACE(v)  (XG(settings.tracing.v))

#define XDEBUG_OPCODE_OVERRIDE_ASSIGN(f,o) \
	int xdebug_##f##_handler(zend_execute_data *execute_data) \
	{ \
		return xdebug_common_assign_dim_handler((o), execute_data); \
	}

#define XDEBUG_OPCODE_OVERRIDE_ASSIGN_OP(f) \
	int xdebug_##f##_handler(zend_execute_data *execute_data) \
	{ \
		const char *op = get_assign_operation(execute_data->opline->extended_value); \
		return xdebug_common_assign_dim_handler(op, execute_data); \
	}

int xdebug_assign_handler(zend_execute_data *execute_data);
int xdebug_qm_assign_handler(zend_execute_data *execute_data);
int xdebug_assign_op_handler(zend_execute_data *execute_data);
int xdebug_assign_dim_op_handler(zend_execute_data *execute_data);
int xdebug_assign_obj_op_handler(zend_execute_data *execute_data);
int xdebug_assign_static_prop_op_handler(zend_execute_data *execute_data);
int xdebug_pre_inc_handler(zend_execute_data *execute_data);
int xdebug_post_inc_handler(zend_execute_data *execute_data);
int xdebug_pre_dec_handler(zend_execute_data *execute_data);
int xdebug_post_dec_handler(zend_execute_data *execute_data);
int xdebug_pre_inc_obj_handler(zend_execute_data *execute_data);
int xdebug_post_inc_obj_handler(zend_execute_data *execute_data);
int xdebug_pre_dec_obj_handler(zend_execute_data *execute_data);
int xdebug_post_dec_obj_handler(zend_execute_data *execute_data);
int xdebug_assign_concat_handler(zend_execute_data *execute_data);
int xdebug_assign_dim_handler(zend_execute_data *execute_data);
int xdebug_assign_obj_handler(zend_execute_data *execute_data);
int xdebug_assign_ref_handler(zend_execute_data *execute_data);
int xdebug_assign_obj_ref_handler(zend_execute_data *execute_data);
int xdebug_assign_static_prop_handler(zend_execute_data *execute_data);
int xdebug_assign_static_prop_ref_handler(zend_execute_data *execute_data);
int xdebug_pre_inc_static_prop_handler(zend_execute_data *execute_data);
int xdebug_pre_dec_static_prop_handler(zend_execute_data *execute_data);
int xdebug_post_inc_static_prop_handler(zend_execute_data *execute_data);
int xdebug_post_dec_static_prop_handler(zend_execute_data *execute_data);

xdebug_file *xdebug_trace_open_file(char *fname, zend_string *script_filename, long options);

#endif
