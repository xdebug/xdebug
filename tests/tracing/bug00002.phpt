--TEST--
Test for traces to file
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
require 'bug00002.inc';
require_once 'capture-trace.inc';

$action = 'do_stuff';
$action();
xdebug_stop_trace();
?>
--EXPECTF--

TRACE START [%d-%d-%d %d:%d:%d.%d]
    %f%w%d     -> do_stuff() %sbug00002.php:6
    %f%w%d     -> xdebug_stop_trace() %sbug00002.php:7
    %f%w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
