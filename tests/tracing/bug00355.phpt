--TEST--
Test for bug #355: Non-unique functions numbers in function traces
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=1
xdebug.dump_globals=0
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
require_once 'capture-trace.inc';

function foo()
{
    echo "Hi";
	echo strrev( "Hi" ), "\n";
}

function bar()
{
    echo "There\n";
	echo strrev( "There" ), "\n";
}

register_shutdown_function("bar");

foo();

xdebug_stop_trace();
?>
--EXPECTF--
HiiH
Version: %d.%s
File format: %d
TRACE START [%s]
2	1	1	%f	%d
2	7	0	%f	%d	register_shutdown_function	0		%sbug00355.php	16	1	'bar'
2	7	1	%f	%d
2	8	0	%f	%d	foo	1		%sbug00355.php	18	0
3	9	0	%f	%d	strrev	0		%sbug00355.php	7	1	'Hi'
3	9	1	%f	%d
2	8	1	%f	%d
2	10	0	%f	%d	xdebug_stop_trace	0		%sbug00355.php	20	0
			%f	%d
TRACE END   [%s]

There
erehT
