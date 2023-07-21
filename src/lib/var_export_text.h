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

#ifndef __HAVE_XDEBUG_LIB_VAR_EXPORT_TEXT_H__
#define __HAVE_XDEBUG_LIB_VAR_EXPORT_TEXT_H__

#include "var.h"

xdebug_str* xdebug_get_zval_value_text_ansi(zval *val, int mode, int debug_zval, xdebug_var_export_options *options);
#define xdebug_get_zval_value_text(v,d,o) xdebug_get_zval_value_text_ansi(v,0,d,o);
#define xdebug_get_zval_value_ansi(v,d,o) xdebug_get_zval_value_text_ansi(v,1,d,o);

xdebug_str* xdebug_get_zval_synopsis_text_ansi(zval *val, int mode, int debug_zval, xdebug_var_export_options *options);

#endif
