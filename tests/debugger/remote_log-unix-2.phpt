--TEST--
Test for Xdebug's remote log (with unix sockets and header)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; !win');
?>
--ENV--
I_LIKE_COOKIES=unix:///tmp/haxx0r.sock
--INI--
xdebug.mode=debug
xdebug.start_with_request=yes
xdebug.log={TMP}/{RUNID}{TEST_PHP_WORKER}remote-unix.txt
xdebug.discover_client_host=1
xdebug.client_host=unix:///tmp/xdbg.sock
xdebug.client_port=0
xdebug.client_discovery_header=I_LIKE_COOKIES
--FILE--
<?php
echo strlen("foo"), "\n";
echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'remote-unix.txt' );
unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'remote-unix.txt' );
?>
--EXPECTF--
3
[%d] Log opened at %d-%d-%d %d:%d:%d.%d
[%d] [Step Debug] INFO: Checking for client discovery headers: 'I_LIKE_COOKIES'.
[%d] [Step Debug] INFO: Checking header 'I_LIKE_COOKIES'.
[%d] [Step Debug] WARN: Invalid remote address provided containing URI spec 'unix:///tmp/haxx0r.sock'.
[%d] [Step Debug] WARN: Could not discover client host through HTTP headers, connecting to configured address/port: unix:///tmp/xdbg.sock:0.
[%d] [Step Debug] WARN: Creating socket for 'unix:///tmp/xdbg.sock', connect: No such file or directory.
[%d] [Step Debug] ERR: Could not connect to debugging client. Tried: unix:///tmp/xdbg.sock:0 (fallback through xdebug.client_host/xdebug.client_port).
