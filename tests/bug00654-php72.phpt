--TEST--
Test for bug #654: Xdebug hides error message in CLI (>= PHP 7.2)
--SKIPIF--
<?php
if (!extension_loaded("session")) { echo "skip Session extension required\n"; }
if (!version_compare(phpversion(), "7.2", '>=')) echo "skip >= PHP 7.2 needed\n";
?>
--INI--
error_reporting=-1
xdebug.default_enable=1
xdebug.dump.SERVER=
--FILE--
<?php
echo "FOO\n";
session_start();
?>
--EXPECTF--
FOO

Warning: session_start(): Cannot start session when headers already sent in %sbug00654-php72.php on line 3

Call Stack:
%w%f %w%d   1. {main}() %sbug00654-php72.php:0
%w%f %w%d   2. session_start() %sbug00654-php72.php:3
