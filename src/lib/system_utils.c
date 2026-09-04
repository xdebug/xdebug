/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2026 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
 */
#define _GNU_SOURCE 1
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "arg.h"
#include "str.h"
#include "xdebug_strndup.h"

#ifdef __linux__
int xdebug_scan_mountinfo_for_private_tmp(const char *buffer, char **private_tmp)
{
	int         i;
	int         retval = 0;
	xdebug_arg *lines;

	lines = xdebug_arg_ctor();
	xdebug_explode("\n", buffer, lines, -1);

	/* Check whether each line has " /tmp ", and if so, parse out the previous stanza, which should have "systemd-private" in it, and extract accordingly. */
	for (i = 0; i < lines->c; i++) {
		const char *tmp_entry_start, *mount_start, *mount_end;
		bool tmpfs_present, dev_mapper_system_tmplv_present;

		/* check whether "tmpfs tmpfs" is present (for Fedora etc) */
		tmpfs_present = strstr(lines->args[i], " tmpfs tmpfs ") != NULL;

		/* check wehther /dev/mapper/system-tmplv is used (for Suse etc) */
		dev_mapper_system_tmplv_present = strstr(lines->args[i], "/dev/mapper/system-tmplv") != NULL;

		tmp_entry_start = strstr(lines->args[i], " /tmp ");
		if (tmp_entry_start == NULL) {
			continue;
		}

		/* find the previous ' ' */
		mount_start = memrchr(lines->args[i], ' ', tmp_entry_start - lines->args[i]);
		if (mount_start == NULL) {
			continue;
		}
		mount_start++; /* skip to '/' */

		/* find next ' ' */
		mount_end = strchr(mount_start, ' ');
		if (mount_end == NULL) {
			continue;
		}

		/* sanity check to see whether we're >= 5 chars */
		if (mount_end - mount_start < 5) {
			continue;
		}

		/* strip final /tmp, if present */
		if (mount_end[-4] == '/' && mount_end[-3] == 't' && mount_end[-2] == 'm' && mount_end[-1] == 'p') {
			mount_end -= 4;
		}

		if (tmpfs_present || dev_mapper_system_tmplv_present) {
			xdebug_str tmp_path = XDEBUG_STR_INITIALIZER;
			xdebug_str_add_literal(&tmp_path, "/tmp");
			xdebug_str_addl(&tmp_path, mount_start, mount_end - mount_start, false);

			*private_tmp = XDEBUG_STR_VAL((&tmp_path));
		} else {
			*private_tmp = xdstrndup(mount_start, mount_end - mount_start);
		}

		retval = 1;
		break;
	}

	/* Clean up and return */
	xdebug_arg_dtor(lines);
	return retval;
}

int xdebug_read_systemd_private_tmp_directory(char **private_tmp)
{
	pid_t       current_pid;
	char       *mountinfo_fn;
	FILE       *mountinfo_fd;
	size_t      bytes_read;
	char        buffer[8192] = { 0 };
	int         retval = 0;

	/* Open right file in /proc */
	current_pid = getpid();
	mountinfo_fn = xdebug_sprintf("/proc/%ld/mountinfo", current_pid);
	mountinfo_fd = fopen(mountinfo_fn, "r");
	xdfree(mountinfo_fn);
	if (!mountinfo_fd) {
		return retval;
	}

	/* Read contents and split in lines */
	bytes_read = fread(buffer, 1, sizeof(buffer), mountinfo_fd);
	if (!bytes_read) {
		fclose(mountinfo_fd);
		return retval;
	}

	retval = xdebug_scan_mountinfo_for_private_tmp(buffer, private_tmp);

	/* Clean up and return */
	fclose(mountinfo_fd);
	return retval;
}

#endif
