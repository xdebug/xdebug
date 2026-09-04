#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "system_utils.h"

int main(int argc, char *argv[])
{
	char *cmd;
	char *result = NULL;
	char buffer[8192];
	FILE  *fp = fopen(argv[1], "r");

	fread((char*) &buffer, sizeof(buffer), 1, fp);
	xdebug_scan_mountinfo_for_private_tmp(buffer, &result);

	printf("Result: %s\n", result);

	free(result);
	fclose(fp);
}
