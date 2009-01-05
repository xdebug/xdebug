/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2008 Derick Rethans <derick@xdebug.org>           |
   |           (c) 1997-2004 Jim Winstead <jimw@trainedmonkey.com>        |
   |           (c) 1998-2004 Andi Gutmans <andi@zend.com> and             |
   |                         Zeev Suraski <zeev@zend.com>                 |
   +----------------------------------------------------------------------+
   | This source file is subject to the following Modified BSD license:   |
   |                                                                      |
   | Redistribution and use in source and binary forms, with or without   |
   | modification, are permitted provided that the following conditions   |
   | are met:                                                             |
   |                                                                      |
   |   1. Redistributions of source code must retain the above copyright  |
   |      notice, this list of conditions and the following disclaimer.   |
   |                                                                      |
   |   2. Redistributions in binary form must reproduce the above         |
   |      copyright notice, this list of conditions and the following     |
   |      disclaimer in the documentation and/or other materials provided |
   |      with the distribution.                                          |
   |                                                                      |
   |   3. The name of the author may not be used to endorse or promote    |
   |      products derived from this software without specific prior      |
   |      written permission.                                             |
   |                                                                      |
   | THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR |
   | IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED       |
   | WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   |
   | ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY       |
   | DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL   |
   | DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE    |
   | GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS        |
   | INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER |
   | IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR      |
   | OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN  |
   | IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                        |
   +----------------------------------------------------------------------+
 */
/* $Id: xdebug_compat.c,v 1.12 2009-01-05 17:24:34 derick Exp $ */

#include "php.h"
#include "main/php_version.h"
#include "xdebug_compat.h"
#include "zend_extensions.h"

#if PHP_MAJOR_VERSION >= 6
void xdebug_php_var_dump(zval **struc, int level TSRMLS_DC)
{
	php_var_dump(struc, 1, 1 TSRMLS_CC);
}
#endif


#define T(offset) (*(temp_variable *)((char *) Ts + offset))

zval *xdebug_zval_ptr(znode *node, temp_variable *Ts TSRMLS_DC)
{
	switch (node->op_type) {
		case IS_CONST:
			return &node->u.constant;
			break;
		case IS_TMP_VAR:
			return &T(node->u.var).tmp_var;
			break;
		case IS_VAR:
			if (T(node->u.var).var.ptr) {
				return T(node->u.var).var.ptr;
			} else {
				temp_variable *T = &T(node->u.var);
				zval *str = T->str_offset.str;

				if (T->str_offset.str->type != IS_STRING
					|| ((int)T->str_offset.offset<0)
					|| (T->str_offset.str->value.str.len <= T->str_offset.offset)) {
					zend_error(E_NOTICE, "Uninitialized string offset:  %d", T->str_offset.offset);
					T->tmp_var.value.str.val = STR_EMPTY_ALLOC();
					T->tmp_var.value.str.len = 0;
				} else {
					char c = str->value.str.val[T->str_offset.offset];

					T->tmp_var.value.str.val = estrndup(&c, 1);
					T->tmp_var.value.str.len = 1;
				}
				T->tmp_var.XDEBUG_REFCOUNT=1;
				T->tmp_var.XDEBUG_IS_REF=1;
				T->tmp_var.type = IS_STRING;
				return &T->tmp_var;
			}
			break;
		case IS_UNUSED:
			return NULL;
			break;
	}
	return NULL;
}
