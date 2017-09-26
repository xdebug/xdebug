--TEST--
Test computerized function traces (level3, comp)
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=1
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.collect_vars=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.force_error_reporting=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE), XDEBUG_TRACE_COMPUTERIZED);

function foo( $a )
{
    $aa = $a;
	// do nothing really
}

$bar = 42;
@foo( $bar );
$bar1 = "string";
@foo( $bar1 );
$bar2 = "string\nwith\nnewline";
@foo( $bar2 );
$bar3 = 1;
$bar4 = false;
@foo( $bar3, $bar4 );
$bar5 = true;
$bar6 = null;
@foo( $bar5, $bar6 );
$bar7 = "foo";
$bar8 = "bar";
$bar9 = 3.1415;
@foo( $bar7, $bar8, $bar9 );

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
Version: %s
File format: %d
TRACE START [%d-%d-%d %d:%d:%d]
2	3	1	%f	%d
1		2			{main}	1		%sfunctrace_comp_3a.php	2	$tf = '%s'
1		2			{main}	1		%sfunctrace_comp_3a.php	10	$bar = 42
2	4	0	%f	%d	foo	1		%sfunctrace_comp_3a.php	11	1	42
2		2			foo	1		%sfunctrace_comp_3a.php	6	$aa = 42
2	4	1	%f	%d
1		2			{main}	1		%sfunctrace_comp_3a.php	12	$bar1 = 'string'
2	5	0	%f	%d	foo	1		%sfunctrace_comp_3a.php	13	1	'string'
2		2			foo	1		%sfunctrace_comp_3a.php	6	$aa = 'string'
2	5	1	%f	%d
1		2			{main}	1		%sfunctrace_comp_3a.php	14	$bar2 = 'string\nwith\nnewline'
2	6	0	%f	%d	foo	1		%sfunctrace_comp_3a.php	15	1	'string\nwith\nnewline'
2		2			foo	1		%sfunctrace_comp_3a.php	6	$aa = 'string\nwith\nnewline'
2	6	1	%f	%d
1		2			{main}	1		%sfunctrace_comp_3a.php	16	$bar3 = 1
1		2			{main}	1		%sfunctrace_comp_3a.php	17	$bar4 = FALSE
2	7	0	%f	%d	foo	1		%sfunctrace_comp_3a.php	18	2	1	FALSE
2		2			foo	1		%sfunctrace_comp_3a.php	6	$aa = 1
2	7	1	%f	%d
1		2			{main}	1		%sfunctrace_comp_3a.php	19	$bar5 = TRUE
1		2			{main}	1		%sfunctrace_comp_3a.php	20	$bar6 = NULL
2	8	0	%f	%d	foo	1		%sfunctrace_comp_3a.php	21	2	TRUE	NULL
2		2			foo	1		%sfunctrace_comp_3a.php	6	$aa = TRUE
2	8	1	%f	%d
1		2			{main}	1		%sfunctrace_comp_3a.php	22	$bar7 = 'foo'
1		2			{main}	1		%sfunctrace_comp_3a.php	23	$bar8 = 'bar'
1		2			{main}	1		%sfunctrace_comp_3a.php	24	$bar9 = 3.1415
2	9	0	%f	%d	foo	1		%sfunctrace_comp_3a.php	25	3	'foo'	'bar'	3.1415
2		2			foo	1		%sfunctrace_comp_3a.php	6	$aa = 'foo'
2	9	1	%f	%d
2	10	0	%f	%d	xdebug_stop_trace	0		%sfunctrace_comp_3a.php	27	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d]
