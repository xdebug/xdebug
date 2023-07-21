--TEST--
Test for bug #709: Xdebug doesn't understand E_USER_DEPRECATED
--INI--
xdebug.mode=develop
xdebug.trace_format=0
--FILE--
<?php
trigger_error('ZOMG', E_USER_DEPRECATED);
?>
--EXPECTF--
Deprecated: ZOMG in %sbug00709.php on line 2

Call Stack:
%w%f %w%d   1. {main}() %sbug00709.php:0
%w%f %w%d   2. trigger_error($message = 'ZOMG', $error_%s = 16384) %sbug00709.php:2
