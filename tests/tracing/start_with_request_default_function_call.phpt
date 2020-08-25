--TEST--
Starting Tracing: default, function call
--INI--
xdebug.mode=trace
xdebug.trigger_value=FOOBAR
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
$fileName = xdebug_start_trace();

xdebug_stop_trace();

echo file_get_contents($fileName);
unlink($fileName);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> xdebug_stop_trace() %sstart_with_request_default_function_call.php:4
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
