--TEST--
Test for bug #931: Serialization of closure causes segfault.
--INI--
xdebug.default_enable=1
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
xdebug.trace_format=0
--FILE--
<?php
session_start();
$_SESSION['test'] = function() { };
?>
--EXPECTF--
Fatal error: Uncaught exception 'Exception' with message 'Serialization of 'Closure' is not allowed' in [no active file] on line 0
