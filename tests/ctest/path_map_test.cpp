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

	mapping = remote_to_local(test_map, "/var/www/");

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

	mapping = remote_to_local(test_map, "/var/www/");

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

	mapping = remote_to_local(test_map, "/var/www/");

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

	mapping = remote_to_local(test_map, "/var/www/");

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

	mapping = remote_to_local(test_map, "/usr/local/www/servers/example.com/");
	check_map(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/home/derick/projects/example.com/");

	mapping = remote_to_local(test_map, "/usr/local/www/servers/example.net/");
	check_map(XDEBUG_PATH_MAP_TYPE_DIRECTORY, "/home/derick/projects/example.net/");
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
	check_result(PATH_MAPS_MISMATCHED_TYPES, 4, "Can't determine type of remote mapping part ('/usr/local/www/servers/example.org?')");
};

TEST(path_maps_file, local_part_unknown_type)
{
	const char *map = R""""(
remote_prefix: /usr/local/www
local_prefix: /home/derick/projects
/servers/example.org/ = /example.org?
)"""";

	result = test_map_from_file(map);
	check_result(PATH_MAPS_MISMATCHED_TYPES, 4, "Can't determine type of local mapping part ('/home/derick/projects/example.org?')");
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

	mapping = remote_to_local(test_map, "/usr/local/www/example.php");
	check_map(XDEBUG_PATH_MAP_TYPE_FILE, "/home/derick/project/example.php");
};
