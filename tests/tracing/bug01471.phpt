--TEST--
Test for bug #1471: Crash with tracing and ternairy op assignment
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.auto_profile=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

$ff = "bb";
$c = "aa" == $ff ? 1 : 0;

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                           => $tf = '%s' %sbug01471.php:2
                           => $ff = 'bb' %sbug01471.php:4
                           => $c = 0 %sbug01471.php:5
%w%f %w%d     -> xdebug_stop_trace() %sbug01471.php:7
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
