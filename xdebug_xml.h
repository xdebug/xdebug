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
   | Authors:  Derick Rethans <derick@xdebug.org>                         |
   +----------------------------------------------------------------------+
 */

#include "xdebug_str.h"

#ifndef __HAVE_XDEBUG_XML_H__
#define __HAVE_XDEBUG_XML_H__

typedef struct _xdebug_xml_attribute xdebug_xml_attribute;
typedef struct _xdebug_xml_text_node xdebug_xml_text_node;
typedef struct _xdebug_xml_node xdebug_xml_node;

struct _xdebug_xml_attribute
{
	char *name;
	char *value;
	struct _xdebug_xml_attribute *next;
	int   free_name;
	int   free_value;
};

/* todo: support multiple text nodes inside an element */
struct _xdebug_xml_text_node
{
	char *text;
	int   free_value;
	int   encode;
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


#define xdebug_xml_node_init(t)         xdebug_xml_node_init_ex((t), 0)
#define xdebug_xml_add_attribute(x,a,v) xdebug_xml_add_attribute_ex((x), (a), (v), 0, 0);

xdebug_xml_node *xdebug_xml_node_init_ex(char *tag, int free_tag);
void xdebug_xml_add_attribute_ex(xdebug_xml_node* xml, char *attribute, char *value, int free_name, int free_value);
void xdebug_xml_add_child(xdebug_xml_node *xml, xdebug_xml_node *child);

void xdebug_xml_add_text_ex(xdebug_xml_node *xml, char *text, int free_text, int encode);
#define xdebug_xml_add_text(x,t) 	 xdebug_xml_add_text_ex((x), (t), 1, 0)
#define xdebug_xml_add_text_encode(x,t)  xdebug_xml_add_text_ex((x), (t), 1, 1)

void xdebug_xml_return_node(xdebug_xml_node* node, struct xdebug_str *output);
void xdebug_xml_node_dtor(xdebug_xml_node* xml);

#endif
