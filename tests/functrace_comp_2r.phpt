--TEST--
Test computerized function traces (level2, comp, with return)
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=1
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.collect_vars=0
xdebug.collect_params=2
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
2	4	0	%f	%d	foo	1		%sfunctrace_comp_2r.php	9	1	long
2	4	1	%f	%d
2	4	R			long
2	5	0	%f	%d	foo	1		%sfunctrace_comp_2r.php	10	1	string(6)
2	5	1	%f	%d
2	5	R			string(6)
2	6	0	%f	%d	foo	1		%sfunctrace_comp_2r.php	11	1	string(19)
2	6	1	%f	%d
2	6	R			string(19)
2	7	0	%f	%d	foo	1		%sfunctrace_comp_2r.php	12	2	long	%r(bool|false)%r
2	7	1	%f	%d
2	7	R			long
2	8	0	%f	%d	foo	1		%sfunctrace_comp_2r.php	13	2	%r(bool|true)%r	null
2	8	1	%f	%d
2	8	R			%r(bool|true)%r
2	9	0	%f	%d	foo	1		%sfunctrace_comp_2r.php	14	3	string(3)	string(3)	double
2	9	1	%f	%d
2	9	R			string(3)
2	10	0	%f	%d	xdebug_stop_trace	0		%sfunctrace_comp_2r.php	16	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d]
