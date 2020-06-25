--TEST--
Function Monitor: Setting a replacement list of functions to spy on
--INI--
xdebug.mode=develop
--FILE--
<?php
xdebug_start_function_monitor( [ 'strlen', 'array_push' ] );
xdebug_stop_function_monitor();
xdebug_start_function_monitor( [ 'strlen', 'array_pull' ] );
?>
===DONE===
--EXPECT--
===DONE===
