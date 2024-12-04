#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "maps_private.h"
#include "parser.h"

int main(int argc, char *argv[])
{
	struct xdebug_path_maps *test_map;
	int   error_code = 0;
	int   error_line = 0;
	char *error_message = NULL;
	FILE *test_path_f;
	char test_path[4096];

	xdebug_str *local_path;
	size_t      local_line;
	int         mapping_type;

	test_map = xdebug_path_maps_ctor();
	xdebug_path_maps_parse_file(test_map, "RELATIVE", "fuzz-seeds/directory.map", &error_code, &error_line, &error_message);
	test_path_f = fopen(argv[1], "r");
	fread(test_path, sizeof(test_path) - 1, 1, test_path_f);
	fclose(test_path_f);

	mapping_type = remote_to_local(test_map, test_path, 2, &local_path, &local_line);

	printf("Error code: %d\n", error_code);

	if (error_message) {
		free(error_message);
	}

	xdebug_path_maps_dtor(test_map);
}
