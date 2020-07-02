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
xdebug.remote_log=/tmp/{RUNID}remote-log4.txt
xdebug.remote_connect_back=1
xdebug.remote_host=unix:///tmp/xdbg.sock
xdebug.remote_port=0
xdebug.remote_addr_header=I_LIKE_COOKIES
--FILE--
<?php
if (sys_get_temp_dir() !== '/tmp') die('Unexpected temp dir: '.sys_get_temp_dir());
echo strlen("foo"), "\n";
echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . 'remote-log4.txt' );
unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . 'remote-log4.txt' );
?>
--EXPECTF--
3
[%d] Log opened at %d-%d-%d %d:%d:%d
[%d] I: Checking remote connect back address.
[%d] I: Checking user configured header 'I_LIKE_COOKIES'.
[%d] W: Invalid remote address provided containing URI spec 'unix:///tmp/haxx0r.sock'.
[%d] W: Remote address not found, connecting to configured address/port: unix:///tmp/xdbg.sock:0. :-|
[%d] W: Creating socket for 'unix:///tmp/xdbg.sock', connect: No such file or directory.
[%d] E: Could not connect to client. :-(
[%d] Log closed at %d-%d-%d %d:%d:%d
