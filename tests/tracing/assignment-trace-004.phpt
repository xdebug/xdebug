--TEST--
Test for tracing assignments in user-readable function traces
--INI--
xdebug.mode=trace,develop
xdebug.start_with_request=0
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
xdebug.force_error_reporting=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

$t = 42;
$t += $b;
@$t += $b;

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
%s: Undefined variable%sb in %sassignment-trace-004.php on line 5

Call Stack:
%w%f %w%d   1. {main}() %sassignment-trace-004.php:0

TRACE START [%d-%d-%d %d:%d:%d.%d]
                           => $tf = '%s' %sassignment-trace-004.php:2
                           => $t = 42 %sassignment-trace-004.php:4
                           => $t += %r(NULL|\*uninitialized\*)%r %sassignment-trace-004.php:5
                           => $t += %r(NULL|\*uninitialized\*)%r %sassignment-trace-004.php:6
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-004.php:8
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
