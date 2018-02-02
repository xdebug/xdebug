--TEST--
Test for Xdebug's remote log (can not connect, no remote callback)
--SKIPIF--
<?php if (substr(PHP_OS, 0, 3) == "WIN") die("skip Not for Windows"); ?>
--INI--
xdebug.remote_enable=1
xdebug.remote_log=/tmp/remote-log1.txt
xdebug.remote_autostart=1
xdebug.remote_connect_back=0
xdebug.remote_host=doesnotexist
xdebug.remote_port=9002
--FILE--
<?php
echo strlen("foo"), "\n";
echo file_get_contents(sys_get_temp_dir() . "/remote-log1.txt");
unlink (sys_get_temp_dir() . "/remote-log1.txt");
?>
--EXPECTF--
3
Log opened at %d-%d-%d %d:%d:%d
I: Connecting to configured address/port: doesnotexist:9002.
W: Creating socket for 'doesnotexist:9002', getaddrinfo: %s.
E: Could not connect to client. :-(
Log closed at %d-%d-%d %d:%d:%d

Log opened at %d-%d-%d %d:%d:%d
I: Connecting to configured address/port: doesnotexist:9002.
W: Creating socket for 'doesnotexist:9002', getaddrinfo: %s.
E: Could not connect to client. :-(
Log closed at %d-%d-%d %d:%d:%d
