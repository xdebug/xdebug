--TEST--
Test computerized function traces (comp)
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=1
xdebug.dump_globals=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
--FILE--
<?php
require_once 'capture-trace.inc';

function foo( $a )
{
	// do nothing really
}

@foo( 42 );
@foo( "string" );
@foo( "string\nwith\nnewline" );
@foo( 1, false );
@foo( true, null );
@foo( "foo", "bar", 3.1415 );

xdebug_stop_trace();
?>
--EXPECTF--
Version: %s
File format: %d
TRACE START [%d-%d-%d %d:%d:%d.%d]
2	1	1	%f	%d
2	7	0	%f	%d	foo	1		%sfunctrace_comp.php	9	1	42
2	7	1	%f	%d
2	8	0	%f	%d	foo	1		%sfunctrace_comp.php	10	1	'string'
2	8	1	%f	%d
2	9	0	%f	%d	foo	1		%sfunctrace_comp.php	11	1	'string\nwith\nnewline'
2	9	1	%f	%d
2	10	0	%f	%d	foo	1		%sfunctrace_comp.php	12	2	1	FALSE
2	10	1	%f	%d
2	11	0	%f	%d	foo	1		%sfunctrace_comp.php	13	2	TRUE	NULL
2	11	1	%f	%d
2	12	0	%f	%d	foo	1		%sfunctrace_comp.php	14	3	'foo'	'bar'	3.1415
2	12	1	%f	%d
2	13	0	%f	%d	xdebug_stop_trace	0		%sfunctrace_comp.php	16	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
