#include "CppUTest/TestHarness.h"

#include <unistd.h>

#include "maps_private.h"
#include "parser.h"

TEST_GROUP(fuzz_cases)
{
	struct xdebug_path_maps *test_map;
	bool result;
	int   error_code;
	int   error_line;
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
		error_line = -1;
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

		return xdebug_path_maps_parse_file(test_map, filename, &error_code, &error_line, &error_message);
	}

	void check_result(size_t expected_error_code, int expected_error_line, const char *expected_error_message)
	{
		STRCMP_EQUAL(expected_error_message, error_message);
		LONGS_EQUAL(expected_error_code, error_code);
		LONGS_EQUAL(expected_error_line, error_line);
		LONGS_EQUAL(expected_error_code == PATH_MAPS_OK ? true : false, result);
	}

	void check_map(size_t type, const char *local_path)
	{
		CHECK(mapping);
		LONGS_EQUAL(type, mapping->type);
		STRCMP_EQUAL(local_path, XDEBUG_STR_VAL(mapping->local_path));
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

TEST(fuzz_cases, line_starts_with_equals)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
=/example.php:42-5 = /example.php
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_GARBAGE, 4, "Line starts with a '='");
};

TEST(fuzz_cases, local_part_is_emtpy)
{
	const char *map = R""""(
remote_prefix: /local/www
local_prefix: /hom/project
/projecle.php:5-17 =
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_GARBAGE, 4, "Local part is empty");
};

TEST(fuzz_cases, single_char_remote_part)
{
	const char *map = R""""(
remote_prefix: /local/www
l= /exp:20
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_MISMATCHED_TYPES, 3, "Remote mapping part ('/local/wwwl') type (file) must match local mapping part ('/exp') type (line-range)");
};

TEST(fuzz_cases, remote_part_is_emtpy_after_trim)
{
	const char *map = R""""(
remote_prefix: /local/www
local_prefix: /hom/project
/projecle.php:5-17 = /example.php:8
 = /example.php:8
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_GARBAGE, 5, "Remote part is empty");
};
