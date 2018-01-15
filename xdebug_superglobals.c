/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2018 Derick Rethans                               |
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
   |          Harald Radi <harald.radi@nme.at>                            |
   +----------------------------------------------------------------------+
 */

#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_var.h"
#include "xdebug_compat.h"
#include "xdebug_superglobals.h"
#include "SAPI.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

void xdebug_superglobals_dump_dtor(void *user, void *ptr)
{
	free(ptr);
}

static void dump_hash_elem(zval *z, const char *name, long index_key, const char *elem, int html, xdebug_str *str TSRMLS_DC)
{
	if (html) {
		if (elem) {
			xdebug_str_add(str, xdebug_sprintf("<tr><td colspan='2' align='right' bgcolor='#eeeeec' valign='top'><pre>$%s['%s']&nbsp;=</pre></td>", name, elem), 1);
		} else {
			xdebug_str_add(str, xdebug_sprintf("<tr><td colspan='2' align='right' bgcolor='#eeeeec' valign='top'><pre>$%s[%ld]&nbsp;=</pre></td>", name, index_key), 1);
		}
	}

	if (z != NULL) {
		xdebug_str *val;

		if (html) {
			val = xdebug_get_zval_value_fancy(NULL, z, 0, NULL);

			xdebug_str_addl(str, "<td colspan='3' bgcolor='#eeeeec'>", 34, 0);
			xdebug_str_add_str(str, val);
			xdebug_str_addl(str, "</td>", 5, 0);
		} else {
			val = xdebug_get_zval_value(z, 0, NULL);

			xdebug_str_add(str, xdebug_sprintf("\n   $%s['%s'] = ", name, elem), 1);
			xdebug_str_add_str(str, val);
		}

		xdebug_str_free(val);
	} else {
		/* not found */
		if (html) {
			xdebug_str_add(str, "<td colspan='3' bgcolor='#eeeeec'><i>undefined</i></td>", 0);
		} else {
			xdebug_str_add(str, xdebug_sprintf("\n   $%s['%s'] is undefined", name, elem), 1);
		}
	}

	if (html) {
		xdebug_str_add(str, "</tr>\n", 0);
	}
}

static int dump_hash_elem_va(zval *pDest, zend_ulong index_key, zend_string *hash_key, const char *name, int html, xdebug_str *str)
{
	if (HASH_KEY_IS_NUMERIC(hash_key)) {
		dump_hash_elem(*((zval **) pDest), name, hash_key->h, NULL, html, str TSRMLS_CC);
	} else {
		dump_hash_elem(pDest, name, 0, HASH_APPLY_KEY_VAL(hash_key), html, str TSRMLS_CC);
	}

	return SUCCESS;
}

static void dump_hash(xdebug_llist *l, const char *name, int name_len, int html, xdebug_str *str TSRMLS_DC)
{
	zval *z;
	zend_ulong num;
	zend_string *key;
	zval *val;
	HashTable *ht = NULL;
	xdebug_llist_element *elem;

	if (!XDEBUG_LLIST_COUNT(l)) {
		return;
	}

	{
		zend_string *s_name = zend_string_init(name, name_len, 0);
		if ((z = zend_hash_find(&EG(symbol_table), s_name))) {
			if (Z_TYPE_P(z) == IS_REFERENCE) {
				z = &z->value.ref->val;
			}
			if (Z_TYPE_P(z) == IS_ARRAY) {
				ht = Z_ARRVAL_P(z);
			}
		}
		zend_string_release(s_name);
	}

	if (html) {
		xdebug_str_add(str, xdebug_sprintf("<tr><th colspan='5' align='left' bgcolor='#e9b96e'>Dump <i>$%s</i></th></tr>\n", name), 1);
	} else {
		xdebug_str_add(str, xdebug_sprintf("\nDump $%s", name), 1);
	}

	elem = XDEBUG_LLIST_HEAD(l);

	while (elem != NULL) {
		zend_string *s;

		s = zend_string_init(elem->ptr, strlen(elem->ptr), 0);

		if (ht && (*((char *) (elem->ptr)) == '*')) {
			ZEND_HASH_FOREACH_KEY_VAL_IND(ht, num, key, val) {
				dump_hash_elem_va(val, num, key, name, html, str);
			} ZEND_HASH_FOREACH_END();
		} else if (ht && (z = zend_hash_find(ht, s))) {
			dump_hash_elem(z, name, 0, elem->ptr, html, str TSRMLS_CC);
		} else if(XG(dump_undefined)) {
			dump_hash_elem(NULL, name, 0, elem->ptr, html, str TSRMLS_CC);
		}

		elem = XDEBUG_LLIST_NEXT(elem);

		zend_string_release(s);
	}
}

char* xdebug_get_printable_superglobals(int html TSRMLS_DC)
{
	xdebug_str str = XDEBUG_STR_INITIALIZER;

	dump_hash(&XG(server),  "_SERVER",  HASH_KEY_SIZEOF("_SERVER"),  html, &str TSRMLS_CC);
	dump_hash(&XG(get),     "_GET",     HASH_KEY_SIZEOF("_GET"),     html, &str TSRMLS_CC);
	dump_hash(&XG(post),    "_POST",    HASH_KEY_SIZEOF("_POST"),    html, &str TSRMLS_CC);
	dump_hash(&XG(cookie),  "_COOKIE",  HASH_KEY_SIZEOF("_COOKIE"),  html, &str TSRMLS_CC);
	dump_hash(&XG(files),   "_FILES",   HASH_KEY_SIZEOF("_FILES"),   html, &str TSRMLS_CC);
	dump_hash(&XG(env),     "_ENV",     HASH_KEY_SIZEOF("_ENV"),     html, &str TSRMLS_CC);
	dump_hash(&XG(session), "_SESSION", HASH_KEY_SIZEOF("_SESSION"), html, &str TSRMLS_CC);
	dump_hash(&XG(request), "_REQUEST", HASH_KEY_SIZEOF("_REQUEST"), html, &str TSRMLS_CC);

	return str.d;
}

void xdebug_superglobals_dump_tok(xdebug_llist *l, char *str)
{
	char *tok;
	const char *sep = ",";

	tok = strtok(str, sep);
	while (tok != NULL) {
		char *p = tok + strlen(tok) - 1;

		while ((*tok == ' ') || (*tok == '\t')) {
			tok++;
		}

		while ((p > tok) && ((*p == ' ') || (*p == '\t'))) {
			p--;
		}

		*(p+1) = 0;

		/* we need to strdup each element so that we can safely free it */
		xdebug_llist_insert_next(l, NULL, strdup(tok));

		tok = strtok(NULL, sep);
	}
}

PHP_FUNCTION(xdebug_dump_superglobals)
{
	int html = PG(html_errors);
	char *superglobal_info = NULL;

	if (html) {
		php_printf("<table class='xdebug-superglobals' dir='ltr' border='1' cellspacing='0'>\n");
	}

	superglobal_info = xdebug_get_printable_superglobals(html TSRMLS_CC);
	if (superglobal_info) {
		php_printf("%s", xdebug_get_printable_superglobals(html TSRMLS_CC));
	} else {
		php_printf("<tr><td><i>No information about superglobals is available or configured.</i></td></tr>\n");
	}

	if (html) {
		php_printf("</table>\n");
	}
}
