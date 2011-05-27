--TEST--
Test for Xdebug's remote log (can not connect, with not-found remote callback)
--INI--
xdebug.remote_log=/tmp/remote-log2.txt
xdebug.remote_autostart=1
xdebug.remote_connect_back=1
xdebug.remote_host=doesnotexist2
xdebug.remote_port=9003
--FILE--
<?php
echo strlen("foo"), "\n";
echo file_get_contents("/tmp/remote-log2.txt");
unlink ("/tmp/remote-log2.txt");
?>
--EXPECTF--
3
Log opened at %d-%d-%d %d:%d:%d
I: Checking remote connect back address.
W: Remote address not found, connecting to configured address/port: doesnotexist2:9003. :-|
E: Could not connect to client. :-(
Log closed at %d-%d-%d %d:%d:%d
