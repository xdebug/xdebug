#include "xdebug_str.h"

typedef struct _xdebug_xml_attribute xdebug_xml_attribute;
typedef struct _xdebug_xml_node xdebug_xml_node;

struct _xdebug_xml_attribute
{
	char *name;
	char *value;
	struct _xdebug_xml_attribute *next;
};

struct _xdebug_xml_node
{
	char *tag;
	char *text;
	struct _xdebug_xml_attribute *attribute;
	struct _xdebug_xml_node      *child;
	struct _xdebug_xml_node      *next;
};

void xdebug_xml_return_node(xdebug_xml_node* node, struct xdebug_str *output);
xdebug_xml_node *xdebug_xml_node_init(char *tag, char *text);
void xdebug_xml_add_attribute(xdebug_xml_node* xml, char *attribute, char *value);
void xdebug_xml_add_child(xdebug_xml_node *xml, xdebug_xml_node *child);
void xdebug_xml_node_dtor(xdebug_xml_node* xml);
