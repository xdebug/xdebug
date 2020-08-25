--TEST--
Starting Tracing: always, no trigger match
--INI--
xdebug.mode=trace
xdebug.start_with_request=yes
xdebug.trigger_value=FOOBAR
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
$fileName = xdebug_get_tracefile_name();

xdebug_stop_trace();

echo file_get_contents($fileName);
unlink($fileName);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d   -> {main}() %sstart_with_request_always_no_match.php:0
%w%f %w%d     -> xdebug_get_tracefile_name() %sstart_with_request_always_no_match.php:2
%w%f %w%d     -> xdebug_stop_trace() %sstart_with_request_always_no_match.php:4
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
