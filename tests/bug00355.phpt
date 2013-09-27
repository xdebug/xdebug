--TEST--
Test for bug #355: Non-unique functions numbers in function traces
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=1
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.collect_vars=0
xdebug.collect_params=4
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE), XDEBUG_TRACE_COMPUTERIZED);

function foo()
{
    echo "Hi";
	echo strlen( "Hi" ), "\n";
}

function bar()
{
    echo "There\n";
	echo strlen( "There" ), "\n";
}

register_shutdown_function("bar");

foo();

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
Hi2
Version: 2.%d%s
File format: 3
TRACE START [%s]
2	2	1	%f	%d
2	3	0	%f	%d	register_shutdown_function	0		%sbug00355.php	16	1	'bar'
2	3	1	%f	%d
2	4	0	%f	%d	foo	1		%sbug00355.php	18	0
3	5	0	%f	%d	strlen	0		%sbug00355.php	7	1	'Hi'
3	5	1	%f	%d
2	4	1	%f	%d
2	6	0	%f	%d	xdebug_stop_trace	0		%sbug00355.php	20	0
			%f	%d
TRACE END   [%s]

There
5
