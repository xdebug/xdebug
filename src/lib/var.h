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

#include "zend.h"
#include "php_xdebug.h"

#include "lib/compat.h"
#include "lib/lib.h"
#include "lib/str.h"
#include "lib/xml.h"

#ifndef __HAVE_XDEBUG_VAR_H__
#define __HAVE_XDEBUG_VAR_H__

/* Set correct int format to use */
#include "Zend/zend_long.h"
#if SIZEOF_ZEND_LONG == 4
# define XDEBUG_INT_FMT "%ld"
#else
# define XDEBUG_INT_FMT "%lld"
#endif

typedef struct
{
	int page; /* The number of the page to retrieve */
	int current_element_nr;
	int start_element_nr;
	int end_element_nr;
} xdebug_var_runtime_page;

typedef struct xdebug_var_export_options {
	int max_children;
	int max_data;
	int max_depth;
	int show_hidden;
	int extended_properties;         /* Whether the feature is enabled */
	int encode_as_extended_property; /* Whether the current node's elements need to be encoded */
	int show_location;
	xdebug_var_runtime_page *runtime;
	int no_decoration;
} xdebug_var_export_options;

#define XDEBUG_VAR_TYPE_NORMAL   0x00
#define XDEBUG_VAR_TYPE_STATIC   0x01
#define XDEBUG_VAR_TYPE_CONSTANT 0x02

void xdebug_dump_used_var_with_contents(void *htmlq, xdebug_hash_element* he, void *argument);
void xdebug_get_php_symbol(zval *retval, xdebug_str* name);

xdebug_var_export_options* xdebug_var_export_options_from_ini(void);
xdebug_var_export_options* xdebug_var_get_nolimit_options(void);

#if PHP_VERSION_ID >= 70400
xdebug_str* xdebug_get_property_type(zval* object, zval *val);
#endif
xdebug_str* xdebug_get_property_info(char *mangled_property, int mangled_len, const char **modifier, char **class_name);
#if PHP_VERSION_ID >= 70400
HashTable *xdebug_objdebug_pp(zval **zval_pp);
#else
HashTable *xdebug_objdebug_pp(zval **zval_pp, int *is_tmp);
void xdebug_var_maybe_destroy_ht(HashTable *ht, int is_temp);
#endif


#define XDEBUG_VAR_ATTR_TEXT 0
#define XDEBUG_VAR_ATTR_HTML 1
void xdebug_add_variable_attributes(xdebug_str *str, zval *struc, zend_bool fancy);


char* xdebug_xmlize(char *string, size_t len, size_t *newlen);
char* xdebug_error_type_simple(int type);
char* xdebug_error_type(int type);
zval *xdebug_get_zval(zend_execute_data *zdata, int node_type, const znode_op *node, int *is_var);
zval *xdebug_get_zval_with_opline(zend_execute_data *zdata, const zend_op *opline, int node_type, const znode_op *node, int *is_var);

char* xdebug_show_fname(xdebug_func t, int html, int flags);

#endif
