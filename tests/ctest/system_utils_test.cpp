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

TEST(system_utils, suse_private_tmp_001)
{
	xdebug_scan_mountinfo_for_private_tmp(
		"618 633 254:8 /systemd-private-57d3315f5ac2496098a5d7e5f2bbbea7-apache2.service-pwpfxr/tmp /tmp rw,relatime shared:519 master:79 - ext4 /dev/mapper/system-tmplv rw,discard,data=ordered",
		&result
	);

	STRCMP_EQUAL("/tmp/systemd-private-57d3315f5ac2496098a5d7e5f2bbbea7-apache2.service-pwpfxr", result);
};

TEST(system_utils, suse_private_tmp_002)
{
	xdebug_scan_mountinfo_for_private_tmp(
		"449 453 0:43 /systemd-private-2958f84513fe42429c36a41cecf12691-apache2.service-ARtJKj/tmp /tmp rw,nosuid,nodev shared:271 master:49 - tmpfs tmpfs rw,seclabel,nr_inodes=1048576,inode64,usrquota",
		&result
	);

	STRCMP_EQUAL("/tmp/systemd-private-2958f84513fe42429c36a41cecf12691-apache2.service-ARtJKj", result);
};

TEST(system_utils, suse_private_tmp_002_full)
{
	const char *mountinfo = R""""(
323 106 0:39 /@/.snapshots/1/snapshot / rw,relatime shared:244 master:1 - btrfs /dev/sda2 rw,seclabel,discard=async,space_cache=v2,subvolid=264,subvol=/@/.snapshots/1/snapshot
324 323 0:7 / /dev rw,nosuid shared:245 master:2 - devtmpfs devtmpfs rw,seclabel,size=4034248k,nr_inodes=1008562,mode=755,inode64
325 324 0:27 / /dev/shm rw,nosuid,nodev shared:246 master:3 - tmpfs tmpfs rw,seclabel,inode64,usrquota
326 324 0:28 / /dev/pts rw,nosuid,noexec,relatime shared:247 master:4 - devpts devpts rw,seclabel,gid=5,mode=600,ptmxmode=000
327 324 0:22 / /dev/mqueue rw,nosuid,nodev,noexec,relatime shared:248 master:16 - mqueue mqueue rw,seclabel
328 324 0:40 / /dev/hugepages rw,nosuid,nodev,relatime shared:249 master:19 - hugetlbfs hugetlbfs rw,seclabel,pagesize=2M
329 323 0:26 / /sys rw,nosuid,nodev,noexec,relatime shared:250 master:5 - sysfs sysfs rw,seclabel
330 329 0:8 / /sys/kernel/security rw,nosuid,nodev,noexec,relatime shared:251 master:6 - securityfs securityfs rw
331 329 0:30 / /sys/fs/cgroup rw,nosuid,nodev,noexec,relatime shared:252 master:7 - cgroup2 cgroup2 rw,seclabel,nsdelegate,memory_recursiveprot,memory_hugetlb_accounting
332 329 0:31 / /sys/fs/pstore rw,nosuid,nodev,noexec,relatime shared:253 master:8 - pstore none rw,seclabel
333 329 0:32 / /sys/firmware/efi/efivars rw,nosuid,nodev,noexec,relatime shared:254 master:9 - efivarfs efivarfs rw
334 329 0:33 / /sys/fs/bpf rw,nosuid,nodev,noexec,relatime shared:255 master:10 - bpf bpf rw,mode=700
335 329 0:20 / /sys/kernel/config rw,nosuid,nodev,noexec,relatime shared:256 master:11 - configfs configfs rw
336 329 0:23 / /sys/fs/selinux rw,nosuid,noexec,relatime shared:257 master:12 - selinuxfs selinuxfs rw
337 329 0:14 / /sys/kernel/tracing rw,nosuid,nodev,noexec,relatime shared:258 master:17 - tracefs tracefs rw,seclabel
338 329 0:9 / /sys/kernel/debug rw,nosuid,nodev,noexec,relatime shared:259 master:18 - debugfs debugfs rw,seclabel
442 338 0:14 / /sys/kernel/debug/tracing rw,nosuid,nodev,noexec,relatime shared:260 master:235 - tracefs tracefs rw,seclabel
443 329 0:44 / /sys/fs/fuse/connections rw,nosuid,nodev,noexec,relatime shared:261 master:23 - fusectl fusectl rw
444 323 0:25 / /proc rw,nosuid,nodev,noexec,relatime shared:262 master:13 - proc proc rw
445 444 0:35 / /proc/sys/fs/binfmt_misc rw,relatime shared:263 master:15 - autofs systemd-1 rw,fd=36,pgrp=1,timeout=0,minproto=5,maxproto=5,direct,pipe_ino=4757
446 445 0:51 / /proc/sys/fs/binfmt_misc rw,nosuid,nodev,noexec,relatime shared:264 master:63 - binfmt_misc binfmt_misc rw
447 323 0:29 / /run rw,nosuid,nodev shared:265 master:14 - tmpfs tmpfs rw,seclabel,size=1626072k,nr_inodes=819200,mode=755,inode64
450 447 0:72 / /run/user/1000 rw,nosuid,nodev,relatime shared:266 master:513 - tmpfs tmpfs rw,seclabel,size=813032k,nr_inodes=203258,mode=700,uid=1000,gid=1000,inode64
451 323 0:38 /@/.snapshots /.snapshots rw,relatime shared:268 master:45 - btrfs /dev/sda2 rw,seclabel,discard=async,space_cache=v2,subvolid=263,subvol=/@/.snapshots
452 323 0:42 /@/home /home rw,relatime shared:269 master:47 - btrfs /dev/sda2 rw,seclabel,discard=async,space_cache=v2,subvolid=262,subvol=/@/home
453 323 0:43 / /tmp rw,nosuid,nodev shared:270 master:49 - tmpfs tmpfs rw,seclabel,nr_inodes=1048576,inode64,usrquota
454 323 0:45 /@/root /root rw,relatime shared:272 master:51 - btrfs /dev/sda2 rw,seclabel,discard=async,space_cache=v2,subvolid=260,subvol=/@/root
455 323 0:47 /@/opt /opt rw,relatime shared:273 master:53 - btrfs /dev/sda2 rw,seclabel,discard=async,space_cache=v2,subvolid=261,subvol=/@/opt
456 323 0:46 /@/srv /srv rw,relatime shared:274 master:55 - btrfs /dev/sda2 rw,seclabel,discard=async,space_cache=v2,subvolid=259,subvol=/@/srv
457 323 0:48 /@/usr/local /usr/local rw,relatime shared:275 master:57 - btrfs /dev/sda2 rw,seclabel,discard=async,space_cache=v2,subvolid=258,subvol=/@/usr/local
458 323 0:49 /@/var /var rw,relatime shared:276 master:59 - btrfs /dev/sda2 rw,seclabel,discard=async,space_cache=v2,subvolid=257,subvol=/@/var
459 323 8:1 / /boot/efi rw,relatime shared:278 master:61 - vfat /dev/sda1 rw,fmask=0022,dmask=0077,codepage=437,iocharset=iso8859-1,shortname=mixed,utf8,errors=remount-ro
448 447 0:29 /systemd/inaccessible/dir /run/credentials ro,nosuid,nodev,noexec shared:267 master:14 - tmpfs tmpfs rw,seclabel,size=1626072k,nr_inodes=819200,mode=755,inode64
449 453 0:43 /systemd-private-2958f84513fe42429c36a41cecf12691-apache2.service-ARtJKj/tmp /tmp rw,nosuid,nodev shared:271 master:49 - tmpfs tmpfs rw,seclabel,nr_inodes=1048576,inode64,usrquota
460 458 0:49 /@/var/tmp/systemd-private-2958f84513fe42429c36a41cecf12691-apache2.service-NWNKuq/tmp /var/tmp rw,relatime shared:277 master:59 - btrfs /dev/sda2 rw,seclabel,discard=async,space_cache=v2,subvolid=257,subvol=/@/var
)"""";

	xdebug_scan_mountinfo_for_private_tmp(mountinfo, &result);

	STRCMP_EQUAL("/tmp/systemd-private-2958f84513fe42429c36a41cecf12691-apache2.service-ARtJKj", result);
};
