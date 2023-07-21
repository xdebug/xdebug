--TEST--
Starting Tracing: never, no environment
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
$fileName = xdebug_get_tracefile_name();
var_dump($fileName);
?>
--EXPECTF--
bool(false)
