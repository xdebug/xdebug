/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2022 Derick Rethans <derick@xdebug.org>           |
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

#include "lib/php-header.h"
#include "main/php_version.h"
#include "compat.h"
#include "zend_extensions.h"

#include "zend_compile.h"
#include "ext/standard/base64.h"
#include "ext/standard/php_string.h"

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

/* {{{ base64 tables */
static const char base64_table[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
};

static const char base64_pad = '=';

static const short base64_reverse_table[256] = {
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
	-2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
	-2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
};
/* }}} */

static unsigned char *xdebug_base64_encode_impl(const unsigned char *in, size_t inl, unsigned char *out) /* {{{ */
{

	while (inl > 2) { /* keep going until we have less than 24 bits */
		*out++ = base64_table[in[0] >> 2];
		*out++ = base64_table[((in[0] & 0x03) << 4) + (in[1] >> 4)];
		*out++ = base64_table[((in[1] & 0x0f) << 2) + (in[2] >> 6)];
		*out++ = base64_table[in[2] & 0x3f];

		in += 3;
		inl -= 3; /* we just handle 3 octets of data */
	}

	/* now deal with the tail end of things */
	if (inl != 0) {
		*out++ = base64_table[in[0] >> 2];
		if (inl > 1) {
			*out++ = base64_table[((in[0] & 0x03) << 4) + (in[1] >> 4)];
			*out++ = base64_table[(in[1] & 0x0f) << 2];
			*out++ = base64_pad;
		} else {
			*out++ = base64_table[(in[0] & 0x03) << 4];
			*out++ = base64_pad;
			*out++ = base64_pad;
		}
	}

	*out = '\0';

	return out;
}
/* }}} */

static int xdebug_base64_decode_impl(const unsigned char *in, size_t inl, unsigned char *out, size_t *outl, zend_bool strict) /* {{{ */
{
	int ch;
	size_t i = 0, padding = 0, j = *outl;

	/* run through the whole string, converting as we go */
	while (inl-- > 0) {
		ch = *in++;
		if (ch == base64_pad) {
			padding++;
			continue;
		}

		ch = base64_reverse_table[ch];
		if (!strict) {
			/* skip unknown characters and whitespace */
			if (ch < 0) {
				continue;
			}
		} else {
			/* skip whitespace */
			if (ch == -1) {
				continue;
			}
			/* fail on bad characters or if any data follows padding */
			if (ch == -2 || padding) {
				goto fail;
			}
		}

		switch (i % 4) {
			case 0:
				out[j] = ch << 2;
				break;
			case 1:
				out[j++] |= ch >> 4;
				out[j] = (ch & 0x0f) << 4;
				break;
			case 2:
				out[j++] |= ch >>2;
				out[j] = (ch & 0x03) << 6;
				break;
			case 3:
				out[j++] |= ch;
				break;
		}
		i++;
	}

	/* fail if the input is truncated (only one char in last group) */
	if (strict && i % 4 == 1) {
		goto fail;
	}

	/* fail if the padding length is wrong (not VV==, VVV=), but accept zero padding
	 * RFC 4648: "In some circumstances, the use of padding [--] is not required" */
	if (strict && padding && (padding > 2 || (i + padding) % 4 != 0)) {
		goto fail;
	}

	*outl = j;
	out[j] = '\0';

	return 1;

fail:
	return 0;
}
/* }}} */

unsigned char *xdebug_base64_encode(unsigned char *data, size_t data_len, size_t *new_len)
{
	unsigned char *retval = xdmalloc((((data_len + 2) / 3) + 1) * (4 * sizeof(char)));
	unsigned char *end;

	end = xdebug_base64_encode_impl(data, data_len, retval);
	*new_len = end - retval;

	return retval;
}

unsigned char *xdebug_base64_decode(unsigned char *data, size_t data_len, size_t *new_len)
{
	unsigned char *retval = xdmalloc(data_len + 1);

	xdebug_base64_decode_impl(data, data_len, retval, new_len, 0);

	return retval;
}

