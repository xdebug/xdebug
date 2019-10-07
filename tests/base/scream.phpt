--TEST--
Test for scream support
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.scream=0
error_reporting(E_ALL);
xdebug.force_error_reporting=0
--FILE--
<?php
echo @strstr(), "\n";
ini_set('xdebug.scream', 1);
echo @strstr(), "\n";
ini_set('xdebug.scream', 0);
echo @strstr(), "\n";
?>
--EXPECTF--
SCREAM:  Error suppression ignored for
Warning: %s in %sscream.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sscream.php:0
%w%f %w%d   2. strstr() %sscream.php:4
