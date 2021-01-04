--TEST--
Starting Tracing: trigger, environment [2]
--INI--
xdebug.mode=trace
xdebug.start_with_request=trigger
xdebug.collect_return=0
xdebug.collect_assignments=0
variables_order=EPGCS
--ENV--
XDEBUG_TRACE=anything
--FILE--
<?php
$fileName = xdebug_get_tracefile_name();

xdebug_stop_trace();

echo file_get_contents($fileName);
unlink($fileName);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d   -> {main}() %sstart_with_request_trigger_env-002.php:0
%w%f %w%d     -> xdebug_get_tracefile_name() %sstart_with_request_trigger_env-002.php:2
%w%f %w%d     -> xdebug_stop_trace() %sstart_with_request_trigger_env-002.php:4
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
