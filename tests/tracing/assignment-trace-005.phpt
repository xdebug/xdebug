--TEST--
Test for tracing assignments in user-readable function traces
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
require_once 'capture-trace.inc';

$t = array();
$t[] = 42;

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
                           => $t = [] %sassignment-trace-005.php:4
                           => $t[] = 42 %sassignment-trace-005.php:5
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-005.php:7
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
