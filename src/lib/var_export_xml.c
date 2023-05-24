/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2023 Derick Rethans                               |
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

#include "var_export_xml.h"
#include "Zend/zend_closures.h"

static xdebug_str *prepare_variable_name(xdebug_str *name)
{
	xdebug_str *tmp_name;

	if (name->d[0] == '$' || name->d[0] == ':') {
		tmp_name = xdebug_str_copy(name);
	} else {
		tmp_name = xdebug_str_new();
		xdebug_str_addc(tmp_name, '$');
		xdebug_str_add_str(tmp_name, name);
	}
	if (tmp_name->d[tmp_name->l - 2] == ':' && tmp_name->d[tmp_name->l - 1] == ':') {
		xdebug_str_chop(tmp_name, 2);
	}
	return tmp_name;
}

/*
 * Returns whether we should attempt to encode name, fullname, and value as XML
 * elements instead of attribute values, because XML doesn't support nearly all
 * characters under ASCII 0x32.
 */
static int encoding_requested(char *value, size_t value_len)
{
	size_t i;

	for (i = 0; i < value_len; i++) {
		if (value[i] < 0x20) {
			return 1;
		}
	}
	return 0;
}

static void check_if_extended_properties_are_needed(xdebug_var_export_options *options, xdebug_str *name, xdebug_str *fullname, zval *value)
{
	if (!options->extended_properties || options->encode_as_extended_property) {
		return;
	}

	/* Checking name */
	if (name && encoding_requested(name->d, name->l)) {
		options->encode_as_extended_property = 1;
		return;
	}

	/* Checking full name */
	if (fullname && encoding_requested(fullname->d, fullname->l)) {
		options->encode_as_extended_property = 1;
		return;
	}

	/* Checking for the value portion */
	if (!value) {
		return;
	}
	if (Z_TYPE_P(value) == IS_STRING) {
		if (encoding_requested(Z_STRVAL_P(value), Z_STRLEN_P(value))) {
			options->encode_as_extended_property = 1;
			return;
		}
	}
	if (Z_TYPE_P(value) == IS_OBJECT) {
		if (encoding_requested(STR_NAME_VAL(Z_OBJCE_P(value)->name), STR_NAME_LEN(Z_OBJCE_P(value)->name))) {
			options->encode_as_extended_property = 1;
			return;
		}
	}
}

static void add_xml_attribute_or_element(xdebug_var_export_options *options, xdebug_xml_node *node, const char *field, int field_len, xdebug_str *value)
{
	if (options->encode_as_extended_property || (encoding_requested(value->d, value->l) && options->extended_properties)) {
		xdebug_xml_node *element;
		unsigned char   *tmp_base64;
		size_t           new_len;

		options->encode_as_extended_property = 1;

		element = xdebug_xml_node_init(field);
		xdebug_xml_add_attribute(element, "encoding", "base64");

		tmp_base64 = xdebug_base64_encode((unsigned char*) value->d, value->l, &new_len);
		xdebug_xml_add_text_ex(element, (char*) tmp_base64, new_len, 1, 0);

		xdebug_xml_add_child(node, element);
	} else {
		xdebug_xml_add_attribute_exl(node, field, field_len, xdstrndup(value->d, value->l), value->l, 0, 1);
	}
}

void xdebug_var_xml_attach_uninitialized_var(xdebug_var_export_options *options, xdebug_xml_node *node, xdebug_str *name)
{
	xdebug_xml_node *contents = NULL;
	xdebug_str      *tmp_name;

	contents = xdebug_xml_node_init("property");
	options->encode_as_extended_property = 0;

	tmp_name = prepare_variable_name(name);
	add_xml_attribute_or_element(options, contents, "name", 4, tmp_name);
	add_xml_attribute_or_element(options, contents, "fullname", 8, tmp_name);
	xdebug_str_free(tmp_name);

	xdebug_xml_add_attribute(contents, "type", "uninitialized");
	xdebug_xml_add_child(node, contents);
}

/*****************************************************************************
** XML node printing routines
*/

