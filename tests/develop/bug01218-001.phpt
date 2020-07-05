--TEST--
Test for bug #1218: Xdebug messes with the exception code, by casting it to int
--INI--
xdebug.mode=develop
xdebug.show_exception_trace=1
xdebug.show_local_vars=0
--FILE--
<?php
include dirname(__FILE__) . '/bug01218.inc';
?>
--EXPECTF--
StringCodeException: test in %sbug01218.inc on line 12

Call Stack:
%w%f %w%d   1. {main}() %sbug01218-001.php:0
%w%f %w%d   2. include('%sbug01218.inc') %sbug01218-001.php:2

%sbug01218.inc:17:
string(10) "SomeString"
