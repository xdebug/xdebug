--TEST--
Test for tracing array assignments in user-readable function traces
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.collect_vars=1
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
xdebug.force_error_reporting=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

$t = array( 'a' => 4, 'b' => 9, 'c' => 13 );
$t['a'] += $b;
@$t['a'] += $b;

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
Notice: Undefined variable: b in %sassignment-trace3.php on line 5

Call Stack:
%w%f %w%d   1. {main}() %sassignment-trace3.php:0

TRACE START [%d-%d-%d %d:%d:%d]
                           => $tf = '%s' %sassignment-trace3.php:2
                           => $t = array ('a' => 4, 'b' => 9, 'c' => 13) %sassignment-trace3.php:4
                           => $t['a'] += NULL %sassignment-trace3.php:5
                           => $t['a'] += NULL %sassignment-trace3.php:6
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace3.php:8
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