#define XDEBUG_OBJECT_ITEM_TYPE_PROPERTY           0
#define XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY 1<<0
#define XDEBUG_OBJECT_ITEM_TYPE_READONLY        1<<1


typedef struct
{
	char          type;
	char         *name;
	int           name_len;
	unsigned long index_key;
	zval         *zv;
} xdebug_object_item;

static void merged_hash_object_item_dtor(zval *data)
{
	 xdebug_object_item *item = Z_PTR_P(data);

	 xdfree(item);
}

static int object_item_add_to_merged_hash(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, HashTable *merged, int object_type)
{
	zval **zv = &zv_nptr;
	xdebug_object_item *item;

	item = xdcalloc(1, sizeof(xdebug_object_item));
	item->type = object_type;
	item->zv   = *zv;

	if (hash_key) {
		item->name = (char*) HASH_APPLY_KEY_VAL(hash_key);
		item->name_len = HASH_APPLY_KEY_LEN(hash_key) - 1;
		item->index_key = hash_key->h;
	} else {
		item->name = xdebug_sprintf(XDEBUG_INT_FMT, index_key);
		item->name_len = strlen(item->name);
	}

	zend_hash_next_index_insert_ptr(merged, item);

	return 0;
}

static int object_item_add_zend_prop_to_merged_hash(zend_property_info *zpp, HashTable *merged, int object_type, zend_class_entry *ce)
{
	xdebug_object_item *item;

	if ((zpp->flags & ZEND_ACC_STATIC) == 0) {
		return 0;
	}

	item = xdmalloc(sizeof(xdebug_object_item));
	item->type = object_type;
	item->zv   = &CE_STATIC_MEMBERS(ce)[zpp->offset];
	item->name = (char*) STR_NAME_VAL(zpp->name);
	item->name_len = STR_NAME_LEN(zpp->name);

	zend_hash_next_index_insert_ptr(merged, item);

	return 0;
}

static void add_unencoded_text_value_attribute_or_element(xdebug_var_export_options *options, xdebug_xml_node *node, char *value)
{
	if (options->encode_as_extended_property) {
		xdebug_xml_node *element;
		unsigned char   *tmp_base64;
		size_t           new_len;

		element = xdebug_xml_node_init("value");
		xdebug_xml_add_attribute(element, "encoding", "base64");

		tmp_base64 = xdebug_base64_encode((unsigned char*) value, strlen(value), &new_len);
		xdebug_xml_add_text_ex(element, (char*) tmp_base64, new_len, 1, 0);

		xdebug_xml_add_child(node, element);
	} else {
		xdebug_xml_add_text(node, value);
	}
}

static void add_encoded_text_value_attribute_or_element(xdebug_var_export_options *options, xdebug_xml_node *node, char *value, size_t value_len)
{
	if (options->encode_as_extended_property) {
		xdebug_xml_node *element;
		unsigned char   *tmp_base64;
		size_t           new_len;

		element = xdebug_xml_node_init("value");
		xdebug_xml_add_attribute(element, "encoding", "base64");

		tmp_base64 = xdebug_base64_encode((unsigned char*) value, value_len, &new_len);
		xdebug_xml_add_text_ex(element, (char*) tmp_base64, new_len, 1, 0);

		xdebug_xml_add_child(node, element);

		xdfree(value);
	} else {
		xdebug_xml_add_text_encodel(node, value, value_len);
	}
}


