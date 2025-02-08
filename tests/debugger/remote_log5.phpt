--TEST--
Test for Xdebug's remote log output through PHP's log
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; !win');
?>
--ENV--
I_LIKE_COOKIES=cookiehost
--INI--
xdebug.mode=debug
xdebug.start_with_request=yes
xdebug.log={TMP}/{RUNID}{TEST_PHP_WORKER}remote-log4.txt
xdebug.log_level=3
xdebug.discover_client_host=1
xdebug.client_host=doesnotexist2
xdebug.client_port=9003
xdebug.client_discovery_header=I_LIKE_COOKIES
--FILE--
<?php
echo strlen("foo"), "\n";
echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'remote-log4.txt' );
unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'remote-log4.txt' );
?>
--EXPECTF--
3
%A[Step Debug] WARN: Could not connect to debugging client. Tried: cookiehost:9003 (from I_LIKE_COOKIES HTTP header), doesnotexist2:9003 (fallback through xdebug.client_host/xdebug.client_port).
