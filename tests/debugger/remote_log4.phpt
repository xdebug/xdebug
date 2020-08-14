--TEST--
Test for Xdebug's remote log (with xdebug.remote_addr_header value)
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
xdebug.log={TMPDIR}/{RUNID}remote-log4.txt
xdebug.remote_connect_back=1
xdebug.remote_host=doesnotexist2
xdebug.remote_port=9003
xdebug.remote_addr_header=I_LIKE_COOKIES
--FILE--
<?php
echo strlen("foo"), "\n";
echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . 'remote-log4.txt' );
unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . 'remote-log4.txt' );
?>
--EXPECTF--
Xdebug: [Step Debug] Could not connect to debugging client. Tried: cookiehost:9003 (from I_LIKE_COOKIES HTTP header), doesnotexist2:9003 (fallback through xdebug.remote_host/xdebug.remote_port) :-(
3
[%d] Log opened at %d-%d-%d %d:%d:%d.%d
[%d] [Step Debug] INFO: Checking remote connect back address.
[%d] [Step Debug] INFO: Checking user configured header 'I_LIKE_COOKIES'.
[%d] [Step Debug] INFO: Remote address found, connecting to cookiehost:9003.
[%d] [Step Debug] WARN: Creating socket for 'cookiehost:9003', getaddrinfo: %s.
[%d] [Step Debug] WARN: Could not connect to remote address as found in headers, connecting to configured address/port: doesnotexist2:9003. :-|
[%d] [Step Debug] WARN: Creating socket for 'doesnotexist2:9003', getaddrinfo: %s.
[%d] [Step Debug] ERR: Could not connect to debugging client. Tried: cookiehost:9003 (from I_LIKE_COOKIES HTTP header), doesnotexist2:9003 (fallback through xdebug.remote_host/xdebug.remote_port) :-(