static int xdebug_array_element_export_xml_node(zval *zv_nptr, zend_ulong index_key, zend_string *hash_key, int level, xdebug_xml_node *parent, xdebug_str *parent_name, xdebug_var_export_options *options)
{
	zval            **zv = &zv_nptr;
	xdebug_xml_node  *node;
	xdebug_str       *name;
	xdebug_str        full_name = XDEBUG_STR_INITIALIZER;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		node = xdebug_xml_node_init("property");
		options->encode_as_extended_property = 0;

		if (!HASH_KEY_IS_NUMERIC(hash_key)) { /* string key */
			zend_string *i_string = zend_string_init(HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key) - 1, 0);
			zend_string *tmp_fullname_zstr;

			tmp_fullname_zstr = xdebug_addslashes(i_string);

			name = xdebug_str_create(HASH_APPLY_KEY_VAL(hash_key), HASH_APPLY_KEY_LEN(hash_key) - 1);

			if (parent_name) {
				xdebug_str_add_str(&full_name, parent_name);
				xdebug_str_add_literal(&full_name, "[\"");
				xdebug_str_addl(&full_name, tmp_fullname_zstr->val, tmp_fullname_zstr->len, 0);
				xdebug_str_add_literal(&full_name, "\"]");
			}

			zend_string_release(tmp_fullname_zstr);
			zend_string_release(i_string);
		} else {
			char *tmp_idx = xdebug_sprintf(XDEBUG_INT_FMT, index_key);

			name = xdebug_str_create(tmp_idx, strlen(tmp_idx));
			if (parent_name) {
				xdebug_str_add_str(&full_name, parent_name);
				xdebug_str_addc(&full_name, '[');
				xdebug_str_add_str(&full_name, name);
				xdebug_str_addc(&full_name, ']');
			}

			xdfree(tmp_idx);
		}

		check_if_extended_properties_are_needed(options, name, full_name.l ? &full_name : NULL, *zv);
		add_xml_attribute_or_element(options, node, "name", 4, name);
		if (full_name.l) {
			add_xml_attribute_or_element(options, node, "fullname", 8, &full_name);
		}

		xdebug_xml_add_child(parent, node);
		xdebug_var_export_xml_node(zv, &full_name, node, options, level + 1);

		xdebug_str_destroy(&full_name);
		xdebug_str_free(name);
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static int xdebug_object_element_export_xml_node(xdebug_object_item *item_nptr, int level, xdebug_xml_node *parent, xdebug_str *parent_name, xdebug_var_export_options *options, char *class_name)
{
	xdebug_object_item **item = &item_nptr;
	xdebug_xml_node *node;

	if (options->runtime[level].current_element_nr >= options->runtime[level].start_element_nr &&
		options->runtime[level].current_element_nr < options->runtime[level].end_element_nr)
	{
		const char *modifier;
		xdebug_str *tmp_name = NULL;
		xdebug_str *tmp_fullname = NULL;

		node = xdebug_xml_node_init("property");
		options->encode_as_extended_property = 0;

		if ((*item)->name != NULL) {
			char       *prop_class_name;
			xdebug_str *property_name;

			property_name = xdebug_get_property_info((*item)->name, (*item)->name_len + 1, &modifier, &prop_class_name);

			if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
				tmp_name = xdebug_str_copy(property_name);
			} else {
				tmp_name = xdebug_str_new();

				xdebug_str_addc(tmp_name, '*');
				xdebug_str_add(tmp_name, prop_class_name, 0);
				xdebug_str_addc(tmp_name, '*');
				xdebug_str_add_str(tmp_name, property_name);
			}

			if (parent_name) {
				tmp_fullname = xdebug_str_new();

				xdebug_str_add_str(tmp_fullname, parent_name);
				if ((*item)->type & XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY) {
					xdebug_str_add_literal(tmp_fullname, "::");
				} else {
					xdebug_str_add_literal(tmp_fullname, "->");
				}

				/* Only in dynamic and *public* properties can we have non-standard characters */
				if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
					if (property_name->l == 0) {
						xdebug_str_add_literal(tmp_fullname, "{\"\"}");
					} else {
						if (memchr(property_name->d, '-', property_name->l) == NULL && memchr(property_name->d, '[', property_name->l) == NULL && memchr(property_name->d, '{', property_name->l) == NULL) {
							xdebug_str_add_str(tmp_fullname, property_name);
						} else {
							zend_string *tmp_string = zend_string_init(property_name->d, property_name->l, 0);
							zend_string *tmp_slashed_string;

							tmp_slashed_string = xdebug_addslashes(tmp_string);

							xdebug_str_add_literal(tmp_fullname, "{\"");
							xdebug_str_add_zstr(tmp_fullname, tmp_slashed_string);
							xdebug_str_add_literal(tmp_fullname, "\"}");

							zend_string_release(tmp_slashed_string);
							zend_string_release(tmp_string);
						}
					}
				} else {
					xdebug_str_addc(tmp_fullname, '*');
					xdebug_str_add(tmp_fullname, prop_class_name, 0);
					xdebug_str_addc(tmp_fullname, '*');
					xdebug_str_add_str(tmp_fullname, property_name);
				}
			}

			xdebug_str_free(property_name);
			xdfree(prop_class_name);
		} else { /* Numerical property name */
			modifier = "public";

			{
				char       *tmp_formatted_prop;

				tmp_formatted_prop = xdebug_sprintf(XDEBUG_INT_FMT, (*item)->index_key);
				tmp_name = xdebug_str_create_from_char(tmp_formatted_prop);

				add_xml_attribute_or_element(options, node, "name", 4, tmp_name);

				xdfree(tmp_formatted_prop);
			}

			if (parent_name) {
				char       *tmp_formatted_prop;

				tmp_formatted_prop = xdebug_sprintf("%s%s" XDEBUG_INT_FMT, parent_name, (*item)->type & XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY ? "::" : "->", (*item)->index_key);
				tmp_fullname = xdebug_str_create_from_char(tmp_formatted_prop);

				xdfree(tmp_formatted_prop);
			}
		}

		check_if_extended_properties_are_needed(options, tmp_name, tmp_fullname, (*item)->zv);
		add_xml_attribute_or_element(options, node, "name", 4, tmp_name);
		if (tmp_fullname) {
			add_xml_attribute_or_element(options, node, "fullname", 8, tmp_fullname);
		}


		if ((*item)->type & XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY) {
			xdebug_xml_expand_attribute_value(node, "facet", "static");
		}
		xdebug_xml_expand_attribute_value(node, "facet", modifier);
		if ((*item)->type & XDEBUG_OBJECT_ITEM_TYPE_READONLY) {
			xdebug_xml_expand_attribute_value(node, "facet", "readonly");
		}

		xdebug_xml_add_child(parent, node);
		xdebug_var_export_xml_node(&((*item)->zv), tmp_fullname ? tmp_fullname : NULL, node, options, level + 1);

		if (tmp_name) {
			xdebug_str_free(tmp_name);
		}
		if (tmp_fullname) {
			xdebug_str_free(tmp_fullname);
		}
	}
	options->runtime[level].current_element_nr++;
	return 0;
}

