--TEST--
Test for bug #654: Xdebug hides error message in CLI
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext session');
?>
--INI--
error_reporting=-1
xdebug.mode=develop
xdebug.dump.SERVER=
--FILE--
<?php
echo "FOO\n";
session_start();
?>
--EXPECTF--
FOO

Warning: session_start(): %sheaders%ssent in %sbug00654.php on line 3

Call Stack:
%w%f %w%d   1. {main}() %sbug00654.php:0
%w%f %w%d   2. session_start() %sbug00654.php:3
