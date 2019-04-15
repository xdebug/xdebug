--TEST--
Test for bug #1656: remote_connect_back alters remote_addr_value
--SKIPIF--
<?php if (substr(PHP_OS, 0, 3) == "WIN") die("skip Not for Windows"); ?>
<?php if (getenv("SKIP_UNPARALLEL_TESTS")) { exit("skip Excluding tests that can not be run in parallel"); } ?>
--ENV--
HTTP_X_FORWARDED_FOR=host1, host2
--INI--
xdebug.remote_enable=1
xdebug.remote_log=/tmp/bug01656.txt
xdebug.remote_autostart=1
xdebug.remote_connect_back=1
xdebug.remote_port=9003
--FILE--
<?php
echo $_SERVER["HTTP_X_FORWARDED_FOR"], "\n";
unlink( sys_get_temp_dir() . "/bug01656.txt" );
?>
--EXPECTF--
host1, host2