static void xdebug_var_xml_attach_property_with_contents(zend_property_info *prop_info, xdebug_xml_node *node, xdebug_var_export_options *options, zend_class_entry *class_entry, char *class_name, int *children_count)
{
	const char         *modifier;
	xdebug_xml_node    *contents = NULL;
	char               *prop_class_name;
	xdebug_str         *property_name;

	if ((prop_info->flags & ZEND_ACC_STATIC) == 0) {
		return;
	}

	(*children_count)++;
	property_name = xdebug_get_property_info(STR_NAME_VAL(prop_info->name), STR_NAME_LEN(prop_info->name) + 1, &modifier, &prop_class_name);

	if (strcmp(modifier, "private") != 0 || strcmp(class_name, prop_class_name) == 0) {
		contents = xdebug_get_zval_value_xml_node_ex(property_name, &CE_STATIC_MEMBERS(class_entry)[prop_info->offset], XDEBUG_VAR_TYPE_STATIC, options);
	} else{
		xdebug_str *priv_name = xdebug_str_new();

		xdebug_str_addc(priv_name, '*');
		xdebug_str_add(priv_name, prop_class_name, 0);
		xdebug_str_addc(priv_name, '*');
		xdebug_str_add_str(priv_name, property_name);

		contents = xdebug_get_zval_value_xml_node_ex(priv_name, &CE_STATIC_MEMBERS(class_entry)[prop_info->offset], XDEBUG_VAR_TYPE_STATIC, options);

		xdebug_str_free(priv_name);
	}

	xdebug_str_free(property_name);
	xdfree(prop_class_name);

	if (contents) {
		xdebug_xml_expand_attribute_value(contents, "facet", "static");
		xdebug_xml_expand_attribute_value(contents, "facet", modifier);
		xdebug_xml_add_child(node, contents);
	} else {
		xdebug_var_xml_attach_uninitialized_var(options, node, xdebug_str_create(ZSTR_VAL(prop_info->name), ZSTR_LEN(prop_info->name)));
	}
}

