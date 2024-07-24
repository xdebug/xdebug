#include "CppUTest/TestHarness.h"
#include "maps_private.h"


TEST_GROUP(path_maps_file)
{
	struct xdebug_path_maps test_map;
	bool result;
	int   error_code;
	char *error_message;

	TEST_SETUP()
	{
		result = false;
		error_code = PATH_MAPS_OK;
		error_message = NULL;
	}

	TEST_TEARDOWN()
	{
		if (error_message) {
			free(error_message);
		}
	}
};

TEST(path_maps_file, fopen_non_existing)
{
	result = xdebug_path_maps_parse_file(&test_map, "file-does-not-exist.map", &error_code, &error_message);

	LONGS_EQUAL(result, false);
	LONGS_EQUAL(PATH_MAPS_CANT_OPEN_FILE, error_code);
	STRCMP_EQUAL("Can't open file", error_message);
};

TEST(path_maps_file, fopen_existing)
{
	result = xdebug_path_maps_parse_file(&test_map, "files/simple.map", &error_code, &error_message);

	LONGS_EQUAL(result, true);
	LONGS_EQUAL(PATH_MAPS_OK, error_code);
};
