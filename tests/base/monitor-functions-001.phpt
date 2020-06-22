--TEST--
Function Monitor: Setting a list of functions to spy on
--INI--
xdebug.mode=display
--FILE--
<?php
xdebug_start_function_monitor( [ 'strlen', 'array_push' ] );
?>
===DONE===
--EXPECT--
===DONE===
