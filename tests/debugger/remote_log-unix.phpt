--TEST--
Test for Xdebug's remote log (with unix sockets)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; !win');
?>
--ENV--
I_LIKE_COOKIES=doesnotexist3
--INI--
xdebug.mode=debug
xdebug.start_with_request=yes
xdebug.log={TMP}/{RUNID}{TEST_PHP_WORKER}remote-log4.txt
xdebug.client_host=unix:///tmp/xdbg.sock
xdebug.client_port=0
--FILE--
<?php
echo strlen("foo"), "\n";
echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'remote-log4.txt' );
unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'remote-log4.txt' );
?>
--EXPECTF--
3
[%d] Log opened at %d-%d-%d %d:%d:%d.%d
[%d] [Step Debug] INFO: Connecting to configured address/port: unix:///tmp/xdbg.sock:0.
[%d] [Step Debug] WARN: Creating socket for 'unix:///tmp/xdbg.sock', connect: No such file or directory.
[%d] [Step Debug] ERR: Could not connect to debugging client. Tried: unix:///tmp/xdbg.sock:0 (through xdebug.client_host/xdebug.client_port).
