/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002, 2003, 2004, 2005, 2006 Derick Rethans            |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.0 of the Xdebug license,    |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://xdebug.derickrethans.nl/license.php                           |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | xdebug@derickrethans.nl so we can mail you a copy immediately.       |
   +----------------------------------------------------------------------+
   | Authors: Harald Radi <harald.radi@nme.at>                            |
   +----------------------------------------------------------------------+
 */

#include "php_xdebug.h"
#include "xdebug_private.h"
#include "xdebug_var.h"
#include "xdebug_superglobals.h"
#include "SAPI.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

void dump_dtor(void *user, void *ptr)
{
	free(ptr);
}

static void dump_hash_elem(zval *z, char *name, long index, char *elem, int html TSRMLS_DC)
{
	char buffer[1024];
	int  len;

	if (html) {
		if (elem) {
			php_printf("<tr><td colspan='2' align='right' bgcolor='#eeeeec' valign='top'><pre>$%s['%s']&nbsp;=</pre></td>", name, elem);
		} else {
			php_printf("<tr><td colspan='2' align='right' bgcolor='#eeeeec' valign='top'><pre>$%s[%ld]&nbsp;=</pre></td>", name, index);
		}
	}

	if (z != NULL) {
		char *val;

		if (html) {
			val = get_zval_value_fancy(NULL, z, &len, 0, NULL TSRMLS_CC);
#if HAVE_PHP_MEMORY_USAGE
			php_printf("<td colspan='3' bgcolor='#eeeeec'>");
#else
			php_printf("<td colspan='2' bgcolor='#eeeeec'>");
#endif
			PHPWRITE(val, len);
			php_printf("</td>");
		} else {
			val = get_zval_value(z, 0, NULL);
			printf("\n   $%s['%s'] = %s", name, elem, val);
		}
	} else {
		/* not found */
		if (html) {
#if HAVE_PHP_MEMORY_USAGE
			php_printf("<td colspan='3' bgcolor='#eeeeec'><i>undefined</i></td>");
#else
			php_printf("<td colspan='2' bgcolor='#eeeeec'><i>undefined</i></td>");
#endif
		} else {
			printf("\n   $%s['%s'] is undefined", name, elem);
		}
	}

	if (html) {
		php_printf("</tr>\n");
	}
}

static int dump_hash_elem_va(void *pDest, int num_args, va_list args, zend_hash_key *hash_key)
{
	int html;
	char *name;
#ifdef ZTS
	void ***tsrm_ls;
#endif

	name = va_arg(args, char *);
	html = va_arg(args, int);

#ifdef ZTS
	tsrm_ls = va_arg(args, void ***);
#endif

	if (hash_key->nKeyLength == 0) {
		dump_hash_elem(*((zval **) pDest), name, hash_key->h, NULL, html TSRMLS_CC);
	} else {
		dump_hash_elem(*((zval **) pDest), name, 0, hash_key->arKey, html TSRMLS_CC);
	}

	return SUCCESS;
}

static void dump_hash(xdebug_llist *l, char *name, int name_len, int html TSRMLS_DC)
{
	zval **z;
	HashTable *ht;
	xdebug_llist_element *elem;

	if (!XDEBUG_LLIST_COUNT(l)) {
		return;
	}

	if (zend_hash_find(&EG(symbol_table), name, name_len, (void **) &z) != SUCCESS) {
		ht = NULL;
	} else {
		ht = Z_ARRVAL_PP(z);
	}

	if (html) {
#if HAVE_PHP_MEMORY_USAGE
		php_printf("<tr><th colspan='5' align='left' bgcolor='#e9b96e'>Dump <i>$%s</i></th></tr>\n", name);
#else
		php_printf("<tr><th colspan='4' align='left' bgcolor='#e9b96e'>Dump <i>$%s</i></th></tr>\n", name);
#endif
	} else {
		printf("\nDump $%s", name);
	}

	elem = XDEBUG_LLIST_HEAD(l);

	while (elem != NULL) {
		if (ht && (*((char *) (elem->ptr)) == '*')) {

#ifdef ZTS
#define X_DUMP_ARGS 3
#else
#define X_DUMP_ARGS 2
#endif

			zend_hash_apply_with_arguments(ht, dump_hash_elem_va, X_DUMP_ARGS, name, html TSRMLS_CC);
		} else if (ht && zend_hash_find(ht, elem->ptr, strlen(elem->ptr) + 1, (void **) &z) == SUCCESS) {
			dump_hash_elem(*z, name, 0, elem->ptr, html TSRMLS_CC);
		} else if(XG(dump_undefined)) {
			dump_hash_elem(NULL, name, 0, elem->ptr, html TSRMLS_CC);
		}

		elem = XDEBUG_LLIST_NEXT(elem);
	}
}

void dump_superglobals(int html TSRMLS_DC)
{
	if (XG(dump_once) && XG(dumped)) {
		return;
	}

	XG(dumped) = 1;

	dump_hash(&XG(server),  "_SERVER",  8, html TSRMLS_CC);
	dump_hash(&XG(get),     "_GET",     5, html TSRMLS_CC);
	dump_hash(&XG(post),    "_POST",    6, html TSRMLS_CC);
	dump_hash(&XG(cookie),  "_COOKIE",  8, html TSRMLS_CC);
	dump_hash(&XG(files),   "_FILES",   7, html TSRMLS_CC);
	dump_hash(&XG(env),     "_ENV",     5, html TSRMLS_CC);
	dump_hash(&XG(session), "_SESSION", 9, html TSRMLS_CC);
	dump_hash(&XG(request), "_REQUEST", 9, html TSRMLS_CC);
}

void dump_tok(xdebug_llist *l, char *str)
{
	char *tok, *sep = ",";

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
	int is_cli = (strcmp("cli", sapi_module.name) == 0);
	int html = PG(html_errors);

	if (html) {
		php_printf("<table border='1' cellspacing='0'>\n");
	}

	dump_superglobals(html TSRMLS_CC);

	if (html) {
		php_printf("</table>\n");
	}
}
