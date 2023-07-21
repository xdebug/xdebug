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
@strftime('%b %l %Y %H:%M:%S', 1061728888);
xdebug_stop_trace();
?>
--EXPECTF--

TRACE START [%d-%d-%d %d:%d:%d.%d]
    %f%w%d     -> strftime($format = '%b %l %Y %H:%M:%S', $timestamp = 1061728888) %sbug00003.php:3
    %f%w%d     -> xdebug_stop_trace() %sbug00003.php:4
    %f%w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