void xdebug_var_xml_attach_static_vars(xdebug_xml_node *node, xdebug_var_export_options *options, zend_class_entry *ce)
{
	HashTable        *static_members = &ce->properties_info;
	int               children = 0;
	xdebug_xml_node  *static_container;
	zend_property_info *zpi;

	static_container = xdebug_xml_node_init("property");
	options->encode_as_extended_property = 0;

	xdebug_xml_add_attribute(static_container, "name", "::");
	xdebug_xml_add_attribute(static_container, "fullname", "::");
	xdebug_xml_add_attribute(static_container, "type", "object");
	xdebug_xml_add_attribute_ex(static_container, "classname", xdstrdup(STR_NAME_VAL(ce->name)), 0, 1);

	xdebug_zend_hash_apply_protection_begin(static_members);

#if PHP_VERSION_ID >= 80100
	if (ce->default_static_members_count && !CE_STATIC_MEMBERS(ce)) {
		zend_class_init_statics(ce);
	}
#endif

	ZEND_HASH_FOREACH_PTR(static_members, zpi) {
		xdebug_var_xml_attach_property_with_contents(zpi, static_container, options, ce, STR_NAME_VAL(ce->name), &children);
	} ZEND_HASH_FOREACH_END();

	xdebug_zend_hash_apply_protection_end(static_members);

	xdebug_xml_add_attribute(static_container, "children", children > 0 ? "1" : "0");
	xdebug_xml_add_attribute_ex(static_container, "numchildren", xdebug_sprintf("%d", children), 0, 1);

	xdebug_xml_add_child(node, static_container);
}

