--TEST--
Test for xdebug.halt_level [1]
--INI--
error_level=-1
xdebug.halt_level=0
xdebug.default_enable=1
--FILE--
<?php
ini_set('xdebug.halt_level', 0);
strlen();
echo "Hi!\n";

ini_set('xdebug.halt_level', E_WARNING);
strlen();
echo "Hi!\n";

?>
--EXPECTF--
Warning: %s in %sbug01004-1.php on line 3

Call Stack:
%w%f %w%d   1. {main}() %sbug01004-1.php:0
%w%f %w%d   2. strlen() %sbug01004-1.php:3

Hi!

Warning: %s in %sbug01004-1.php on line 7

Call Stack:
%w%f %w%d   1. {main}() %sbug01004-1.php:0
%w%f %w%d   2. strlen() %sbug01004-1.php:7
