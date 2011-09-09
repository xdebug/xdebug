--TEST--
Test for bug #709: Xdebug doesn't understand E_USER_DEPRECATED.
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if (version_compare(phpversion(), "5.3.0", '<')) echo "skip Feature only supported in PHP 5.3 and greater"; ?>
--INI--
xdebug.default_enable=1
xdebug.trace_format=0
xdebug.collect_params=3
--FILE--
<?php
trigger_error('ZOMG', E_USER_DEPRECATED);
?>
--EXPECTF--
Deprecated: ZOMG in %sbug00709.php on line 2

Call Stack:
%w%f %w%d   1. {main}() %sbug00709.php:0
%w%f %w%d   2. trigger_error('ZOMG', 16384) %sbug00709.php:2
