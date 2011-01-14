--TEST--
Test for bug #654: Xdebug hides error message in CLI
--INI--
error_reporting=-1
xdebug.default_enable=1
--FILE--
<?php
echo "FOO\n";
session_start();
?>
--EXPECTF--
FOO

Warning: session_start(): Cannot send session cookie - headers already sent by (output started at %sbug00654.php:2) in %sbug00654.php on line 3

Call Stack:
%w%f %w%d   1. {main}() %sbug00654.php:0
%w%f %w%d   2. session_start() %sbug00654.php:3


Warning: session_start(): Cannot send session cache limiter - headers already sent (output started at %sbug00654.php:2) in %sbug00654.php on line 3

Call Stack:
%w%f %w%d   1. {main}() %sbug00654.php:0
%w%f %w%d   2. session_start() %sbug00654.php:3
