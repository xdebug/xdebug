#include "CppUTest/TestHarness.h"

#include "maps_private.h"
#include "maps.h"
#include "parser.h"

TEST_GROUP(path_maps_file)
{
	struct xdebug_path_maps *test_map;
	bool result;
	int   error_code;
	char *error_message;
	size_t mapping_count;

	TEST_SETUP()
	{
		test_map = xdebug_path_maps_ctor();
		result = false;
		error_code = PATH_MAPS_OK;
		error_message = NULL;
	}

	TEST_TEARDOWN()
	{
		if (error_message) {
			free(error_message);
		}

		xdebug_path_maps_dtor(test_map);
	}
};

TEST(path_maps_file, fopen_non_existing)
{
	result = xdebug_path_maps_parse_file(test_map, "file-does-not-exist.map", &error_code, &error_message);

	LONGS_EQUAL(false, result);
	LONGS_EQUAL(PATH_MAPS_CANT_OPEN_FILE, error_code);
	STRCMP_EQUAL("Can't open file", error_message);
};

TEST(path_maps_file, fopen_existing)
{
	result = xdebug_path_maps_parse_file(test_map, "files/simple.map", &error_code, &error_message);

	LONGS_EQUAL(true, result);
	LONGS_EQUAL(PATH_MAPS_OK, error_code);
};

TEST(path_maps_file, empty)
{
	result = xdebug_path_maps_parse_file(test_map, "files/empty.map", &error_code, &error_message);

	LONGS_EQUAL(false, result);
	LONGS_EQUAL(PATH_MAPS_NO_RULES, error_code);
	STRCMP_EQUAL("The map file did not provide any mappings", error_message);
};

TEST(path_maps_file, no_rules)
{
	result = xdebug_path_maps_parse_file(test_map, "files/no-rules.map", &error_code, &error_message);

	LONGS_EQUAL(false, result);
	LONGS_EQUAL(PATH_MAPS_NO_RULES, error_code);
	STRCMP_EQUAL("The map file did not provide any mappings", error_message);
};

TEST(path_maps_file, full_path_map)
{
	result = xdebug_path_maps_parse_file(test_map, "files/simple.map", &error_code, &error_message);

	LONGS_EQUAL(true, result);
	LONGS_EQUAL(PATH_MAPS_OK, error_code);
	LONGS_EQUAL(1, xdebug_path_maps_get_rule_count(test_map));
};

TEST(path_maps_file, check_rules)
{
	xdebug_path_mapping *mapping = NULL;
	result = xdebug_path_maps_parse_file(test_map, "files/simple.map", &error_code, &error_message);

	LONGS_EQUAL(true, result);
	LONGS_EQUAL(PATH_MAPS_OK, error_code);
	LONGS_EQUAL(1, xdebug_path_maps_get_rule_count(test_map));

	mapping = remote_to_local(test_map, "/var/www/");
	LONGS_EQUAL(XDEBUG_PATH_MAP_TYPE_DIRECTORY, mapping->type);
	STRCMP_EQUAL("/var/www/", mapping->remote_path);
	STRCMP_EQUAL("/home/derick/projects/example.com/", mapping->local_path);
};

TEST(path_maps_file, check_rules_with_prefix_1)
{
	xdebug_path_mapping *mapping = NULL;
	result = xdebug_path_maps_parse_file(test_map, "files/with-prefix.map", &error_code, &error_message);

	LONGS_EQUAL(true, result);
	LONGS_EQUAL(PATH_MAPS_OK, error_code);
	LONGS_EQUAL(1, xdebug_path_maps_get_rule_count(test_map));

	mapping = remote_to_local(test_map, "/var/www/");
	CHECK(mapping);
	LONGS_EQUAL(XDEBUG_PATH_MAP_TYPE_DIRECTORY, mapping->type);
	STRCMP_EQUAL("/var/www/", mapping->remote_path);
	STRCMP_EQUAL("/home/derick/projects/example.com/", mapping->local_path);
};
