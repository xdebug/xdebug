--TEST--
Test for bug #654: Xdebug hides error message in CLI (>= PHP 7.2)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.2; ext session');
?>
--INI--
error_reporting=-1
xdebug.mode=display
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
