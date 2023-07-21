--TEST--
Test for scream support
--INI--
xdebug.mode=develop
xdebug.scream=0
error_reporting(E_ALL);
xdebug.force_error_reporting=0
--FILE--
<?php
echo @hex2bin('4'), "\n";
ini_set('xdebug.scream', 1);
echo @hex2bin('4'), "\n";
ini_set('xdebug.scream', 0);
echo @hex2bin('4'), "\n";
?>
--EXPECTF--
SCREAM:  Error suppression ignored for
Warning: %s in %sscream.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sscream.php:0
%w%f %w%d   2. hex2bin($%s = '4') %sscream.php:4
