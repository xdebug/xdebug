--TEST--
Test for Xdebug's remote log (can not connect, with not-found remote callback)
--SKIPIF--
<?php if (substr(PHP_OS, 0, 3) == "WIN") die("skip Not for Windows"); ?>
--INI--
xdebug.remote_enable=1
xdebug.remote_log=/tmp/remote-log2.txt
xdebug.remote_autostart=1
xdebug.remote_connect_back=1
xdebug.remote_host=doesnotexist2
xdebug.remote_port=9003
--FILE--
<?php
echo strlen("foo"), "\n";
echo file_get_contents(sys_get_temp_dir() . "/remote-log2.txt");
unlink (sys_get_temp_dir() . "/remote-log2.txt");
?>
--EXPECTF--
3
[%d] Log opened at %d-%d-%d %d:%d:%d
[%d] I: Checking remote connect back address.
[%d] I: Checking header 'HTTP_X_FORWARDED_FOR'.
[%d] I: Checking header 'REMOTE_ADDR'.
[%d] W: Remote address not found, connecting to configured address/port: doesnotexist2:9003. :-|
[%d] W: Creating socket for 'doesnotexist2:9003', getaddrinfo: %s.
[%d] E: Could not connect to client. :-(
[%d] Log closed at %d-%d-%d %d:%d:%d
[%d]
[%d] Log opened at %d-%d-%d %d:%d:%d
[%d] I: Checking remote connect back address.
[%d] I: Checking header 'HTTP_X_FORWARDED_FOR'.
[%d] I: Checking header 'REMOTE_ADDR'.
[%d] W: Remote address not found, connecting to configured address/port: doesnotexist2:9003. :-|
[%d] W: Creating socket for 'doesnotexist2:9003', getaddrinfo: %s.
[%d] E: Could not connect to client. :-(
[%d] Log closed at %d-%d-%d %d:%d:%d
[%d]
