#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../../src/lib/cmd_parser.h"

int main(int argc, char *argv[])
{
	char *cmd;
	xdebug_dbgp_arg *ret_args;
	int   ret = 0;
	char buffer[512];
	FILE  *fp = fopen(argv[1], "r");

	fgets((char*) &buffer, sizeof(buffer), fp);
	ret = xdebug_cmd_parse(buffer, &cmd, &ret_args);

	printf("Return value: %d\n", ret);

	xdfree(cmd);
	xdebug_cmd_arg_dtor(ret_args);
	fclose(fp);
}
