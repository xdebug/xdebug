--TEST--
Test for Xdebug's remote log (with xdebug.remote_addr_header value)
--SKIPIF--
<?php if (substr(PHP_OS, 0, 3) == "WIN") die("skip Not for Windows"); ?>
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--ENV--
I_LIKE_COOKIES=doesnotexist3
--INI--
xdebug.remote_enable=1
xdebug.remote_log=/tmp/remote-log4.txt
xdebug.remote_autostart=1
xdebug.remote_connect_back=1
xdebug.remote_host=doesnotexist2
xdebug.remote_port=9003
xdebug.remote_addr_header=I_LIKE_COOKIES
--FILE--
<?php
echo strlen("foo"), "\n";
echo file_get_contents(sys_get_temp_dir() . "/remote-log4.txt");
unlink (sys_get_temp_dir() . "/remote-log4.txt");
?>
--EXPECTF--
3
[%d] Log opened at %d-%d-%d %d:%d:%d
[%d] I: Checking remote connect back address.
[%d] I: Checking user configured header 'I_LIKE_COOKIES'.
[%d] I: Remote address found, connecting to doesnotexist3:9003.
[%d] W: Creating socket for 'doesnotexist3:9003', getaddrinfo: %s.
[%d] E: Could not connect to client. :-(
[%d] Log closed at %d-%d-%d %d:%d:%d
[%d]
[%d] Log opened at %d-%d-%d %d:%d:%d
[%d] I: Checking remote connect back address.
[%d] I: Checking user configured header 'I_LIKE_COOKIES'.
[%d] I: Remote address found, connecting to doesnotexist3:9003.
[%d] W: Creating socket for 'doesnotexist3:9003', getaddrinfo: %s.
[%d] E: Could not connect to client. :-(
[%d] Log closed at %d-%d-%d %d:%d:%d
[%d]
