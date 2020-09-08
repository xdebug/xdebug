--TEST--
Test for Xdebug's remote log (Windows)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; win');
?>
--ENV--
I_LIKE_COOKIES=doesnotexist3
--INI--
xdebug.mode=debug
xdebug.start_with_request=yes
xdebug.log=C:\Windows\Temp\remote-log4.txt
xdebug.remote_connect_back=1
xdebug.client_host=doesnotexist2
xdebug.client_port=9003
xdebug.remote_addr_header=I_LIKE_COOKIES
--FILE--
<?php
@unlink ("C:\\Windows\\Temp\\remote-log4.txt");
echo strlen("foo"), "\n";
echo file_get_contents("C:\\Windows\\Temp\\remote-log4.txt");
@unlink ("C:\\Windows\\Temp\\remote-log4.txt");
?>
--EXPECTF--
Xdebug: [Step Debug] Could not connect to debugging client. Tried: doesnotexist3:9003 (from I_LIKE_COOKIES HTTP header), doesnotexist2:9003 (fallback through xdebug.client_host/xdebug.client_port) :-(
3
[%d] Log opened at %d-%d-%d %d:%d:%d.%d
[%d] [Step Debug] INFO: Checking remote connect back address.
[%d] [Step Debug] INFO: Checking user configured header 'I_LIKE_COOKIES'.
[%d] [Step Debug] INFO: Remote address found, connecting to doesnotexist3:9003.
[%d] [Step Debug] WARN: Creating socket for 'doesnotexist3:9003', getaddrinfo: %d.
[%d] [Step Debug] WARN: Could not connect to remote address as found in headers, connecting to configured address/port: doesnotexist2:9003. :-|
[%d] [Step Debug] WARN: Creating socket for 'doesnotexist2:9003', getaddrinfo: 11001.
[%d] [Step Debug] ERR: Could not connect to debugging client. Tried: doesnotexist3:9003 (from I_LIKE_COOKIES HTTP header), doesnotexist2:9003 (fallback through xdebug.client_host/xdebug.client_port) :-(
