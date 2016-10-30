--TEST--
Test computerized function traces (level5, comp)
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=1
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.collect_vars=0
xdebug.collect_params=5
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE), XDEBUG_TRACE_COMPUTERIZED);

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
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
Version: %s
File format: %d
TRACE START [%d-%d-%d %d:%d:%d]
2	3	1	%f	%d
2	4	0	%f	%d	foo	1		%sfunctrace_comp_5.php	9	1	aTo0Mjs=
2	4	1	%f	%d
2	5	0	%f	%d	foo	1		%sfunctrace_comp_5.php	10	1	czo2OiJzdHJpbmciOw==
2	5	1	%f	%d
2	6	0	%f	%d	foo	1		%sfunctrace_comp_5.php	11	1	czoxOToic3RyaW5nCndpdGgKbmV3bGluZSI7
2	6	1	%f	%d
2	7	0	%f	%d	foo	1		%sfunctrace_comp_5.php	12	2	aToxOw==	YjowOw==
2	7	1	%f	%d
2	8	0	%f	%d	foo	1		%sfunctrace_comp_5.php	13	2	YjoxOw==	Tjs=
2	8	1	%f	%d
2	9	0	%f	%d	foo	1		%sfunctrace_comp_5.php	14	3	czozOiJmb28iOw==	czozOiJiYXIiOw==	ZDozLjE0MTU%s
2	9	1	%f	%d
2	10	0	%f	%d	xdebug_stop_trace	0		%sfunctrace_comp_5.php	16	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d]
