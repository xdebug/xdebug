--TEST--
Test for tracing assignments in user-readable function traces
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

$t = array();
$t[] = 42;

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                           => $tf = '%s' %sassignment-trace-005.php:2
                           => $t = [] %sassignment-trace-005.php:4
                           => $t[] = 42 %sassignment-trace-005.php:5
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-005.php:7
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
