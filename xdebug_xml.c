#include <stdlib.h>
#include <stdio.h>
#include "xdebug_mm.h"
#include "xdebug_str.h"
#include "xdebug_xml.h"

static void xdebug_xml_return_attribute(xdebug_xml_attribute* attr, xdebug_str* output)
{
	xdebug_str_addl(output, " ", 1, 0);
	xdebug_str_add(output, attr->name, 0);
	xdebug_str_addl(output, "=\"", 2, 0);
	xdebug_str_add(output, attr->value, 0);
	xdebug_str_addl(output, "\"", 1, 0);
	
	if (attr->next) {
		xdebug_xml_return_attribute(attr->next, output);
	}
}

void xdebug_xml_return_node(xdebug_xml_node* node, struct xdebug_str *output)
{
	xdebug_str_addl(output, "<", 1, 0);
	xdebug_str_add(output, node->tag, 0);

	if (node->attribute) {
		xdebug_xml_return_attribute(node->attribute, output);
	}
	xdebug_str_addl(output, ">", 1, 0);

	if (node->child) {
		xdebug_xml_return_node(node->child, output);
	}

	if (node->text) {
		xdebug_str_add(output, node->text, 0);
	}

	xdebug_str_addl(output, "</", 2, 0);
	xdebug_str_add(output, node->tag, 0);
	xdebug_str_addl(output, ">", 1, 0);

	if (node->next) {
		xdebug_xml_return_node(node->next, output);
	}
}

xdebug_xml_node *xdebug_xml_node_init(char *tag, char *text)
{
	xdebug_xml_node *xml = xdmalloc(sizeof (xdebug_xml_node));

	xml->tag = tag;
	xml->text = text;
	xml->child = NULL;
	xml->attribute = NULL;
	xml->next = NULL;

	return xml;
}

void xdebug_xml_add_attribute(xdebug_xml_node* xml, char *attribute, char *value)
{
	xdebug_xml_attribute *attr = xdmalloc(sizeof (xdebug_xml_attribute));
	xdebug_xml_attribute **ptr;

	/* Init structure */
	attr->name = attribute;
	attr->value = value;
	attr->next = NULL;

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

static void xdebug_xml_attribute_dtor(xdebug_xml_attribute *attr)
{
	if (attr->next) {
		xdebug_xml_attribute_dtor(attr->next);
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
	xdfree(xml);
}
