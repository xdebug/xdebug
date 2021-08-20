--TEST--
Test for indirect function call
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
require_once 'capture-trace.inc';

function blaat ()
{
}

$func = 'blaat';
echo $func();

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> blaat() %stest11.php:9
%w%f %w%d      >=> NULL
%w%f %w%d     -> xdebug_stop_trace() %stest11.php:11
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
