--TEST--
Test for internal parameters
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
require_once 'capture-trace.inc';

echo str_repeat ("5", 5), "\n";

xdebug_stop_trace();
?>
--EXPECTF--
55555
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> str_repeat($%s = '5', $%s = 5) %sinternal-parameters.php:4
%w%f %w%d     -> xdebug_stop_trace() %sinternal-parameters.php:6
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
