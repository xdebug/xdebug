--TEST--
Starting Tracing: trigger, trigger match
--INI--
xdebug.mode=trace
xdebug.start_with_request=trigger
xdebug.trigger_value=SOMETHING
xdebug.collect_params=0
xdebug.collect_return=0
xdebug.collect_assignments=0
--ENV--
XDEBUG_TRACE=SOMETHING
--FILE--
<?php
$fileName = xdebug_get_tracefile_name();

xdebug_stop_trace();

echo file_get_contents($fileName);
unlink($fileName);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d   -> {main}() %stests/tracing/start_with_request_trigger_match.php:0
%w%f %w%d     -> xdebug_get_tracefile_name() %stests/tracing/start_with_request_trigger_match.php:2
%w%f %w%d     -> xdebug_stop_trace() %stests/tracing/start_with_request_trigger_match.php:4
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
