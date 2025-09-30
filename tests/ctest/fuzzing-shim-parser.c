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

	test_map = xdebug_path_maps_ctor();
	xdebug_path_maps_parse_file(test_map, "RELATIVE", argv[1], &error_code, &error_line, &error_message);

	printf("Error code: %d\n", error_code);

	if (error_message) {
		free(error_message);
	}

	xdebug_path_maps_dtor(test_map);
}
