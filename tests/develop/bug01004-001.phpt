--TEST--
Test for xdebug.halt_level [1]
--INI--
error_level=-1
xdebug.halt_level=0
xdebug.mode=develop
--FILE--
<?php
ini_set('xdebug.halt_level', 0);
hex2bin("5");
echo "Hi!\n";

ini_set('xdebug.halt_level', E_WARNING);
hex2bin("5");
echo "Hi!\n";

?>
--EXPECTF--
Warning: %s in %sbug01004-001.php on line 3

Call Stack:
%w%f %w%d   1. {main}() %sbug01004-001.php:0
%w%f %w%d   2. hex2bin($%s = '5') %sbug01004-001.php:3

Hi!

Warning: %s in %sbug01004-001.php on line 7

Call Stack:
%w%f %w%d   1. {main}() %sbug01004-001.php:0
%w%f %w%d   2. hex2bin($%s = '5') %sbug01004-001.php:7