void xdebug_var_export_xml_node(zval **struc, xdebug_str *name, xdebug_xml_node *node, xdebug_var_export_options *options, int level)
{
	HashTable *myht;
	zend_ulong num;
	zend_string *key;
	zval *z_val;
	xdebug_object_item *xoi_val;
	zval *tmpz;

	if (!*struc) {
		xdebug_xml_add_attribute(node, "type", "uninitialized");
		return;
	}

	if (Z_TYPE_P(*struc) == IS_INDIRECT) {
		tmpz = ((*struc)->value.zv);
		struc = &tmpz;
	}
	if (Z_TYPE_P(*struc) == IS_REFERENCE) {
		tmpz = &((*struc)->value.ref->val);
		struc = &tmpz;
	}

	switch (Z_TYPE_P(*struc)) {
		case IS_TRUE:
		case IS_FALSE:
			xdebug_xml_add_attribute(node, "type", "bool");
			add_unencoded_text_value_attribute_or_element(options, node, xdebug_sprintf("%d", Z_TYPE_P(*struc) == IS_TRUE ? 1 : 0));
			break;

		case IS_NULL:
			xdebug_xml_add_attribute(node, "type", "null");
			break;

		case IS_LONG:
			xdebug_xml_add_attribute(node, "type", "int");
			add_unencoded_text_value_attribute_or_element(options, node, xdebug_sprintf(XDEBUG_INT_FMT, Z_LVAL_P(*struc)));
			break;

		case IS_DOUBLE:
			xdebug_xml_add_attribute(node, "type", "float");
			add_unencoded_text_value_attribute_or_element(options, node, xdebug_sprintf("%.*H", (int) EG(precision), Z_DVAL_P(*struc)));
			break;

		case IS_STRING:
			xdebug_xml_add_attribute(node, "type", "string");
			if (options->max_data == 0 || (size_t) Z_STRLEN_P(*struc) <= (size_t) options->max_data) {
				add_encoded_text_value_attribute_or_element(options, node, xdstrndup(Z_STRVAL_P(*struc), Z_STRLEN_P(*struc)), Z_STRLEN_P(*struc));
			} else {
				add_encoded_text_value_attribute_or_element(options, node, xdstrndup(Z_STRVAL_P(*struc), options->max_data), options->max_data);
			}
			xdebug_xml_add_attribute_ex(node, "size", xdebug_sprintf("%d", Z_STRLEN_P(*struc)), 0, 1);
			break;

		case IS_ARRAY:
			myht = Z_ARRVAL_P(*struc);
			xdebug_xml_add_attribute(node, "type", "array");
			xdebug_xml_add_attribute(node, "children", myht->nNumOfElements > 0?"1":"0");

			if (!xdebug_zend_hash_is_recursive(myht)) {
				xdebug_xml_add_attribute_ex(node, "numchildren", xdebug_sprintf("%d", myht->nNumOfElements), 0, 1);
				if (level < options->max_depth) {
					xdebug_xml_add_attribute_ex(node, "page", xdebug_sprintf("%d", options->runtime[level].page), 0, 1);
					xdebug_xml_add_attribute_ex(node, "pagesize", xdebug_sprintf("%d", options->max_children), 0, 1);
					options->runtime[level].current_element_nr = 0;
					if (level == 0) {
						options->runtime[level].start_element_nr = options->max_children * options->runtime[level].page;
						options->runtime[level].end_element_nr = options->max_children * (options->runtime[level].page + 1);
					} else {
						options->runtime[level].start_element_nr = 0;
						options->runtime[level].end_element_nr = options->max_children;
					}

					xdebug_zend_hash_apply_protection_begin(myht);

					ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, z_val) {
						xdebug_array_element_export_xml_node(z_val, num, key, level, node, name, options);
					} ZEND_HASH_FOREACH_END();

					xdebug_zend_hash_apply_protection_end(myht);
				}
			} else {
				xdebug_xml_add_attribute(node, "recursive", "1");
			}
			break;

		case IS_OBJECT: {
			HashTable          *merged_hash;
			zend_string        *class_name;
			zend_class_entry   *ce;
			int                 extra_children = 0;
			zend_property_info *zpi_val;

			ALLOC_HASHTABLE(merged_hash);
			zend_hash_init(merged_hash, 128, NULL, merged_hash_object_item_dtor, 0);

			class_name = Z_OBJCE_P(*struc)->name;
			ce = zend_fetch_class(class_name, ZEND_FETCH_CLASS_DEFAULT);

			/* Adding static properties */
			xdebug_zend_hash_apply_protection_begin(&ce->properties_info);

#if PHP_VERSION_ID >= 80100
			zend_class_init_statics(ce);
#else
			if (ce->type == ZEND_INTERNAL_CLASS || (ce->ce_flags & ZEND_ACC_IMMUTABLE)) {
				zend_class_init_statics(ce);
			}
#endif

			ZEND_HASH_FOREACH_PTR(&ce->properties_info, zpi_val) {
				object_item_add_zend_prop_to_merged_hash(zpi_val, merged_hash, (int) XDEBUG_OBJECT_ITEM_TYPE_STATIC_PROPERTY, ce);
			} ZEND_HASH_FOREACH_END();

			xdebug_zend_hash_apply_protection_end(&ce->properties_info);

			/* Adding normal properties */
			myht = xdebug_objdebug_pp(struc, XDEBUG_VAR_OBJDEBUG_DEFAULT);

			if (myht) {
				zval *tmp_val;

				xdebug_zend_hash_apply_protection_begin(myht);

				ZEND_HASH_FOREACH_KEY_VAL_IND(myht, num, key, tmp_val) {
					int flags = XDEBUG_OBJECT_ITEM_TYPE_PROPERTY;

#if PHP_VERSION_ID >= 80100
					if (ce->type != ZEND_INTERNAL_CLASS && !HASH_KEY_IS_NUMERIC(key)) {
						const char         *cls_name, *tmp_prop_name;
						size_t              tmp_prop_name_len;
						zend_string        *unmangled;
						zend_property_info *info;

						zend_unmangle_property_name_ex(key, &cls_name, &tmp_prop_name, &tmp_prop_name_len);
						unmangled = zend_string_init_interned(tmp_prop_name, tmp_prop_name_len, 0);

						info = zend_get_property_info(Z_OBJCE_P(*struc), unmangled, 1);

						zend_string_release(unmangled);

						if (info && info != ZEND_WRONG_PROPERTY_INFO && info->flags & ZEND_ACC_READONLY) {
							flags |= XDEBUG_OBJECT_ITEM_TYPE_READONLY;
						}
					}
#endif
					object_item_add_to_merged_hash(tmp_val, num, key, merged_hash, flags);
				} ZEND_HASH_FOREACH_END();

				xdebug_zend_hash_apply_protection_end(myht);
			}

			xdebug_xml_add_attribute(node, "type", "object");

#if PHP_VERSION_ID >= 80100 // Enums
			{
				zend_class_entry *ce = Z_OBJCE_P(*struc);
				if (ce->ce_flags & ZEND_ACC_ENUM) {
					xdebug_xml_expand_attribute_value(node, "facet", "enum");
				}
			}
#endif

			if (instanceof_function(Z_OBJCE_P(*struc), zend_ce_closure)) {
#if PHP_VERSION_ID < 80200
				xdebug_xml_node *closure_cont, *closure_func;
				const zend_function *closure_function = zend_get_closure_method_def(Z_OBJ_P(*struc));

				closure_cont = xdebug_xml_node_init("property");
				xdebug_xml_add_attribute(closure_cont, "facet", "virtual readonly");
				xdebug_xml_add_attribute(closure_cont, "name", "{closure}");
				xdebug_xml_add_attribute(closure_cont, "type", "array");
				xdebug_xml_add_attribute(closure_cont, "children", "1");
				xdebug_xml_add_attribute(closure_cont, "page", "0");
				xdebug_xml_add_attribute(closure_cont, "pagesize", "2");

				if (closure_function->common.scope) {
					xdebug_xml_node *closure_scope = xdebug_xml_node_init("property");
					xdebug_xml_add_attribute(closure_scope, "facet", "readonly");
					xdebug_xml_add_attribute(closure_scope, "name", "scope");
					xdebug_xml_add_attribute(closure_scope, "type", "string");

					if (closure_function->common.fn_flags & ZEND_ACC_STATIC) {
						xdebug_xml_add_text_ex(
							closure_scope,
							ZSTR_VAL(closure_function->common.scope->name),
							ZSTR_LEN(closure_function->common.scope->name),
							0, 0
						);
					} else {
						xdebug_xml_add_text_ex(closure_scope, (char*) "$this", 6, 0, 0);
					}

					xdebug_xml_add_child(closure_cont, closure_scope);
					xdebug_xml_add_attribute(closure_cont, "numchildren", "2");
				} else {
					xdebug_xml_add_attribute(closure_cont, "numchildren", "1");
				}

				closure_func = xdebug_xml_node_init("property");
				xdebug_xml_add_attribute(closure_func, "facet", "readonly");
				xdebug_xml_add_attribute(closure_func, "name", "function");
				xdebug_xml_add_attribute(closure_func, "type", "string");
				xdebug_xml_add_text_ex(
					closure_func,
					ZSTR_VAL(closure_function->common.function_name),
					ZSTR_LEN(closure_function->common.function_name),
					0, 0
				);
				xdebug_xml_add_child(closure_cont, closure_func);

				xdebug_xml_add_child(node, closure_cont);
				extra_children = 1;
#endif

				xdebug_xml_expand_attribute_value(node, "facet", "closure");
			}

			{
				xdebug_str tmp_str;
				tmp_str.d = ZSTR_VAL(class_name);
				tmp_str.l = ZSTR_LEN(class_name);
				add_xml_attribute_or_element(options, node, "classname", 9, &tmp_str);
			}
			xdebug_xml_add_attribute(node, "children", (merged_hash->nNumOfElements || extra_children) ? "1" : "0");


			if (!myht || !xdebug_zend_hash_is_recursive(myht)) {
				xdebug_xml_add_attribute_ex(
					node, "numchildren",
					xdebug_sprintf("%d", zend_hash_num_elements(merged_hash) + extra_children),
					0, 1
				);
				if (level < options->max_depth) {
					xdebug_xml_add_attribute_ex(node, "page", xdebug_sprintf("%d", options->runtime[level].page), 0, 1);
					xdebug_xml_add_attribute_ex(node, "pagesize", xdebug_sprintf("%d", options->max_children), 0, 1);
					options->runtime[level].current_element_nr = 0;
					if (level == 0) {
						options->runtime[level].start_element_nr = options->max_children * options->runtime[level].page;
						options->runtime[level].end_element_nr = options->max_children * (options->runtime[level].page + 1);
					} else {
						options->runtime[level].start_element_nr = 0;
						options->runtime[level].end_element_nr = options->max_children;
					}

					xdebug_zend_hash_apply_protection_begin(merged_hash);

					ZEND_HASH_FOREACH_KEY_PTR(merged_hash, num, key, xoi_val) {
						xdebug_object_element_export_xml_node(xoi_val, level, node, name, options, ZSTR_VAL(class_name));
					} ZEND_HASH_FOREACH_END();

					xdebug_zend_hash_apply_protection_end(merged_hash);
				}
			}

			zend_hash_destroy(merged_hash);
			FREE_HASHTABLE(merged_hash);
			zend_release_properties(myht);

			break;
		}

		case IS_RESOURCE: {
			char *type_name;

			xdebug_xml_add_attribute(node, "type", "resource");
			type_name = (char *) zend_rsrc_list_get_rsrc_type(Z_RES_P(*struc));
			xdebug_xml_add_text(node, xdebug_sprintf("resource id='%ld' type='%s'", Z_RES_P(*struc)->handle, type_name ? type_name : "Unknown"));
			break;
		}

		case IS_UNDEF:
			xdebug_xml_add_attribute(node, "type", "uninitialized");
			break;

		default:
			xdebug_xml_add_attribute(node, "type", "null");
			break;
	}
}

