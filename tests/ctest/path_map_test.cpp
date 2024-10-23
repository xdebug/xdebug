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
	int   error_line;
	char *error_message;
	size_t mapping_count;
	FILE *filep;

	xdebug_str *local_path;
	size_t      local_line;
	int         mapping_type;

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
		mapping_type = -1;
		local_path = NULL;
		local_line = -1;
	}

	bool test_map_from_file(const char *data_string)
	{
		bool retval = false;
		char templ[] = "/tmp/xdct.XXXXXX";

		int fp = mkstemp(templ);

		filep = fdopen(fp, "w");
		fputs(data_string, filep);
		fclose(filep);
		filep = NULL;

		filename = strdup(templ);

		retval = xdebug_path_maps_parse_file(test_map, filename, &error_code, &error_line, &error_message);

		unlink(filename);
		free(filename);
		filename = NULL;

		return retval;
	}

	void check_result(size_t expected_error_code, int expected_error_line, const char *expected_error_message)
	{
		STRCMP_EQUAL(expected_error_message, error_message);
		LONGS_EQUAL(expected_error_code, error_code);
		LONGS_EQUAL(expected_error_line, error_line);
		LONGS_EQUAL(expected_error_code == PATH_MAPS_OK ? true : false, result);
	}

	void check_map(size_t expected_type, const char *expected_local_path)
	{
		CHECK(mapping_type != XDEBUG_PATH_MAP_TYPE_UNKNOWN);
		LONGS_EQUAL(expected_type, mapping_type);
		STRCMP_EQUAL(expected_local_path, XDEBUG_STR_VAL(local_path));
	}

	void test_remote_to_local(const char *remote_path, size_t remote_line)
	{
		reset_result();
		mapping_type = remote_to_local(test_map, remote_path, remote_line, &local_path, &local_line);
	}

	void check_map_with_range(size_t expected_type, const char *expected_local_path, size_t expected_local_line)
	{
		check_map(expected_type, expected_local_path);
		LONGS_EQUAL(expected_local_line, local_line);
	}

	TEST_TEARDOWN()
	{
		if (error_message) {
			free(error_message);
		}
		reset_result();

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

private:
	void reset_result()
	{
		if (local_path) {
			xdebug_str_free(local_path);
			local_path = NULL;
		}
	}
};

TEST(path_maps_file, fopen_non_existing)
{
	result = xdebug_path_maps_parse_file(test_map, "file-does-not-exist.map", &error_code, &error_line, &error_message);

	LONGS_EQUAL(false, result);
	LONGS_EQUAL(PATH_MAPS_CANT_OPEN_FILE, error_code);
	STRCMP_EQUAL("Can't open file", error_message);
};

TEST(path_maps_file, no_trailing_newline)
{
	const char *map = R""""(/var/www/ = /home/derick/projects/example.com/)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_NO_NEWLINE, 1, "Line does not end in a new line");
};

TEST(path_maps_file, only_new_line)
{
	const char *map = R""""(
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_NO_RULES, 1, "The map file did not provide any mappings");
};

TEST(path_maps_file, comment_no_rules)
{
	const char *map = R""""(# This is the first line of a comment
# This is the second line of a comment
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_NO_RULES, 2, "The map file did not provide any mappings");
};

TEST(path_maps_file, empty)
{
	const char *map = R""""()"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_NO_RULES, 0, "The map file did not provide any mappings");
};

TEST(path_maps_file, no_rules)
{
	const char *map = R""""(
remote_prefix: /var/www/
local_prefix: /home/derick/projects/example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_NO_RULES, 3, "The map file did not provide any mappings");
};

TEST(path_maps_file, full_path_map)
{
	const char *map = R""""(
/var/www/ = /home/derick/projects/example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);
};

