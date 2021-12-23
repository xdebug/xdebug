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

#include "lib/str.h"

#ifndef __HAVE_XDEBUG_XML_H__
#define __HAVE_XDEBUG_XML_H__

typedef struct _xdebug_xml_attribute xdebug_xml_attribute;
typedef struct _xdebug_xml_text_node xdebug_xml_text_node;
typedef struct _xdebug_xml_node xdebug_xml_node;

struct _xdebug_xml_attribute
{
	char                         *name;
	int                           name_len;
	xdebug_str                   *value;
	struct _xdebug_xml_attribute *next;
	int                           free_name;
};

/* todo: support multiple text nodes inside an element */
struct _xdebug_xml_text_node
{
	char *text;
	int   free_value;
	int   encode;
	int   text_len;
};

struct _xdebug_xml_node
{
	char *tag;
	struct _xdebug_xml_text_node *text;
	struct _xdebug_xml_attribute *attribute;
	struct _xdebug_xml_node      *child;
	struct _xdebug_xml_node      *next;
	int   free_tag;
};


#define xdebug_xml_node_init(t)            xdebug_xml_node_init_ex((t), 0)
#define xdebug_xml_add_attribute_ex(x,a,v,fa,fv) { const char *ta = (a), *tv = (v); xdebug_xml_add_attribute_exl((x), (ta), strlen((ta)), (tv), strlen((tv)), fa, fv); }
#define xdebug_xml_add_attribute(x,a,v)    xdebug_xml_add_attribute_ex((x), (a), (v), 0, 0);

xdebug_xml_node *xdebug_xml_node_init_ex(const char *tag, int free_tag);
void xdebug_xml_add_attribute_exl(xdebug_xml_node* xml, const char *attribute, size_t attribute_len, const char *value, size_t value_len, int free_name, int free_value);
void xdebug_xml_add_child(xdebug_xml_node *xml, xdebug_xml_node *child);

void xdebug_xml_add_text_ex(xdebug_xml_node *xml, char *text, int length, int free_text, int encode);
void xdebug_xml_add_text(xdebug_xml_node *xml, char *text);
void xdebug_xml_add_text_encode(xdebug_xml_node *xml, char *text);
#define xdebug_xml_add_textl(x,t,l) 	 xdebug_xml_add_text_ex((x), (t), (l), 1, 0)
#define xdebug_xml_add_text_encodel(x,t,l)  xdebug_xml_add_text_ex((x), (t), (l), 1, 1)

xdebug_str *xdebug_xml_get_attribute_value(xdebug_xml_node *xml, const char *attribute);
#define xdebug_xml_expand_attribute_value(n,a,v) { \
	xdebug_str *orig_value = xdebug_xml_get_attribute_value((n), (a)); \
	if (orig_value) { \
		xdebug_str_addc(orig_value, ' '); \
		xdebug_str_add(orig_value, v, 0); \
	} else { \
		xdebug_xml_add_attribute((n), (a), (v)); \
	} \
}

void xdebug_xml_return_node(xdebug_xml_node* node, struct xdebug_str *output);
void xdebug_xml_node_dtor(xdebug_xml_node* xml);

#endif
