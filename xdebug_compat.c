/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2018 Derick Rethans <derick@xdebug.org>           |
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

#include "zend_compile.h"
#include "ext/standard/base64.h"
#include "ext/standard/php_string.h"

zval *xdebug_zval_ptr(int op_type, const znode_op *node, zend_execute_data *zdata TSRMLS_DC)
{
	zend_free_op should_free;

	return zend_get_zval_ptr(op_type, node, zdata, &should_free, BP_VAR_R);
}

char *xdebug_str_to_str(char *haystack, size_t length, const char *needle, size_t needle_len, const char *str, size_t str_len, size_t *new_len)
{
	zend_string *new_str;
	char *retval;

	new_str = php_str_to_str(haystack, length, (char*) needle, needle_len, (char*) str, str_len);
	*new_len = new_str->len;

	retval = estrndup(new_str->val, new_str->len);

	zend_string_release(new_str);

	return retval;
}

char *xdebug_base64_encode(unsigned char *data, int data_len, int *new_len)
{
	zend_string *new_str;
	char *retval;

	new_str = php_base64_encode(data, data_len);
	*new_len = new_str->len;

	retval = estrndup(new_str->val, new_str->len);

	zend_string_release(new_str);

	return retval;
}

unsigned char *xdebug_base64_decode(unsigned char *data, int data_len, int *new_len)
{
	zend_string *new_str;
	char *retval;

	new_str = php_base64_decode(data, data_len);
	*new_len = new_str->len;

	retval = estrndup(new_str->val, new_str->len);

	zend_string_release(new_str);

	return (unsigned char*) retval;
}

void xdebug_stripcslashes(char *str, int *len)
{
	char *source, *target, *end;
	int  nlen = *len, i;
	char numtmp[4];

	for (source=str, end=str+nlen, target=str; source < end; source++) {
		if (*source == '\\' && source+1 < end) {
			source++;
			switch (*source) {
				case 'n':  *target++='\n'; nlen--; break;
				case 'r':  *target++='\r'; nlen--; break;
				case 'a':  *target++='\a'; nlen--; break;
				case 't':  *target++='\t'; nlen--; break;
				case 'v':  *target++='\v'; nlen--; break;
				case 'b':  *target++='\b'; nlen--; break;
				case 'f':  *target++='\f'; nlen--; break;
				case '\\': *target++='\\'; nlen--; break;
				case 'x':
					if (source+1 < end && isxdigit((int)(*(source+1)))) {
						numtmp[0] = *++source;
						if (source+1 < end && isxdigit((int)(*(source+1)))) {
							numtmp[1] = *++source;
							numtmp[2] = '\0';
							nlen-=3;
						} else {
							numtmp[1] = '\0';
							nlen-=2;
						}
						*target++=(char)strtol(numtmp, NULL, 16);
						break;
					}
					XDEBUG_BREAK_INTENTIONALLY_MISSING

				default:
					i=0;
					while (source < end && *source >= '0' && *source <= '7' && i<3) {
						numtmp[i++] = *source++;
					}
					if (i) {
						numtmp[i]='\0';
						*target++=(char)strtol(numtmp, NULL, 8);
						nlen-=i;
						source--;
					} else {
						*target++=*source;
						nlen--;
					}
			}
		} else {
			*target++=*source;
		}
	}

	if (nlen != 0) {
		*target='\0';
	}

	*len = nlen;
}

zend_class_entry *xdebug_fetch_class(char *classname, int classname_len, int flags TSRMLS_DC)
{
	zend_class_entry *tmp_ce;
	zend_string *classname_str = zend_string_init(classname, classname_len, 0);

	tmp_ce = zend_fetch_class(classname_str, flags TSRMLS_CC);
	zend_string_release(classname_str);

	return tmp_ce;
}

int xdebug_get_constant(xdebug_str *val, zval *const_val TSRMLS_DC)
{
	zval *tmp_const = NULL;
	tmp_const = zend_get_constant_str(val->d, val->l);

	if (tmp_const) {
		*const_val = *tmp_const;
	}

	return tmp_const != NULL;
}

void xdebug_setcookie(const char *name, int name_len, char *value, int value_len, time_t expires, const char *path, int path_len, const char *domain, int domain_len, int secure, int url_encode, int httponly TSRMLS_CC)
{
	zend_string *name_s   = zend_string_init(name, name_len, 0);
	zend_string *value_s  = zend_string_init(value, value_len, 0);
	zend_string *path_s   = zend_string_init(path, path_len, 0);
	zend_string *domain_s = zend_string_init(domain, domain_len, 0);
	php_setcookie(name_s, value_s, expires, path_s, domain_s, secure, url_encode, httponly);
	zend_string_release(name_s);
	zend_string_release(value_s);
	zend_string_release(path_s);
	zend_string_release(domain_s);
}

char *xdebug_get_compiled_variable_name(zend_op_array *op_array, uint32_t var, int *cv_len)
{
	zend_string *cv = NULL;
	cv = zend_get_compiled_variable_name(op_array, var);
	*cv_len = cv->len;

	return cv->val;
}

zval *xdebug_read_property(zend_class_entry *ce, zval *exception, const char *name, int length, int flags TSRMLS_DC)
{
	zval dummy;

	return zend_read_property(ce, exception, name, length, flags, &dummy);
}
