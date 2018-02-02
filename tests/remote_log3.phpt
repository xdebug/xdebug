--TEST--
Test for Xdebug's remote log (with xdebug.remote_addr_header)
--SKIPIF--
<?php if (substr(PHP_OS, 0, 3) == "WIN") die("skip Not for Windows"); ?>
--INI--
xdebug.remote_enable=1
xdebug.remote_log=/tmp/remote-log3.txt
xdebug.remote_autostart=1
xdebug.remote_connect_back=1
xdebug.remote_host=doesnotexist2
xdebug.remote_port=9003
xdebug.remote_addr_header=I_LIKE_COOKIES
--FILE--
<?php
echo strlen("foo"), "\n";
echo file_get_contents(sys_get_temp_dir() . "/remote-log3.txt");
unlink (sys_get_temp_dir() . "/remote-log3.txt");
?>
--EXPECTF--
3
Log opened at %d-%d-%d %d:%d:%d
I: Checking remote connect back address.
I: Checking user configured header 'I_LIKE_COOKIES'.
I: Checking header 'HTTP_X_FORWARDED_FOR'.
I: Checking header 'REMOTE_ADDR'.
W: Remote address not found, connecting to configured address/port: doesnotexist2:9003. :-|
W: Creating socket for 'doesnotexist2:9003', getaddrinfo: %s.
E: Could not connect to client. :-(
Log closed at %d-%d-%d %d:%d:%d

Log opened at %d-%d-%d %d:%d:%d
I: Checking remote connect back address.
I: Checking user configured header 'I_LIKE_COOKIES'.
I: Checking header 'HTTP_X_FORWARDED_FOR'.
I: Checking header 'REMOTE_ADDR'.
W: Remote address not found, connecting to configured address/port: doesnotexist2:9003. :-|
W: Creating socket for 'doesnotexist2:9003', getaddrinfo: %s.
E: Could not connect to client. :-(
Log closed at %d-%d-%d %d:%d:%d
