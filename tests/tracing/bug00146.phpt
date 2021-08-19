--TEST--
Test for bug #146: Array key names with quotes in traces are not escaped
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
require_once 'capture-trace.inc';

function foo($a)
{
	return $a;
}

$array = array("te\"st's" => 42);
$a = foo($array);

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> foo($a = ['te"st\'s' => 42]) %sbug00146.php:10
%w%f %w%d      >=> ['te"st\'s' => 42]
%w%f %w%d     -> xdebug_stop_trace() %sbug00146.php:12
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
