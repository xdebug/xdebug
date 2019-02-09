--TEST--
Test for Xdebug's remote log (with unix sockets)
--SKIPIF--
<?php if (substr(PHP_OS, 0, 3) == "WIN") die("skip Not for Windows"); ?>
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--ENV--
I_LIKE_COOKIES=doesnotexist3
--INI--
xdebug.remote_enable=1
xdebug.remote_log=/tmp/remote-log4.txt
xdebug.remote_autostart=1
xdebug.remote_host=unix:///tmp/xdbg.sock
xdebug.remote_port=0
--FILE--
<?php
if (sys_get_temp_dir() !== '/tmp') die('Unexpected temp dir: '.sys_get_temp_dir());
echo strlen("foo"), "\n";
echo file_get_contents(sys_get_temp_dir() . "/remote-log4.txt");
unlink (sys_get_temp_dir() . "/remote-log4.txt");
?>
--EXPECTF--
3
[%d] Log opened at %d-%d-%d %d:%d:%d
[%d] I: Connecting to configured address/port: unix:///tmp/xdbg.sock:0.
[%d] W: Creating socket for 'unix:///tmp/xdbg.sock', connect: No such file or directory.
[%d] E: Could not connect to client. :-(
[%d] Log closed at %d-%d-%d %d:%d:%d
[%d]
[%d] Log opened at %d-%d-%d %d:%d:%d
[%d] I: Connecting to configured address/port: unix:///tmp/xdbg.sock:0.
[%d] W: Creating socket for 'unix:///tmp/xdbg.sock', connect: No such file or directory.
[%d] E: Could not connect to client. :-(
[%d] Log closed at %d-%d-%d %d:%d:%d
[%d]
