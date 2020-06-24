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
#include "php_xdebug.h"

#include "lib_private.h"

extern ZEND_DECLARE_MODULE_GLOBALS(xdebug);

static int xdebug_header_handler(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s);
static int (*xdebug_orig_header_handler)(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s);

static void xdebug_header_remove_with_prefix(xdebug_llist *headers, char *prefix, size_t prefix_len)
{
	xdebug_llist_element *le;
	char                 *header;

	for (le = XDEBUG_LLIST_HEAD(XG_LIB(headers)); le != NULL; /* intentionally left blank*/) {
		header = XDEBUG_LLIST_VALP(le);

		if ((strlen(header) > prefix_len + 1) && (header[prefix_len] == ':') && (strncasecmp(header, prefix, prefix_len) == 0)) {
			xdebug_llist_element *current = le;

			le = XDEBUG_LLIST_NEXT(le);
			xdebug_llist_remove(headers, current, NULL);
		} else {
			le = XDEBUG_LLIST_NEXT(le);
		}
	}
}

static int xdebug_header_handler(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s)
{
	if (XG_LIB(headers)) {
		switch (op) {
			case SAPI_HEADER_ADD:
				xdebug_llist_insert_next(XG_LIB(headers), XDEBUG_LLIST_TAIL(XG_LIB(headers)), xdstrdup(h->header));
				break;
			case SAPI_HEADER_REPLACE: {
				char *colon_offset = strchr(h->header, ':');

				if (colon_offset) {
					char save = *colon_offset;

					*colon_offset = '\0';
					xdebug_header_remove_with_prefix(XG_LIB(headers), h->header, strlen(h->header));
					*colon_offset = save;
				}

				xdebug_llist_insert_next(XG_LIB(headers), XDEBUG_LLIST_TAIL(XG_LIB(headers)), xdstrdup(h->header));
			} break;
			case SAPI_HEADER_DELETE_ALL:
				xdebug_llist_empty(XG_LIB(headers), NULL);
			case SAPI_HEADER_DELETE:
			case SAPI_HEADER_SET_STATUS:
				break;
		}
	}
	if (xdebug_orig_header_handler) {
		return xdebug_orig_header_handler(h, op, s);
	}
	return SAPI_HEADER_ADD;
}

void xdebug_lib_zend_startup_overload_sapi_headers(void)
{
	/* Override header handler in SAPI */
	if (xdebug_orig_header_handler != NULL) {
		return;
	}

	xdebug_orig_header_handler = sapi_module.header_handler;
	sapi_module.header_handler = xdebug_header_handler;
}

void xdebug_lib_zend_shutdown_restore_sapi_headers(void)
{
	/* Restore original header handler in SAPI */
	sapi_module.header_handler = xdebug_orig_header_handler;
	xdebug_orig_header_handler = NULL;
}


PHP_FUNCTION(xdebug_get_headers)
{
	xdebug_llist_element *le;
	char                 *string;

	array_init(return_value);
	for (le = XDEBUG_LLIST_HEAD(XG_LIB(headers)); le != NULL; le = XDEBUG_LLIST_NEXT(le)) {
		string = XDEBUG_LLIST_VALP(le);
		add_next_index_string(return_value, string);
	}
}
