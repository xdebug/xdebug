#include "CppUTest/TestHarness.h"

#include <unistd.h>

#include "system_utils.h"

TEST_GROUP(system_utils)
{
	char *result;

	TEST_SETUP()
	{
		result = NULL;
	}

	TEST_TEARDOWN()
	{
		if (result) {
			free(result);
		}
	}

};

TEST(system_utils, debian_private_tmp)
{
	xdebug_scan_mountinfo_for_private_tmp(
		"450 440 254:1 /tmp/systemd-private-1123811641874a8fb180e2072ea70ec3-php8.4-fpm.service-ae34J8/tmp /tmp rw,relatime shared:245 master:1 - ext4 /dev/mapper/mezcal--vg-root rw,errors=remount-ro\n",
		&result
	);

	STRCMP_EQUAL("/tmp/systemd-private-1123811641874a8fb180e2072ea70ec3-php8.4-fpm.service-ae34J8", result);
};

TEST(system_utils, fedora_private_tmp)
{
	xdebug_scan_mountinfo_for_private_tmp(
		"1076 1069 0:37 /systemd-private-276b8e9bc96845beb1071462c4598431-php-fpm.service-MZ2AB6/tmp /tmp rw,nosuid,nodev shared:581 master:38 - tmpfs tmpfs rw,seclabel,size=2 517164k,nr_inodes=409600,inode64",
		&result
	);

	STRCMP_EQUAL("/tmp/systemd-private-276b8e9bc96845beb1071462c4598431-php-fpm.service-MZ2AB6", result);
};

TEST(system_utils, suse_private_tmp)
{
	xdebug_scan_mountinfo_for_private_tmp(
		"618 633 254:8 /systemd-private-57d3315f5ac2496098a5d7e5f2bbbea7-apache2.service-pwpfxr/tmp /tmp rw,relatime shared:519 master:79 - ext4 /dev/mapper/system-tmplv rw,discard,data=ordered",
		&result
	);

	STRCMP_EQUAL("/tmp/systemd-private-57d3315f5ac2496098a5d7e5f2bbbea7-apache2.service-pwpfxr", result);
};
