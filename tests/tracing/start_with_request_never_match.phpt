--TEST--
Starting Tracing: never, trigger match
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trigger_value=SOMETHING
xdebug.collect_return=0
xdebug.collect_assignments=0
--ENV--
XDEBUG_TRACE=SOMETHING
--FILE--
<?php
$fileName = xdebug_get_tracefile_name();
var_dump($fileName);
?>
--EXPECTF--
bool(false)
