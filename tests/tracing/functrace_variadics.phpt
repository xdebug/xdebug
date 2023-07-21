--TEST--
Test for function traces with variadics
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
--FILE--
<?php
require_once 'capture-trace.inc';

function foo( $a, ...$b )
{
	// do nothing really
}

foo( 42 );
foo( 1, false );
foo( "foo", "bar", 3.1415 );

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> foo($a = 42) %sfunctrace_variadics.php:9
%w%f %w%d     -> foo($a = 1, ...$b = variadic(0 => FALSE)) %sfunctrace_variadics.php:10
%w%f %w%d     -> foo($a = 'foo', ...$b = variadic(0 => 'bar', 1 => 3.1415)) %sfunctrace_variadics.php:11
%w%f %w%d     -> xdebug_stop_trace() %sfunctrace_variadics.php:13
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
