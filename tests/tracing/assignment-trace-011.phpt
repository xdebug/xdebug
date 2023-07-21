--TEST--
Test for tracing property assignments in user-readable function traces
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
require_once 'capture-trace.inc';

$a = new StdClass;

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
                           => $a = class stdClass {  } %sassignment-trace-011.php:4
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-011.php:6
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
