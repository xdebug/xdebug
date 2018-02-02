--TEST--
Test computerized function traces (level5, comp, with return)
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=1
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.collect_vars=0
xdebug.collect_params=5
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE), XDEBUG_TRACE_COMPUTERIZED);

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
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
Version: %s
File format: %d
TRACE START [%d-%d-%d %d:%d:%d]
2	3	1	%f	%d
2	4	0	%f	%d	foo	1		%sfunctrace_comp_5r.php	9	1	aTo0Mjs=
2	4	1	%f	%d
2	4	R			aTo0Mjs=
2	5	0	%f	%d	foo	1		%sfunctrace_comp_5r.php	10	1	czo2OiJzdHJpbmciOw==
2	5	1	%f	%d
2	5	R			czo2OiJzdHJpbmciOw==
2	6	0	%f	%d	foo	1		%sfunctrace_comp_5r.php	11	1	czoxOToic3RyaW5nCndpCWgKbmV3bGluZSI7
2	6	1	%f	%d
2	6	R			czoxOToic3RyaW5nCndpCWgKbmV3bGluZSI7
2	7	0	%f	%d	foo	1		%sfunctrace_comp_5r.php	12	2	aToxOw==	YjowOw==
2	7	1	%f	%d
2	7	R			aToxOw==
2	8	0	%f	%d	foo	1		%sfunctrace_comp_5r.php	13	2	YjoxOw==	Tjs=
2	8	1	%f	%d
2	8	R			YjoxOw==
2	9	0	%f	%d	foo	1		%sfunctrace_comp_5r.php	14	3	czozOiJmb28iOw==	czozOiJiYXIiOw==	ZDozLjE0MTU%s
2	9	1	%f	%d
2	9	R			czozOiJmb28iOw==
2	10	0	%f	%d	xdebug_stop_trace	0		%sfunctrace_comp_5r.php	16	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d]
