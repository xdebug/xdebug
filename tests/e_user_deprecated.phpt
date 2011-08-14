--TEST--
Test to make sure E_USER_DEPRECATED is labeled correctly.
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.default_enable=1
xdebug.trace_format=0
xdebug.collect_params=3
--FILE--
<?php
trigger_error('ZOMG', E_USER_DEPRECATED);
?>
--EXPECTF--
Deprecated: ZOMG in %se_user_deprecated.php on line 2

Call Stack:
%w%f %w%d   1. {main}() %se_user_deprecated.php:0
%w%f %w%d   2. trigger_error('ZOMG', 16384) %se_user_deprecated.php:2