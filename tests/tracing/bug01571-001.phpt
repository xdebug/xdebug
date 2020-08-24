--TEST--
Test for bug #1571: Stack traces don't show file/line for closures in namespaces
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
--FILE--
<?php
namespace A;

$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

$a = function(){

};
$a(1, $a);

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> A\{closure:%sbug01571-001.php:6-8}(1, class Closure {  }) %sbug01571-001.php:9
%w%f %w%d     -> xdebug_stop_trace() %sbug01571-001.php:11
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
