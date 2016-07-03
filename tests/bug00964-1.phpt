--TEST--
Test for bug #964: IP retrival from X-Forwarded-For complies with rfc7239 (without comma)
--SKIPIF--
<?php if (substr(PHP_OS, 0, 3) == "WIN") die("skip Not for Windows"); ?>
--ENV--
HTTP_X_FORWARDED_FOR=192.168.111.111
--INI--
xdebug.remote_enable=1
xdebug.remote_log=/tmp/bug964.txt
xdebug.remote_autostart=1
xdebug.remote_connect_back=1
xdebug.remote_port=9003
--FILE--
<?php
preg_match("#Remote address found, connecting to (192\.168\.111\.111):9003#", file_get_contents(sys_get_temp_dir() . "/bug964.txt"), $match);
unlink (sys_get_temp_dir() . "/bug964.txt");
echo $match[1];
?>
--EXPECTF--
192.168.111.111