zend_string *xdebug_addslashes(zend_string *str)
{
	/* maximum string length, worst case situation */
	char *target;
	const char *source, *end;
	size_t offset;
	zend_string *new_str;

	if (!str) {
		return ZSTR_EMPTY_ALLOC();
	}

	source = ZSTR_VAL(str);
	end = source + ZSTR_LEN(str);

	while (source < end) {
		switch (*source) {
			case '\0':
			case '\'':
			case '\"':
			case '\\':
				goto do_escape;
			default:
				source++;
				break;
		}
	}

	return zend_string_copy(str);

do_escape:
	offset = source - (char *)ZSTR_VAL(str);
	new_str = zend_string_safe_alloc(2, ZSTR_LEN(str) - offset, offset, 0);
	memcpy(ZSTR_VAL(new_str), ZSTR_VAL(str), offset);
	target = ZSTR_VAL(new_str) + offset;

	while (source < end) {
		switch (*source) {
			case '\0':
				*target++ = '\\';
				*target++ = '0';
				break;
			case '\'':
			case '\"':
			case '\\':
				*target++ = '\\';
				XDEBUG_BREAK_INTENTIONALLY_MISSING

			default:
				*target++ = *source;
				break;
		}
		source++;
	}

	*target = '\0';

	if (ZSTR_LEN(new_str) - (target - ZSTR_VAL(new_str)) > 16) {
		new_str = zend_string_truncate(new_str, target - ZSTR_VAL(new_str), 0);
	} else {
		ZSTR_LEN(new_str) = target - ZSTR_VAL(new_str);
	}

	return new_str;
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

zend_ulong xdebug_get_pid(void)
{
#ifndef ZTS
	return (zend_ulong) getpid();
#else
# ifdef _WIN32
	return (zend_ulong) GetCurrentThreadId();
# else
	return (zend_ulong) pthread_self();
# endif
#endif
}

void xdebug_setcookie(const char *name, int name_len, char *value, int value_len, time_t expires, const char *path, int path_len, const char *domain, int domain_len, int secure, int url_encode, int httponly)
{
	zend_string *name_s   = name ? zend_string_init(name, name_len, 0) : NULL;
	zend_string *value_s  = value ? zend_string_init(value, value_len, 0) : NULL;
	zend_string *path_s   = path ? zend_string_init(path, path_len, 0) : NULL;
	zend_string *domain_s = domain ? zend_string_init(domain, domain_len, 0) : NULL;
	zend_string *samesite_s = zend_string_init("Lax", sizeof("Lax") - 1, 0);

	php_setcookie(name_s, value_s, expires, path_s, domain_s, secure, httponly, samesite_s, url_encode);

	zend_string_release(samesite_s);
	name ? zend_string_release(name_s) : 0;
	value ? zend_string_release(value_s) : 0;
	path ? zend_string_release(path_s) : 0;
	domain ? zend_string_release(domain_s) : 0;
}

char *xdebug_get_compiled_variable_name(zend_op_array *op_array, uint32_t var, int *cv_len)
{
	zend_string *cv = NULL;
	cv = zend_get_compiled_variable_name(op_array, var);
	*cv_len = cv->len;

	return cv->val;
}

#ifdef ZEND_HASH_GET_APPLY_COUNT /* PHP 7.2 or earlier recursion protection */
zend_bool xdebug_zend_hash_is_recursive(HashTable* ht)
{
	return (ZEND_HASH_GET_APPLY_COUNT(ht) > 0);
}

zend_bool xdebug_zend_hash_apply_protection_begin(HashTable* ht)
{
	if (!ht) {
		return 1;
	}
	if (ZEND_HASH_GET_APPLY_COUNT(ht) > 0) {
		return 0;
	}
	if (ZEND_HASH_APPLY_PROTECTION(ht)) {
		ZEND_HASH_INC_APPLY_COUNT(ht);
	}
	return 1;
}

zend_bool xdebug_zend_hash_apply_protection_end(HashTable* ht)
{
	if (!ht) {
		return 1;
	}
	if (ZEND_HASH_GET_APPLY_COUNT(ht) == 0) {
		return 0;
	}
	if (ZEND_HASH_APPLY_PROTECTION(ht)) {
		ZEND_HASH_DEC_APPLY_COUNT(ht);
	}
	return 1;
}
#else /* PHP 7.3 or later */
zend_bool xdebug_zend_hash_is_recursive(HashTable* ht)
{
	return GC_IS_RECURSIVE(ht);
}

zend_bool xdebug_zend_hash_apply_protection_begin(zend_array* ht)
{
	if (GC_IS_RECURSIVE(ht)) {
		return 0;
	}
	if (!(GC_FLAGS(ht) & GC_IMMUTABLE)) {
		GC_PROTECT_RECURSION(ht);
	}
	return 1;
}

zend_bool xdebug_zend_hash_apply_protection_end(zend_array* ht)
{
	if (!GC_IS_RECURSIVE(ht)) {
		return 0;
	}
	if (!(GC_FLAGS(ht) & GC_IMMUTABLE)) {
		GC_UNPROTECT_RECURSION(ht);
	}
	return 1;
}
#endif