xdebug_xml_node* xdebug_get_zval_value_xml_node_ex(xdebug_str *name, zval *val, int var_type, xdebug_var_export_options *options)
{
	xdebug_xml_node *node;
	xdebug_str      *short_name = NULL;
	xdebug_str      *full_name = NULL;

	node = xdebug_xml_node_init("property");
	options->encode_as_extended_property = 0;

	if (name) {
		switch (var_type) {
			case XDEBUG_VAR_TYPE_NORMAL: {
				short_name = prepare_variable_name(name);
				full_name =  xdebug_str_copy(short_name);
			} break;

			case XDEBUG_VAR_TYPE_STATIC: {
				xdebug_str tmp_formatted_name = XDEBUG_STR_INITIALIZER;

				xdebug_str_add_literal(&tmp_formatted_name, "::");
				xdebug_str_add_str(&tmp_formatted_name, name);

				short_name = xdebug_str_copy(&tmp_formatted_name);
				full_name =  xdebug_str_copy(&tmp_formatted_name);

				xdebug_str_destroy(&tmp_formatted_name);
			} break;

			case XDEBUG_VAR_TYPE_CONSTANT:
				short_name = xdebug_str_copy(name);
				full_name =  xdebug_str_copy(name);
				break;
		}

		check_if_extended_properties_are_needed(options, short_name, full_name, val);
		add_xml_attribute_or_element(options, node, "name", 4, short_name);
		add_xml_attribute_or_element(options, node, "fullname", 8, full_name);
	}
	xdebug_var_export_xml_node(&val, full_name ? full_name : NULL, node, options, 0);

	if (short_name) {
		xdebug_str_free(short_name);
	}
	if (full_name) {
		xdebug_str_free(full_name);
	}

	return node;
}
