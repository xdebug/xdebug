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

#include <stdlib.h>
#include <stdio.h>
#include "mm.h"
#include "str.h"
#include "var.h"
#include "xml.h"
#include "compat.h"

static void xdebug_xml_return_attribute(xdebug_xml_attribute* attr, xdebug_str* output)
{
	char *tmp;
	size_t newlen;

	xdebug_str_addc(output, ' ');

	/* attribute name */
	tmp = xdebug_xmlize(attr->name, attr->name_len, &newlen);
	xdebug_str_addl(output, tmp, newlen, 0);
	efree(tmp);

	/* attribute value */
	xdebug_str_add_literal(output, "=\"");
	if (attr->value) {
		tmp = xdebug_xmlize(attr->value, attr->value_len, &newlen);
		xdebug_str_add(output, tmp, 0);
		efree(tmp);
	}
	xdebug_str_addc(output, '\"');

	if (attr->next) {
		xdebug_xml_return_attribute(attr->next, output);
	}
}

static void xdebug_xml_return_text_node(xdebug_xml_text_node* node, xdebug_str* output)
{
	xdebug_str_add_literal(output, "<![CDATA[");
	if (node->encode) {
		/* if cdata tags are in the text, then we must base64 encode */
		size_t         new_len = 0;
		unsigned char *encoded_text;

		encoded_text = xdebug_base64_encode((unsigned char*) node->text, node->text_len, &new_len);
		xdebug_str_add(output, (char*) encoded_text, 0);
		xdfree(encoded_text);
	} else {
		xdebug_str_add(output, node->text, 0);
	}
	xdebug_str_add_literal(output, "]]>");
}

void xdebug_xml_return_node(xdebug_xml_node* node, struct xdebug_str *output)
{
	xdebug_str_addc(output, '<');
	xdebug_str_add(output, node->tag, 0);

	if (node->text && node->text->encode) {
		xdebug_xml_add_attribute_ex(node, "encoding", "base64", 0, 0);
	}
	if (node->attribute) {
		xdebug_xml_return_attribute(node->attribute, output);
	}
	xdebug_str_addc(output, '>');

	if (node->child) {
		xdebug_xml_return_node(node->child, output);
	}

	if (node->text) {
		xdebug_xml_return_text_node(node->text, output);
	}

	xdebug_str_add_literal(output, "</");
	xdebug_str_add(output, node->tag, 0);
	xdebug_str_addc(output, '>');

	if (node->next) {
		xdebug_xml_return_node(node->next, output);
	}
}

xdebug_xml_node *xdebug_xml_node_init_ex(const char *tag, int free_tag)
{
	xdebug_xml_node *xml = xdmalloc(sizeof (xdebug_xml_node));

	xml->tag = (char*) tag;
	xml->text = NULL;
	xml->child = NULL;
	xml->attribute = NULL;
	xml->next = NULL;
	xml->free_tag = free_tag;

	return xml;
}

void xdebug_xml_add_attribute_exl(xdebug_xml_node* xml, const char *attribute, size_t attribute_len, const char *value, size_t value_len, int free_name, int free_value)
{
	xdebug_xml_attribute *attr = xdmalloc(sizeof (xdebug_xml_attribute));
	xdebug_xml_attribute **ptr;

	/* Init structure */
	attr->name = (char*) attribute;
	attr->value = (char*) value;
	attr->name_len = attribute_len;
	attr->value_len = value_len;
	attr->next = NULL;
	attr->free_name = free_name;
	attr->free_value = free_value;

	/* Find last attribute in node */
	ptr = &xml->attribute;
	while (*ptr != NULL) {
		ptr = &(*ptr)->next;
	}
	*ptr = attr;
}

void xdebug_xml_add_child(xdebug_xml_node *xml, xdebug_xml_node *child)
{
	xdebug_xml_node **ptr;

	ptr = &xml->child;
	while (*ptr != NULL) {
		ptr = &((*ptr)->next);
	}
	*ptr = child;
}

static void xdebug_xml_text_node_dtor(xdebug_xml_text_node* node)
{
	if (node->free_value && node->text) {
		xdfree(node->text);
	}
	xdfree(node);
}

void xdebug_xml_add_text(xdebug_xml_node *xml, char *text)
{
	xdebug_xml_add_text_ex(xml, text, strlen(text), 1, 0);
}

void xdebug_xml_add_text_encode(xdebug_xml_node *xml, char *text)
{
	xdebug_xml_add_text_ex(xml, text, strlen(text), 1, 1);
}

void xdebug_xml_add_text_ex(xdebug_xml_node *xml, char *text, int length, int free_text, int encode)
{
	xdebug_xml_text_node *node = xdmalloc(sizeof (xdebug_xml_text_node));
	node->free_value = free_text;
	node->encode = encode;

	if (xml->text) {
		xdebug_xml_text_node_dtor(xml->text);
	}
	node->text = text;
	node->text_len = length;
	xml->text = node;
	if (!encode && strstr(node->text, "]]>")) {
		node->encode = 1;
	}
}

static void xdebug_xml_attribute_dtor(xdebug_xml_attribute *attr)
{
	if (attr->next) {
		xdebug_xml_attribute_dtor(attr->next);
	}
	if (attr->free_name) {
		xdfree(attr->name);
	}
	if (attr->free_value) {
		xdfree(attr->value);
	}
	xdfree(attr);
}

void xdebug_xml_node_dtor(xdebug_xml_node* xml)
{
	if (xml->next) {
		xdebug_xml_node_dtor(xml->next);
	}
	if (xml->child) {
		xdebug_xml_node_dtor(xml->child);
	}
	if (xml->attribute) {
		xdebug_xml_attribute_dtor(xml->attribute);
	}
	if (xml->free_tag) {
		xdfree(xml->tag);
	}
	if (xml->text) {
		xdebug_xml_text_node_dtor(xml->text);
	}
	xdfree(xml);
}
