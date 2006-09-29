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

static void dump_hash_elem(zval *z, char *name, long index, char *elem, int html, int log TSRMLS_DC)
{
	char buffer[1024];
	int  len;

	if (html) {
		if (elem) {
			php_printf("<tr><td colspan='2' align='right' bgcolor='#ffffcc'>$%s['%s'] =</td>", name, elem);
		} else {
			php_printf("<tr><td colspan='2' align='right' bgcolor='#ffffcc'>$%s[%ld] =</td>", name, index);
		}
	}

	if (z != NULL) {
		char *val;

		if (html) {
			val = get_zval_value_fancy(NULL, z, &len, 0, NULL TSRMLS_CC);
			php_printf("<td bgcolor='#ffffcc'>");
			PHPWRITE(val, len);
			php_printf("</td>");
		} else {
			val = get_zval_value(z, 0, NULL);
			printf("\n   $%s['%s'] = %s", name, elem, val);
		}

		if (log) {
			snprintf(buffer, 1024, "PHP   $%s['%s'] = %s", name, elem, val);
			php_log_err(buffer TSRMLS_CC);
		}
	} else {
		/* not found */
		if (html) {
			php_printf("<td bgcolor='#ffff77'><i>undefined</i></td>");
		} else {
			printf("\n   $%s['%s'] is undefined", name, elem);
		}

		if (log) {
			snprintf(buffer, 1024, "PHP   $%s['%s'] is undefined", name, elem);
			php_log_err(buffer TSRMLS_CC);
		}
	}

	if (html) {
		php_printf("</tr>\n");
	}
}

static int dump_hash_elem_va(void *pDest, int num_args, va_list args, zend_hash_key *hash_key)
{
	int html, log;
	char *name;
#ifdef ZTS
	void ***tsrm_ls;
#endif

	name = va_arg(args, char *);
	html = va_arg(args, int);
	log = va_arg(args, int);

#ifdef ZTS
	tsrm_ls = va_arg(args, void ***);
#endif

	if (hash_key->nKeyLength == 0) {
		dump_hash_elem(*((zval **) pDest), name, hash_key->h, NULL, html, log TSRMLS_CC);
	} else {
		dump_hash_elem(*((zval **) pDest), name, 0, hash_key->arKey, html, log TSRMLS_CC);
	}

	return SUCCESS;
}

static void dump_hash(xdebug_llist *l, char *name, int name_len, int html, int log TSRMLS_DC)
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
		php_printf("<tr><th colspan='3' bgcolor='#dddd22'>Dump <i>$%s</i></th></tr>\n", name);
	} else {
		printf("\nDump $%s", name);
	}

	if (log) {
		char buffer[64];

		snprintf(buffer, 64, "PHP Dump $%s:", name);
		php_log_err(buffer TSRMLS_CC);
	}

	elem = XDEBUG_LLIST_HEAD(l);

	while (elem != NULL) {
		if (ht && (*((char *) (elem->ptr)) == '*')) {

#ifdef ZTS
#define X_DUMP_ARGS 4
#else
#define X_DUMP_ARGS 3
#endif

			zend_hash_apply_with_arguments(ht, dump_hash_elem_va, X_DUMP_ARGS, name, html, log TSRMLS_CC);
		} else if (ht && zend_hash_find(ht, elem->ptr, strlen(elem->ptr) + 1, (void **) &z) == SUCCESS) {
			dump_hash_elem(*z, name, 0, elem->ptr, html, log TSRMLS_CC);
		} else if(XG(dump_undefined)) {
			dump_hash_elem(NULL, name, 0, elem->ptr, html, log TSRMLS_CC);
		}

		elem = XDEBUG_LLIST_NEXT(elem);
	}
}

void dump_superglobals(int html, int log TSRMLS_DC)
{
	if (XG(dump_once) && XG(dumped)) {
		return;
	}

	XG(dumped) = 1;

	dump_hash(&XG(server),  "_SERVER",  8, html, log TSRMLS_CC);
	dump_hash(&XG(get),     "_GET",     5, html, log TSRMLS_CC);
	dump_hash(&XG(post),    "_POST",    6, html, log TSRMLS_CC);
	dump_hash(&XG(cookie),  "_COOKIE",  8, html, log TSRMLS_CC);
	dump_hash(&XG(files),   "_FILES",   7, html, log TSRMLS_CC);
	dump_hash(&XG(env),     "_ENV",     5, html, log TSRMLS_CC);
	dump_hash(&XG(session), "_SESSION", 9, html, log TSRMLS_CC);
	dump_hash(&XG(request), "_REQUEST", 9, html, log TSRMLS_CC);
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

	dump_superglobals(html , PG(log_errors) && !is_cli TSRMLS_CC);

	if (html) {
		php_printf("</table>\n");
	}
}
