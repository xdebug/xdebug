--TEST--
Starting Tracing: trigger, environment [1]
--INI--
xdebug.mode=trace
xdebug.trace_format=0
xdebug.start_with_request=trigger
xdebug.collect_return=0
xdebug.collect_assignments=0
variables_order=PGCS
--ENV--
XDEBUG_TRACE=anything
--FILE--
<?php
$tf = xdebug_get_tracefile_name();

xdebug_stop_trace();

if (preg_match('@\.gz$@', $tf)) {
	$fp = gzopen($tf, 'r');
	echo stream_get_contents($fp);
} else {
	echo file_get_contents($tf);
}
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d   -> {main}() %sstart_with_request_trigger_env-001.php:0
%w%f %w%d     -> xdebug_get_tracefile_name() %sstart_with_request_trigger_env-001.php:2
%w%f %w%d     -> xdebug_stop_trace() %sstart_with_request_trigger_env-001.php:4
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
