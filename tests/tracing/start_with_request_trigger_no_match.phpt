--TEST--
Starting Tracing: trigger, no trigger match
--INI--
xdebug.mode=trace
xdebug.start_with_request=trigger
xdebug.trigger_value=FOOBAR
xdebug.collect_return=0
xdebug.collect_assignments=0
--ENV--
XDEBUG_TRACE=SOMETHINGELSE
--FILE--
<?php
$fileName = xdebug_get_tracefile_name();
var_dump($fileName);
?>
--EXPECTF--
bool(false)