TEST(path_maps_file, check_rules)
{
	const char *map = R""""(
/var/www/ = /home/derick/projects/example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/var/www/", 1);

	check_map(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/home/derick/projects/example.com/");
};

TEST(path_maps_file, check_rule_with_comment)
{
	const char *map = R""""(
# We map our remote path to our local projects directory
/var/www/ = /home/derick/projects/example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/var/www/", 1);

	check_map(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/home/derick/projects/example.com/");
};

TEST(path_maps_file, check_rule_with_odd_spaces)
{
	const char *map = R""""(
# We map our remote path to our local projects directory
	/var/www/	=	/home/derick/projects/example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/var/www/", 1);

	check_map(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/home/derick/projects/example.com/");
};

TEST(path_maps_file, check_rules_with_prefix_1)
{
	const char *map = R""""(
remote_prefix: /var
local_prefix: /home/derick/projects
/www/ = /example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/var/www/", 1);

	check_map(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/home/derick/projects/example.com/");
};

TEST(path_maps_file, empty_remote_prefix)
{
	const char *map = R""""(
remote_prefix:
local_prefix: /usr/local/www
/www/ = /example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_INVALID_PREFIX, 2, "Prefix is empty");
}

TEST(path_maps_file, empty_local_prefix)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix:
/www/ = /example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_INVALID_PREFIX, 3, "Prefix is empty");
}

TEST(path_maps_file, non_absolute_remote_prefix)
{
	const char *map = R""""(
remote_prefix: var
local_prefix: /usr/local/www
/www/ = /example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_INVALID_PREFIX, 2, "Prefix is not an absolute path: 'var'");
}

TEST(path_maps_file, non_absolute_local_prefix)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix:home/derick/projects
/www/ = /example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_INVALID_PREFIX, 3, "Prefix is not an absolute path: 'home/derick/projects'");
}

TEST(path_maps_file, check_multiple_rules_with_prefix_1)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/projects
/servers/example.com/ = /example.com/
/servers/example.net/ = /example.net/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/servers/example.com/", 1);
	check_map(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/home/derick/projects/example.com/");
}

TEST(path_maps_file, check_multiple_rules_with_prefix_2)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/projects
/servers/example.com/ = /example.com/
/servers/example.net/ = /example.net/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);


	test_remote_to_local("/usr/local/www/servers/example.net/", 1);
	check_map(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/home/derick/projects/example.net/");
};

TEST(path_maps_file, check_multiple_rules_with_prefix_and_file)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/projects
/servers/example.com/ = /example.com/
/servers/example.net/ = /example.net/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/servers/example.net/my-script.php", 1);
	check_map(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/home/derick/projects/example.net/my-script.php");
};

TEST(path_maps_file, check_multiple_rules_with_prefix_and_file_in_subdirectory)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/projects
/servers/example.com/ = /example.com/
/servers/example.net/ = /example.net/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/servers/example.net/public/router.php", 1);
	check_map(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/home/derick/projects/example.net/public/router.php");

	test_remote_to_local("/usr/local/www/servers/example.net/public/index.php", 1);
	check_map_with_range(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/home/derick/projects/example.net/public/index.php", 1);

	test_remote_to_local("/usr/local/www/servers/public/index.php", 1);
	CHECK_EQUAL(XDEBUG_PATH_MAP_TYPE_UNKNOWN, mapping_type);

	test_remote_to_local("/local/www/servers/public/index.php", 1);
	CHECK_EQUAL(XDEBUG_PATH_MAP_TYPE_UNKNOWN, mapping_type);

	test_remote_to_local("/usr/local/www/servers/example.net/public/index.php", 8051);
	check_map_with_range(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/home/derick/projects/example.net/public/index.php", 8051);
};

TEST(path_maps_file, no_double_separator_remote_prefix)
{
	const char *map = R""""(
remote_prefix: /usr/local/www/
local_prefix: /home/derick/projects
/servers/example.com/ = /example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_DOUBLE_SEPARATOR, 4, "Remote prefix ends with separator ('/usr/local/www/') and mapping line begins with separator ('/servers/example.com/')");
};

TEST(path_maps_file, no_double_separator_local_prefix)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/projects/
/servers/example.com/ = /example.com/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_DOUBLE_SEPARATOR, 4, "Local prefix ends with separator ('/home/derick/projects/') and mapping line begins with separator ('/example.com/')");
};

TEST(path_maps_file, check_local_matches_remote_mapping_type_dir_file)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/projects
/servers/example.com/ = /example.com/
/servers/example.net/ = /example.net
/servers/example.org/ = /example.org/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_MISMATCHED_TYPES, 5, "Remote mapping part ('/usr/local/www/servers/example.net/') type (directory) must match local mapping part ('/home/derick/projects/example.net') type (file)");
};

TEST(path_maps_file, check_local_matches_remote_mapping_type_file_dir)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/projects
/servers/example.com/ = /example.com/
/servers/example.net = /example.net/
/servers/example.org/ = /example.org/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_MISMATCHED_TYPES, 5, "Remote mapping part ('/usr/local/www/servers/example.net') type (file) must match local mapping part ('/home/derick/projects/example.net/') type (directory)");
};

TEST(path_maps_file, remote_part_unknown_type)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/projects
/servers/example.org? = /example.org/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_MISMATCHED_TYPES, 4, "Remote mapping part ('/usr/local/www/servers/example.org?') type (file) must match local mapping part ('/home/derick/projects/example.org/') type (directory)");
};

TEST(path_maps_file, local_part_unknown_type)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/projects
/servers/example.org/ = /example.org?
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_MISMATCHED_TYPES, 4, "Remote mapping part ('/usr/local/www/servers/example.org/') type (directory) must match local mapping part ('/home/derick/projects/example.org?') type (file)");
};

TEST(path_maps_file, check_type_file)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php = /example.php
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/example.php", 1);
	check_map(XDEBUG_PATH_MAP_TYPE_FILE, "/home/derick/project/example.php");
};

TEST(path_maps_file, remote_path_wrong_start_range_number)
{
	const char *map = R""""(
/example.php:53x = /example.php
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 2, "Remote element: Non-number found as range: ':53x'");
};

TEST(path_maps_file, local_path_wrong_start_range_number)
{
	const char *map = R""""(
/example.php = /example.php:72c
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 2, "Local element: Non-number found as range: ':72c'");
};

TEST(path_maps_file, remote_path_only_range_number)
{
	const char *map = R""""(
:53x = /example.php
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 2, "Remote element: Element only contains a range, but no path: ':53x'");
};

