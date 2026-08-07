--TEST--
Test computerized function traces (comp, with return)
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=1
xdebug.dump_globals=0
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
--FILE--
<?php
require_once 'capture-trace.inc';

function foo( $a )
{
	return $a;
}

$r = @foo( 42 );
$r = @foo( "string" );
$r = @foo( "string\nwi\th\nnewline" );
$r = @foo( 1, false );
$r = @foo( true, null );
$r = @foo( "foo", "bar", 3.1415 );

xdebug_stop_trace();
?>
--EXPECTF--
Version: %s
File format: %d
TRACE START [%d-%d-%d %d:%d:%d.%d]
2	%d	1	%f	%d
2	%d	0	%f	%d	foo	1		%sfunctrace_comp_r.php	9	1	42
2	%d	1	%f	%d
2	%d	R			42
2	%d	0	%f	%d	foo	1		%sfunctrace_comp_r.php	10	1	'string'
2	%d	1	%f	%d
2	%d	R			'string'
2	%d	0	%f	%d	foo	1		%sfunctrace_comp_r.php	11	1	'string\nwi\th\nnewline'
2	%d	1	%f	%d
2	%d	R			'string\nwi\th\nnewline'
2	%d	0	%f	%d	foo	1		%sfunctrace_comp_r.php	12	2	1	FALSE
2	%d	1	%f	%d
2	%d	R			1
2	%d	0	%f	%d	foo	1		%sfunctrace_comp_r.php	13	2	TRUE	NULL
2	%d	1	%f	%d
2	%d	R			TRUE
2	%d	0	%f	%d	foo	1		%sfunctrace_comp_r.php	14	3	'foo'	'bar'	3.1415
2	%d	1	%f	%d
2	%d	R			'foo'
2	%d	0	%f	%d	xdebug_stop_trace	0		%sfunctrace_comp_r.php	16	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
