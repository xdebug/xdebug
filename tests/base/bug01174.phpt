--TEST--
Test for bug #1174: SIGFPE in xdebug_get_monitored_functions()
--INI--
xdebug.mode=develop
--FILE--
<?php
xdebug_start_function_monitor([]);
$functions = xdebug_get_monitored_functions();
?>
==DONE==
--EXPECT--
==DONE==