TEST(path_maps_file, local_path_only_range_number)
{
	const char *map = R""""(
/example.php = :72c
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 2, "Local element: Element only contains a range, but no path: ':72c'");
};

TEST(path_maps_file, remote_path_with_directory)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/examples/:53x = /examples/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "Remote element: Ranges are not supported with directories: '/examples/:53x'");
};

TEST(path_maps_file, local_path_with_directory)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/examples/ = /examples/:72c
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "Local element: Ranges are not supported with directories: '/examples/:72c'");
};

TEST(path_maps_file, remote_single_range_number)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:1 = /example.php:42
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/example.php", 1);
	check_map_with_range(XDEBUG_PATH_MAP_TYPE_LINES, "/home/derick/project/example.php", 42);
};

TEST(path_maps_file, remote_range_less_than_one)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:0 = /example.php
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "Remote element: Line number much be larger than 0: '/example.php:0'");
};

TEST(path_maps_file, local_range_less_than_one)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php = /example.php:0
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "Local element: Line number much be larger than 0: '/example.php:0'");
};

TEST(path_maps_file, remote_range_empty_start)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:-42 = /example.php
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "Remote element: The starting line number must be provided: '/example.php:-42'");
};

TEST(path_maps_file, local_range_empty_start)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:7 = /example.php:-8
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "Local element: The starting line number must be provided: '/example.php:-8'");
};

TEST(path_maps_file, remote_range_wrong_begin_lineno)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:6x-42 = /example.php
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "Remote element: Non-number found as begin range: ':6x-42'");
};

TEST(path_maps_file, local_range_wrong_begin_lineno)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php = /example.php:7y-43
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "Local element: Non-number found as begin range: ':7y-43'");
};

TEST(path_maps_file, remote_range_wrong_end_lineno)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:6-42x = /example.php
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "Remote element: Non-number found as end range: ':6-42x'");
};

TEST(path_maps_file, local_range_wrong_end_lineno)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php = /example.php:7-43y
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "Local element: Non-number found as end range: ':7-43y'");
};

TEST(path_maps_file, remote_range_wrong_range_order)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:42-5 = /example.php
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "Remote element: End of range (42) is before start of range (5): ':42-5'");
};

TEST(path_maps_file, local_range_wrong_range_order)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php = /example.php:75-8
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "Local element: End of range (75) is before start of range (8): ':75-8'");
};

TEST(path_maps_file, range_span_mismatch)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:2-5 = /example.php:2-8
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 4, "The remote range span (2-5) needs to have the same difference (3) as the local range span (2-8) difference (6)");
};

TEST(path_maps_file, range_span_single)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:5 = /example.php:8
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/example.php", 5);
	check_map_with_range(XDEBUG_PATH_MAP_TYPE_LINES, "/home/derick/project/example.php", 8);
};

TEST(path_maps_file, range_span_n_to_1)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:5-17 = /example.php:8
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/example.php", 8);
	check_map_with_range(XDEBUG_PATH_MAP_TYPE_LINES, "/home/derick/project/example.php", 8);
};

TEST(path_maps_file, range_span_n_to_m)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:5-17 = /example.php:8-20
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/example.php", 14);
	check_map_with_range(XDEBUG_PATH_MAP_TYPE_LINES, "/home/derick/project/example.php", 17);
};

TEST(path_maps_file, multiple_ranges_one_file_1)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:5-17 = /example.php:8-20
/example.php:18 = /example.php:21
/example.php:19-33 = /example.php:24
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/example.php", 6);
	check_map_with_range(XDEBUG_PATH_MAP_TYPE_LINES, "/home/derick/project/example.php", 9);
};

TEST(path_maps_file, multiple_ranges_one_file_2)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:5-17 = /example.php:8-20
/example.php:18 = /example.php:21
/example.php:19-33 = /example.php:24
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/example.php", 18);
	check_map_with_range(XDEBUG_PATH_MAP_TYPE_LINES, "/home/derick/project/example.php", 21);
};

TEST(path_maps_file, multiple_ranges_one_file_3)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:5-17 = /example.php:8-20
/example.php:18 = /example.php:21
/example.php:19-33 = /example.php:24
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/example.php", 32);
	check_map_with_range(XDEBUG_PATH_MAP_TYPE_LINES, "/home/derick/project/example.php", 24);

	test_remote_to_local("/usr/local/www/example.php", 33);
	check_map_with_range(XDEBUG_PATH_MAP_TYPE_LINES, "/home/derick/project/example.php", 24);
};

TEST(path_maps_file, multiple_ranges_outside_ranges)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:5-17 = /example.php:8-20
/example.php:18 = /example.php:21
/example.php:19-33 = /example.php:24
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/example.php", 42);
	CHECK_EQUAL(XDEBUG_PATH_MAP_TYPE_UNKNOWN, mapping_type);
};

TEST(path_maps_file, multiple_ranges_with_benchmark)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:1 = /example.php:7
/example.php:2 = /example.php:8
/example.php:3 = /example.php:9
/example.php:4 = /example.php:10
/example.php:5 = /example.php:11
/example.php:6 = /example.php:12
/example.php:7 = /example.php:13
/example.php:8 = /example.php:14
/example.php:9 = /example.php:15
/example.php:10 = /example.php:16
/example.php:11 = /example.php:17
/example.php:12 = /example.php:18
/example.php:13 = /example.php:19
/example.php:14 = /example.php:20
/example.php:15 = /example.php:21
/example.php:16 = /example.php:22
/example.php:17 = /example.php:23
/example.php:18 = /example.php:24
/example.php:19 = /example.php:25
/example.php:20 = /example.php:26
/example.php:21 = /example.php:27
/example.php:22 = /example.php:28
/example.php:23 = /example.php:29
/example.php:24 = /example.php:30
/example.php:25 = /example.php:31
/example.php:26 = /example.php:32
/example.php:27 = /example.php:33
/example.php:28 = /example.php:34
/example.php:29 = /example.php:35
/example.php:30 = /example.php:36
/example.php:31 = /example.php:37
/example.php:32 = /example.php:38
/example.php:33 = /example.php:39
/example.php:34 = /example.php:40
/example.php:35 = /example.php:41
/example.php:36 = /example.php:42
/example.php:37 = /example.php:43
/example.php:38 = /example.php:44
/example.php:39 = /example.php:45
/example.php:40 = /example.php:46
/example.php:41 = /example.php:47
/example.php:42 = /example.php:48
/example.php:43 = /example.php:49
/example.php:44 = /example.php:50
/example.php:45 = /example.php:51
/example.php:46 = /example.php:52
/example.php:47 = /example.php:53
/example.php:48 = /example.php:54
/example.php:49 = /example.php:55
/example.php:50 = /example.php:56
/example.php:51 = /example.php:57
/example.php:52 = /example.php:58
/example.php:53 = /example.php:59
/example.php:54 = /example.php:60
/example.php:55 = /example.php:61
/example.php:56 = /example.php:62
/example.php:57 = /example.php:63
/example.php:58 = /example.php:64
/example.php:59 = /example.php:65
/example.php:60 = /example.php:66
/example.php:61 = /example.php:67
/example.php:62 = /example.php:68
/example.php:63 = /example.php:69
/example.php:64 = /example.php:70
/example.php:65 = /example.php:71
/example.php:66 = /example.php:72
/example.php:67 = /example.php:73
/example.php:68 = /example.php:74
/example.php:69 = /example.php:75
/example.php:70 = /example.php:76
/example.php:71 = /example.php:77
/example.php:72 = /example.php:78
/example.php:73 = /example.php:79
/example.php:74 = /example.php:80
/example.php:75 = /example.php:81
/example.php:76 = /example.php:82
/example.php:77 = /example.php:83
/example.php:78 = /example.php:84
/example.php:79 = /example.php:85
/example.php:80 = /example.php:86
/example.php:81 = /example.php:87
/example.php:82 = /example.php:88
/example.php:83 = /example.php:89
/example.php:84 = /example.php:90
/example.php:85 = /example.php:91
/example.php:86 = /example.php:92
/example.php:87 = /example.php:93
/example.php:88 = /example.php:94
/example.php:89 = /example.php:95
/example.php:90 = /example.php:96
/example.php:91 = /example.php:97
/example.php:92 = /example.php:98
/example.php:93 = /example.php:99
/example.php:94 = /example.php:100
/example.php:95 = /example.php:101
/example.php:96 = /example.php:102
/example.php:97 = /example.php:103
/example.php:98 = /example.php:104
/example.php:99 = /example.php:105
/example.php:100 = /example.php:106
/example.php:101 = /example.php:107
/example.php:102 = /example.php:108
/example.php:103 = /example.php:109
/example.php:104 = /example.php:110
/example.php:105 = /example.php:111
/example.php:106 = /example.php:112
/example.php:107 = /example.php:113
/example.php:108 = /example.php:114
/example.php:109 = /example.php:115
/example.php:110 = /example.php:116
/example.php:111 = /example.php:117
/example.php:112 = /example.php:118
/example.php:113 = /example.php:119
/example.php:114 = /example.php:120
/example.php:115 = /example.php:121
/example.php:116 = /example.php:122
/example.php:117 = /example.php:123
/example.php:118 = /example.php:124
/example.php:119 = /example.php:125
/example.php:120 = /example.php:126
/example.php:121 = /example.php:127
/example.php:122 = /example.php:128
/example.php:123 = /example.php:129
/example.php:124 = /example.php:130
/example.php:125 = /example.php:131
/example.php:126 = /example.php:132
/example.php:127 = /example.php:133
/example.php:128 = /example.php:134
/example.php:129 = /example.php:135
/example.php:130 = /example.php:136
/example.php:131 = /example.php:137
/example.php:132 = /example.php:138
/example.php:133 = /example.php:139
/example.php:134 = /example.php:140
/example.php:135 = /example.php:141
/example.php:136 = /example.php:142
/example.php:137 = /example.php:143
/example.php:138 = /example.php:144
/example.php:139 = /example.php:145
/example.php:140 = /example.php:146
/example.php:141 = /example.php:147
/example.php:142 = /example.php:148
/example.php:143 = /example.php:149
/example.php:144 = /example.php:150
/example.php:145 = /example.php:151
/example.php:146 = /example.php:152
/example.php:147 = /example.php:153
/example.php:148 = /example.php:154
/example.php:149 = /example.php:155
/example.php:150 = /example.php:156
/example.php:151 = /example.php:157
/example.php:152 = /example.php:158
/example.php:153 = /example.php:159
/example.php:154 = /example.php:160
/example.php:155 = /example.php:161
/example.php:156 = /example.php:162
/example.php:157 = /example.php:163
/example.php:158 = /example.php:164
/example.php:159 = /example.php:165
/example.php:160 = /example.php:166
/example.php:161 = /example.php:167
/example.php:162 = /example.php:168
/example.php:163 = /example.php:169
/example.php:164 = /example.php:170
/example.php:165 = /example.php:171
/example.php:166 = /example.php:172
/example.php:167 = /example.php:173
/example.php:168 = /example.php:174
/example.php:169 = /example.php:175
/example.php:170 = /example.php:176
/example.php:171 = /example.php:177
/example.php:172 = /example.php:178
/example.php:173 = /example.php:179
/example.php:174 = /example.php:180
/example.php:175 = /example.php:181
/example.php:176 = /example.php:182
/example.php:177 = /example.php:183
/example.php:178 = /example.php:184
/example.php:179 = /example.php:185
/example.php:180 = /example.php:186
/example.php:181 = /example.php:187
/example.php:182 = /example.php:188
/example.php:183 = /example.php:189
/example.php:184 = /example.php:190
/example.php:185 = /example.php:191
/example.php:186 = /example.php:192
/example.php:187 = /example.php:193
/example.php:188 = /example.php:194
/example.php:189 = /example.php:195
/example.php:190 = /example.php:196
/example.php:191 = /example.php:197
/example.php:192 = /example.php:198
/example.php:193 = /example.php:199
/example.php:194 = /example.php:200
/example.php:195 = /example.php:201
/example.php:196 = /example.php:202
/example.php:197 = /example.php:203
/example.php:198 = /example.php:204
/example.php:199 = /example.php:205
/example.php:200 = /example.php:206
/example.php:201 = /example.php:207
/example.php:202 = /example.php:208
/example.php:203 = /example.php:209
/example.php:204 = /example.php:210
/example.php:205 = /example.php:211
/example.php:206 = /example.php:212
/example.php:207 = /example.php:213
/example.php:208 = /example.php:214
/example.php:209 = /example.php:215
/example.php:210 = /example.php:216
/example.php:211 = /example.php:217
/example.php:212 = /example.php:218
/example.php:213 = /example.php:219
/example.php:214 = /example.php:220
/example.php:215 = /example.php:221
/example.php:216 = /example.php:222
/example.php:217 = /example.php:223
/example.php:218 = /example.php:224
/example.php:219 = /example.php:225
/example.php:220 = /example.php:226
/example.php:221 = /example.php:227
/example.php:222 = /example.php:228
/example.php:223 = /example.php:229
/example.php:224 = /example.php:230
/example.php:225 = /example.php:231
/example.php:226 = /example.php:232
/example.php:227 = /example.php:233
/example.php:228 = /example.php:234
/example.php:229 = /example.php:235
/example.php:230 = /example.php:236
/example.php:231 = /example.php:237
/example.php:232 = /example.php:238
/example.php:233 = /example.php:239
/example.php:234 = /example.php:240
/example.php:235 = /example.php:241
/example.php:236 = /example.php:242
/example.php:237 = /example.php:243
/example.php:238 = /example.php:244
/example.php:239 = /example.php:245
/example.php:240 = /example.php:246
/example.php:241 = /example.php:247
/example.php:242 = /example.php:248
/example.php:243 = /example.php:249
/example.php:244 = /example.php:250
/example.php:245 = /example.php:251
/example.php:246 = /example.php:252
/example.php:247 = /example.php:253
/example.php:248 = /example.php:254
/example.php:249 = /example.php:255
/example.php:250 = /example.php:256
/example.php:251 = /example.php:257
/example.php:252 = /example.php:258
/example.php:253 = /example.php:259
/example.php:254 = /example.php:260
/example.php:255 = /example.php:261
/example.php:256 = /example.php:262
/example.php:257 = /example.php:263
/example.php:258 = /example.php:264
/example.php:259 = /example.php:265
/example.php:260 = /example.php:266
/example.php:261 = /example.php:267
/example.php:262 = /example.php:268
/example.php:263 = /example.php:269
/example.php:264 = /example.php:270
/example.php:265 = /example.php:271
/example.php:266 = /example.php:272
/example.php:267 = /example.php:273
/example.php:268 = /example.php:274
/example.php:269 = /example.php:275
/example.php:270 = /example.php:276
/example.php:271 = /example.php:277
/example.php:272 = /example.php:278
/example.php:273 = /example.php:279
/example.php:274 = /example.php:280
/example.php:275 = /example.php:281
/example.php:276 = /example.php:282
/example.php:277 = /example.php:283
/example.php:278 = /example.php:284
/example.php:279 = /example.php:285
/example.php:280 = /example.php:286
/example.php:281 = /example.php:287
/example.php:282 = /example.php:288
/example.php:283 = /example.php:289
/example.php:284 = /example.php:290
/example.php:285 = /example.php:291
/example.php:286 = /example.php:292
/example.php:287 = /example.php:293
/example.php:288 = /example.php:294
/example.php:289 = /example.php:295
/example.php:290 = /example.php:296
/example.php:291 = /example.php:297
/example.php:292 = /example.php:298
/example.php:293 = /example.php:299
/example.php:294 = /example.php:300
/example.php:295 = /example.php:301
/example.php:296 = /example.php:302
/example.php:297 = /example.php:303
/example.php:298 = /example.php:304
/example.php:299 = /example.php:305
/example.php:300 = /example.php:306
/example.php:301 = /example.php:307
/example.php:302 = /example.php:308
/example.php:303 = /example.php:309
/example.php:304 = /example.php:310
/example.php:305 = /example.php:311
/example.php:306 = /example.php:312
/example.php:307 = /example.php:313
/example.php:308 = /example.php:314
/example.php:309 = /example.php:315
/example.php:310 = /example.php:316
/example.php:311 = /example.php:317
/example.php:312 = /example.php:318
/example.php:313 = /example.php:319
/example.php:314 = /example.php:320
/example.php:315 = /example.php:321
/example.php:316 = /example.php:322
/example.php:317 = /example.php:323
/example.php:318 = /example.php:324
/example.php:319 = /example.php:325
/example.php:320 = /example.php:326
/example.php:321 = /example.php:327
/example.php:322 = /example.php:328
/example.php:323 = /example.php:329
/example.php:324 = /example.php:330
/example.php:325 = /example.php:331
/example.php:326 = /example.php:332
/example.php:327 = /example.php:333
/example.php:328 = /example.php:334
/example.php:329 = /example.php:335
/example.php:330 = /example.php:336
/example.php:331 = /example.php:337
/example.php:332 = /example.php:338
/example.php:333 = /example.php:339
/example.php:334 = /example.php:340
/example.php:335 = /example.php:341
/example.php:336 = /example.php:342
/example.php:337 = /example.php:343
/example.php:338 = /example.php:344
/example.php:339 = /example.php:345
/example.php:340 = /example.php:346
/example.php:341 = /example.php:347
/example.php:342 = /example.php:348
/example.php:343 = /example.php:349
/example.php:344 = /example.php:350
/example.php:345 = /example.php:351
/example.php:346 = /example.php:352
/example.php:347 = /example.php:353
/example.php:348 = /example.php:354
/example.php:349 = /example.php:355
/example.php:350 = /example.php:356
/example.php:351 = /example.php:357
/example.php:352 = /example.php:358
/example.php:353 = /example.php:359
/example.php:354 = /example.php:360
/example.php:355 = /example.php:361
/example.php:356 = /example.php:362
/example.php:357 = /example.php:363
/example.php:358 = /example.php:364
/example.php:359 = /example.php:365
/example.php:360 = /example.php:366
/example.php:361 = /example.php:367
/example.php:362 = /example.php:368
/example.php:363 = /example.php:369
/example.php:364 = /example.php:370
/example.php:365 = /example.php:371
/example.php:366 = /example.php:372
/example.php:367 = /example.php:373
/example.php:368 = /example.php:374
/example.php:369 = /example.php:375
/example.php:370 = /example.php:376
/example.php:371 = /example.php:377
/example.php:372 = /example.php:378
/example.php:373 = /example.php:379
/example.php:374 = /example.php:380
/example.php:375 = /example.php:381
/example.php:376 = /example.php:382
/example.php:377 = /example.php:383
/example.php:378 = /example.php:384
/example.php:379 = /example.php:385
/example.php:380 = /example.php:386
/example.php:381 = /example.php:387
/example.php:382 = /example.php:388
/example.php:383 = /example.php:389
/example.php:384 = /example.php:390
/example.php:385 = /example.php:391
/example.php:386 = /example.php:392
/example.php:387 = /example.php:393
/example.php:388 = /example.php:394
/example.php:389 = /example.php:395
/example.php:390 = /example.php:396
/example.php:391 = /example.php:397
/example.php:392 = /example.php:398
/example.php:393 = /example.php:399
/example.php:394 = /example.php:400
/example.php:395 = /example.php:401
/example.php:396 = /example.php:402
/example.php:397 = /example.php:403
/example.php:398 = /example.php:404
/example.php:399 = /example.php:405
/example.php:400 = /example.php:406
/example.php:401 = /example.php:407
/example.php:402 = /example.php:408
/example.php:403 = /example.php:409
/example.php:404 = /example.php:410
/example.php:405 = /example.php:411
/example.php:406 = /example.php:412
/example.php:407 = /example.php:413
/example.php:408 = /example.php:414
/example.php:409 = /example.php:415
/example.php:410 = /example.php:416
/example.php:411 = /example.php:417
/example.php:412 = /example.php:418
/example.php:413 = /example.php:419
/example.php:414 = /example.php:420
/example.php:415 = /example.php:421
/example.php:416 = /example.php:422
/example.php:417 = /example.php:423
/example.php:418 = /example.php:424
/example.php:419 = /example.php:425
/example.php:420 = /example.php:426
/example.php:421 = /example.php:427
/example.php:422 = /example.php:428
/example.php:423 = /example.php:429
/example.php:424 = /example.php:430
/example.php:425 = /example.php:431
/example.php:426 = /example.php:432
/example.php:427 = /example.php:433
/example.php:428 = /example.php:434
/example.php:429 = /example.php:435
/example.php:430 = /example.php:436
/example.php:431 = /example.php:437
/example.php:432 = /example.php:438
/example.php:433 = /example.php:439
/example.php:434 = /example.php:440
/example.php:435 = /example.php:441
/example.php:436 = /example.php:442
/example.php:437 = /example.php:443
/example.php:438 = /example.php:444
/example.php:439 = /example.php:445
/example.php:440 = /example.php:446
/example.php:441 = /example.php:447
/example.php:442 = /example.php:448
/example.php:443 = /example.php:449
/example.php:444 = /example.php:450
/example.php:445 = /example.php:451
/example.php:446 = /example.php:452
/example.php:447 = /example.php:453
/example.php:448 = /example.php:454
/example.php:449 = /example.php:455
/example.php:450 = /example.php:456
/example.php:451 = /example.php:457
/example.php:452 = /example.php:458
/example.php:453 = /example.php:459
/example.php:454 = /example.php:460
/example.php:455 = /example.php:461
/example.php:456 = /example.php:462
/example.php:457 = /example.php:463
/example.php:458 = /example.php:464
/example.php:459 = /example.php:465
/example.php:460 = /example.php:466
/example.php:461 = /example.php:467
/example.php:462 = /example.php:468
/example.php:463 = /example.php:469
/example.php:464 = /example.php:470
/example.php:465 = /example.php:471
/example.php:466 = /example.php:472
/example.php:467 = /example.php:473
/example.php:468 = /example.php:474
/example.php:469 = /example.php:475
/example.php:470 = /example.php:476
/example.php:471 = /example.php:477
/example.php:472 = /example.php:478
/example.php:473 = /example.php:479
/example.php:474 = /example.php:480
/example.php:475 = /example.php:481
/example.php:476 = /example.php:482
/example.php:477 = /example.php:483
/example.php:478 = /example.php:484
/example.php:479 = /example.php:485
/example.php:480 = /example.php:486
/example.php:481 = /example.php:487
/example.php:482 = /example.php:488
/example.php:483 = /example.php:489
/example.php:484 = /example.php:490
/example.php:485 = /example.php:491
/example.php:486 = /example.php:492
/example.php:487 = /example.php:493
/example.php:488 = /example.php:494
/example.php:489 = /example.php:495
/example.php:490 = /example.php:496
/example.php:491 = /example.php:497
/example.php:492 = /example.php:498
/example.php:493 = /example.php:499
/example.php:494 = /example.php:500
/example.php:495 = /example.php:501
/example.php:496 = /example.php:502
/example.php:497 = /example.php:503
/example.php:498 = /example.php:504
/example.php:499 = /example.php:505
/example.php:500 = /example.php:506
/example.php:501 = /example.php:507
/example.php:502 = /example.php:508
/example.php:503 = /example.php:509
/example.php:504 = /example.php:510
/example.php:505 = /example.php:511
/example.php:506 = /example.php:512
/example.php:507 = /example.php:513
/example.php:508 = /example.php:514
/example.php:509 = /example.php:515
/example.php:510 = /example.php:516
/example.php:511 = /example.php:517
/example.php:512 = /example.php:518
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	srandom(time(NULL));

	for (int i = 1; i < 500000; i++) {
		int j = 1 + (random() % 511);

		test_remote_to_local("/usr/local/www/example.php", j);
		check_map_with_range(XDEBUG_PATH_MAP_TYPE_LINES, "/home/derick/project/example.php", j + 6);
	}
};

TEST(path_maps_file, fuzz_test_01)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/projects
/servers/example.com/ = /example.com/
/servers/example.net/ = /example.net/
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("", 8051);
	CHECK_EQUAL(XDEBUG_PATH_MAP_TYPE_UNKNOWN, mapping_type);
};

TEST(path_maps_file, multiple_ranges_with_gap)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:1-10 = /example.php:7-16
/example.php:22-32 = /example.php:44-54
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www/example.php", 13);
	CHECK_EQUAL(XDEBUG_PATH_MAP_TYPE_UNKNOWN, mapping_type);
};

TEST(path_maps_file, wrong_order_of_lines_in_map_1)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:4 = /example.php:3
/example.php:4 = /example.php:7
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 5, "The remote range begin line (4) must be higher than the previous range end line (4)");
};

TEST(path_maps_file, wrong_order_of_lines_in_map_2)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/project
/example.php:21-32 = /example.php:43-54
/example.php:1-20 = /example.php:7-26
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_WRONG_RANGE, 5, "The remote range begin line (1) must be higher than the previous range end line (32)");
};

TEST(path_maps_file, two_files)
{
	const char *map1 = R""""(
remote_prefix: /usr/local/www1
local_prefix: /home/derick/project1
/example.php:1-20 = /example.php:7-26
/example.php:21-32 = /example.php:43-54
)"""";

	const char *map2 = R""""(
remote_prefix: /usr/local/www2
local_prefix: /home/derick/project2
/example.php:1-20 = /example.php:7-26
/example.php:21-32 = /example.php:43-54
)"""";

	result = test_map_from_file(map1);
	result = test_map_from_file(map2);
	check_result(PATH_MAPS_OK, -1, NULL);

	test_remote_to_local("/usr/local/www1/example.php", 2);
	check_map_with_range(XDEBUG_PATH_MAP_TYPE_LINES, "/home/derick/project1/example.php", 8);

	test_remote_to_local("/usr/local/www2/example.php", 2);
	check_map_with_range(XDEBUG_PATH_MAP_TYPE_LINES, "/home/derick/project2/example.php", 8);
};
