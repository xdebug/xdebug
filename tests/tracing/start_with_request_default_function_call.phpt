--TEST--
Starting Tracing: default, function call
--INI--
xdebug.mode=trace
xdebug.trace_format=0
xdebug.trigger_value=FOOBAR
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
$tf = xdebug_start_trace();

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
%w%f %w%d     -> xdebug_stop_trace() %sstart_with_request_default_function_call.php:4
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
