--TEST--
Test for bug #2214: Array keys aren't escaped in traces
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
require_once 'capture-trace.inc';

require dirname( __FILE__ ) . '/bug02214.inc';

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> dirname($path = '%sbug02214-001.php') %sbug02214-001.php:4
%w%f %w%d     -> require(%sbug02214.inc) %sbug02214-001.php:4
%w%f %w%d       -> func($a = ['\n' => '\n', '\r' => '\r', '\r\n' => '\r\n']) %sbug02214.inc:%d
%w%f %w%d     -> xdebug_stop_trace() %sbug02214-001.php:6
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
