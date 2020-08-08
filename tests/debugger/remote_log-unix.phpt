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
xdebug.remote_log={TMPDIR}/{RUNID}remote-log4.txt
xdebug.remote_host=unix:///tmp/xdbg.sock
xdebug.remote_port=0
--FILE--
<?php
echo strlen("foo"), "\n";
echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . 'remote-log4.txt' );
unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . 'remote-log4.txt' );
?>
--EXPECTF--
3
[%d] Log opened at %d-%d-%d %d:%d:%d
[%d] I: Connecting to configured address/port: unix:///tmp/xdbg.sock:0.
[%d] W: Creating socket for 'unix:///tmp/xdbg.sock', connect: No such file or directory.
[%d] E: Could not connect to client. :-(
[%d] Log closed at %d-%d-%d %d:%d:%d
