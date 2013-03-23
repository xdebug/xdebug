/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2013 Derick Rethans <derick@xdebug.org>           |
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
/* $Id: xdebug_compat.c,v 1.13 2010-05-07 20:39:13 derick Exp $ */

#include "php.h"
#include "main/php_version.h"
#include "xdebug_compat.h"
#include "zend_extensions.h"

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 1
void *php_zend_memrchr(const void *s, int c, size_t n)
{
	register unsigned char *e;

	if (n <= 0) {
		return NULL;
	}

	for (e = (unsigned char *)s + n - 1; e >= (unsigned char *)s; e--) {
		if (*e == (unsigned char)c) {
			return (void *)e;
		}
	}

	return NULL;
}
#endif

#if PHP_VERSION_ID >= 50500
# define T(offset) (*EX_TMP_VAR(zdata, offset))
#else
# define T(offset) (*(temp_variable *)((char*)zdata->Ts + offset))
#endif

zval *xdebug_zval_ptr(int op_type, XDEBUG_ZNODE *node, zend_execute_data *zdata TSRMLS_DC)
{
	switch (op_type & 0x0F) {
		case IS_CONST:
#if PHP_VERSION_ID >= 50399
			return node->zv;
#else
			return &node->u.constant;
#endif
			break;
		case IS_TMP_VAR:
			return &T(XDEBUG_ZNODEP_ELEM(node, var)).tmp_var;
			break;
		case IS_VAR:
			if (T(XDEBUG_ZNODEP_ELEM(node, var)).var.ptr) {
				return T(XDEBUG_ZNODEP_ELEM(node, var)).var.ptr;
			} else {
				temp_variable *T = &T(XDEBUG_ZNODEP_ELEM(node, var));
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
