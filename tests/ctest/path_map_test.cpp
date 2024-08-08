#include "CppUTest/TestHarness.h"

#include <unistd.h>

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
	FILE *filep;
	xdebug_path_mapping *mapping;
	char *filename;

	TEST_SETUP()
	{
		test_map = xdebug_path_maps_ctor();
		result = false;
		error_code = PATH_MAPS_OK;
		error_message = NULL;
		filename = NULL;
		filep = NULL;
		mapping = NULL;
	}

	bool test_map_from_file(const char *data_string)
	{
		char templ[] = "/tmp/xdct.XXXXXX";

		int fp = mkstemp(templ);

		filep = fdopen(fp, "w");
		fputs(data_string, filep);
		fclose(filep);
		filep = NULL;

		filename = strdup(templ);

		return xdebug_path_maps_parse_file(test_map, filename, &error_code, &error_message);
	}

	void check_result(size_t expected_error_code, size_t result_count)
	{
		LONGS_EQUAL(expected_error_code == PATH_MAPS_OK ? true : false, result);
		LONGS_EQUAL(expected_error_code, error_code);
		LONGS_EQUAL(result_count, xdebug_path_maps_get_rule_count(test_map));
	}

	void check_map(size_t type, const char *remote_path, const char *local_path)
	{
		CHECK(mapping);
		LONGS_EQUAL(type, mapping->type);
		STRCMP_EQUAL(remote_path, mapping->remote_path);
		STRCMP_EQUAL(local_path, mapping->local_path);
	}

	TEST_TEARDOWN()
	{
		if (error_message) {
			free(error_message);
		}

		if (filename) {
			unlink(filename);
			free(filename);
			filename = NULL;
		}
		if (filep) {
			fclose(filep);
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

TEST(path_maps_file, no_trailing_newline)
{
	const char *map = R""""(/var/www/ = /home/derick/projects/example.com/)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_NO_NEWLINE, 0);
	STRCMP_EQUAL("Line XXX does not end in a new line", error_message);
};

TEST(path_maps_file, empty)
{
	const char *map = R""""()"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_NO_RULES, 0);
};

TEST(path_maps_file, no_rules)
{
	const char *map = R""""(
remote_prefix: /var/www/
local_prefix: /home/derick/projects/example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_NO_RULES, 0);
	LONGS_EQUAL(false, result);
	STRCMP_EQUAL("The map file did not provide any mappings", error_message);
};

TEST(path_maps_file, full_path_map)
{
	const char *map = R""""(
/var/www/ = /home/derick/projects/example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, 1);
};

TEST(path_maps_file, check_rules)
{
	const char *map = R""""(
/var/www/ = /home/derick/projects/example.com/
)"""";

	result = test_map_from_file(map);
	check_result(error_code, 1);

	mapping = remote_to_local(test_map, "/var/www/");

	check_map(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/var/www/", "/home/derick/projects/example.com/");
};

TEST(path_maps_file, check_rules_with_prefix_1)
{
	const char *map = R""""(
remote_prefix: /var
local_prefix: /home/derick/projects
/www/ = /example.com/
)"""";

	result = test_map_from_file(map);
	check_result(error_code, 1);

	mapping = remote_to_local(test_map, "/var/www/");

	check_map(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/var/www/", "/home/derick/projects/example.com/");
};
