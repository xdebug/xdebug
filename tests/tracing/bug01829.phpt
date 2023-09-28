--TEST--
Test for bug #1829: Range and precision of time
--INI--
xdebug.mode=trace
xdebug.trace_format=0
xdebug.start_with_request=no
xdebug.collect_assignments=0
xdebug.collect_return=0
--FILE--
<?php
require_once 'capture-trace.inc';

var_dump( 42 );

xdebug_stop_trace();
?>
--EXPECTF--
int(42)
TRACE START [20%d-%d-%d %d:%d:%d.%d]
%w0.%d %w%d     -> var_dump($value = 42) %sbug01829.php:4
%w0.%d %w%d     -> xdebug_stop_trace() %sbug01829.php:6
%w0.%d %w%d
TRACE END   [20%d-%d-%d %d:%d:%d.%d]
