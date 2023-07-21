--TEST--
Test for Xdebug's remote log (can not connect, with not-found remote callback)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; !win');
?>
--INI--
xdebug.mode=debug
xdebug.start_with_request=yes
xdebug.log={TMP}/{RUNID}{TEST_PHP_WORKER}remote-log2.txt
xdebug.discover_client_host=1
xdebug.client_host=doesnotexist2
xdebug.client_port=9003
--FILE--
<?php
echo strlen("foo"), "\n";
echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'remote-log2.txt' );
unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'remote-log2.txt' );
?>
--EXPECTF--
3
[%d] Log opened at %d-%d-%d %d:%d:%d.%d
[%d] [Step Debug] INFO: Checking for client discovery headers: 'HTTP_X_FORWARDED_FOR,REMOTE_ADDR'.
[%d] [Step Debug] INFO: Checking header 'HTTP_X_FORWARDED_FOR'.
[%d] [Step Debug] INFO: Checking header 'REMOTE_ADDR'.
[%d] [Step Debug] WARN: Could not discover client host through HTTP headers, connecting to configured address/port: doesnotexist2:9003.
[%d] [Step Debug] WARN: Creating socket for 'doesnotexist2:9003', getaddrinfo: %s.
[%d] [Step Debug] ERR: Could not connect to debugging client. Tried: doesnotexist2:9003 (fallback through xdebug.client_host/xdebug.client_port).
