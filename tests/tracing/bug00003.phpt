--TEST--
Test for bug #3: Crash tracing to file
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
date.timezone=Europe/Oslo
--FILE--
<?php
require_once 'capture-trace.inc';
@printf('%d %d %d %d:%d:%d', 1061728888, 1061728888, 1061728888, 1061728888, 1061728888, 1061728888);
xdebug_stop_trace();
?>
--EXPECTF--
%ATRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f%w%d     -> printf($format = '%s', ...$values = variadic(%s)) %sbug00003.php:3
%w%f%w%d     -> xdebug_stop_trace() %sbug00003.php:4
%w%f%w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
