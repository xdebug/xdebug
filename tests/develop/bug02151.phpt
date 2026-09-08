--TEST--
Test for bug #2151: Xdebug alters the output of floating point numbers in var_dump
--INI--
xdebug.mode=develop
--FILE--
<?php
var_dump(.2 + .1);
?>
--EXPECTF--
%sbug02151.php:2:
float(0.30000000000000004)
