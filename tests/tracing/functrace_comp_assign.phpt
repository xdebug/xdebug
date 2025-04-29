--TEST--
Test computerized function traces (comp, with assignments)
--INI--
xdebug.mode=trace
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.trace_format=1
xdebug.dump_globals=0
xdebug.force_error_reporting=0
--FILE--
<?php
require_once 'capture-trace.inc';

function foo( $a )
{
	$aa = $a;
	$bar11 = 1;
	++$bar11;
	++$bar11;
	--$bar11;
	--$bar11;
	$bar11 += 1;
	$bar11 -= 1;
	$bar11 *= 2;
	$bar11 %= 2;
	$bar11 /= 2;
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

$bar10 = 1;
++$bar10;
++$bar10;
--$bar10;
--$bar10;
$bar10 += 1;
$bar10 -= 1;
$bar10 *= 2;
$bar10 %= 2;
$bar10 /= 2;

xdebug_stop_trace();
?>
--EXPECTF--
Version: %s
File format: %d
TRACE START [%d-%d-%d %d:%d:%d.%d]
2		A						%s	%d	$tf = '%s'
2	1	1	%f	%d
1		A						%sfunctrace_comp_assign.php	20	$bar = 42
2	%d	0	%f	%d	foo	1		%sfunctrace_comp_assign.php	21	1	42
2		A						%sfunctrace_comp_assign.php	6	$aa = 42
2		A						%sfunctrace_comp_assign.php	7	$bar11 = 1
2		A						%sfunctrace_comp_assign.php	8	++$bar11
2		A						%sfunctrace_comp_assign.php	9	++$bar11
2		A						%sfunctrace_comp_assign.php	10	--$bar11
2		A						%sfunctrace_comp_assign.php	11	--$bar11
2		A						%sfunctrace_comp_assign.php	12	$bar11 += 1
2		A						%sfunctrace_comp_assign.php	13	$bar11 -= 1
2		A						%sfunctrace_comp_assign.php	14	$bar11 *= 2
2		A						%sfunctrace_comp_assign.php	15	$bar11 %= 2
2		A						%sfunctrace_comp_assign.php	16	$bar11 /= 2
2	%d	1	%f	%d
1		A						%sfunctrace_comp_assign.php	22	$bar1 = 'string'
2	%d	0	%f	%d	foo	1		%sfunctrace_comp_assign.php	23	1	'string'
2		A						%sfunctrace_comp_assign.php	6	$aa = 'string'
2		A						%sfunctrace_comp_assign.php	7	$bar11 = 1
2		A						%sfunctrace_comp_assign.php	8	++$bar11
2		A						%sfunctrace_comp_assign.php	9	++$bar11
2		A						%sfunctrace_comp_assign.php	10	--$bar11
2		A						%sfunctrace_comp_assign.php	11	--$bar11
2		A						%sfunctrace_comp_assign.php	12	$bar11 += 1
2		A						%sfunctrace_comp_assign.php	13	$bar11 -= 1
2		A						%sfunctrace_comp_assign.php	14	$bar11 *= 2
2		A						%sfunctrace_comp_assign.php	15	$bar11 %= 2
2		A						%sfunctrace_comp_assign.php	16	$bar11 /= 2
2	%d	1	%f	%d
1		A						%sfunctrace_comp_assign.php	24	$bar2 = 'string\nwith\nnewline'
2	%d	0	%f	%d	foo	1		%sfunctrace_comp_assign.php	25	1	'string\nwith\nnewline'
2		A						%sfunctrace_comp_assign.php	6	$aa = 'string\nwith\nnewline'
2		A						%sfunctrace_comp_assign.php	7	$bar11 = 1
2		A						%sfunctrace_comp_assign.php	8	++$bar11
2		A						%sfunctrace_comp_assign.php	9	++$bar11
2		A						%sfunctrace_comp_assign.php	10	--$bar11
2		A						%sfunctrace_comp_assign.php	11	--$bar11
2		A						%sfunctrace_comp_assign.php	12	$bar11 += 1
2		A						%sfunctrace_comp_assign.php	13	$bar11 -= 1
2		A						%sfunctrace_comp_assign.php	14	$bar11 *= 2
2		A						%sfunctrace_comp_assign.php	15	$bar11 %= 2
2		A						%sfunctrace_comp_assign.php	16	$bar11 /= 2
2	%d	1	%f	%d
1		A						%sfunctrace_comp_assign.php	26	$bar3 = 1
1		A						%sfunctrace_comp_assign.php	27	$bar4 = FALSE
2	%d	0	%f	%d	foo	1		%sfunctrace_comp_assign.php	28	2	1	FALSE
2		A						%sfunctrace_comp_assign.php	6	$aa = 1
2		A						%sfunctrace_comp_assign.php	7	$bar11 = 1
2		A						%sfunctrace_comp_assign.php	8	++$bar11
2		A						%sfunctrace_comp_assign.php	9	++$bar11
2		A						%sfunctrace_comp_assign.php	10	--$bar11
2		A						%sfunctrace_comp_assign.php	11	--$bar11
2		A						%sfunctrace_comp_assign.php	12	$bar11 += 1
2		A						%sfunctrace_comp_assign.php	13	$bar11 -= 1
2		A						%sfunctrace_comp_assign.php	14	$bar11 *= 2
2		A						%sfunctrace_comp_assign.php	15	$bar11 %= 2
2		A						%sfunctrace_comp_assign.php	16	$bar11 /= 2
2	%d	1	%f	%d
1		A						%sfunctrace_comp_assign.php	29	$bar5 = TRUE
1		A						%sfunctrace_comp_assign.php	%d	$bar6 = NULL
2	%d	0	%f	%d	foo	1		%sfunctrace_comp_assign.php	%d	2	TRUE	NULL
2		A						%sfunctrace_comp_assign.php	6	$aa = TRUE
2		A						%sfunctrace_comp_assign.php	7	$bar11 = 1
2		A						%sfunctrace_comp_assign.php	8	++$bar11
2		A						%sfunctrace_comp_assign.php	9	++$bar11
2		A						%sfunctrace_comp_assign.php	10	--$bar11
2		A						%sfunctrace_comp_assign.php	11	--$bar11
2		A						%sfunctrace_comp_assign.php	12	$bar11 += 1
2		A						%sfunctrace_comp_assign.php	13	$bar11 -= 1
2		A						%sfunctrace_comp_assign.php	14	$bar11 *= 2
2		A						%sfunctrace_comp_assign.php	15	$bar11 %= 2
2		A						%sfunctrace_comp_assign.php	16	$bar11 /= 2
2	%d	1	%f	%d
1		A						%sfunctrace_comp_assign.php	%d	$bar7 = 'foo'
1		A						%sfunctrace_comp_assign.php	%d	$bar8 = 'bar'
1		A						%sfunctrace_comp_assign.php	%d	$bar9 = 3.1415
2	%d	0	%f	%d	foo	1		%sfunctrace_comp_assign.php	%d	3	'foo'	'bar'	3.1415
2		A						%sfunctrace_comp_assign.php	6	$aa = 'foo'
2		A						%sfunctrace_comp_assign.php	7	$bar11 = 1
2		A						%sfunctrace_comp_assign.php	8	++$bar11
2		A						%sfunctrace_comp_assign.php	9	++$bar11
2		A						%sfunctrace_comp_assign.php	10	--$bar11
2		A						%sfunctrace_comp_assign.php	11	--$bar11
2		A						%sfunctrace_comp_assign.php	12	$bar11 += 1
2		A						%sfunctrace_comp_assign.php	13	$bar11 -= 1
2		A						%sfunctrace_comp_assign.php	14	$bar11 *= 2
2		A						%sfunctrace_comp_assign.php	15	$bar11 %= 2
2		A						%sfunctrace_comp_assign.php	16	$bar11 /= 2
2	%d	1	%f	%d
1		A						%sfunctrace_comp_assign.php	%d	$bar10 = 1
1		A						%sfunctrace_comp_assign.php	%d	++$bar10
1		A						%sfunctrace_comp_assign.php	%d	++$bar10
1		A						%sfunctrace_comp_assign.php	40	--$bar10
1		A						%sfunctrace_comp_assign.php	41	--$bar10
1		A						%sfunctrace_comp_assign.php	42	$bar10 += 1
1		A						%sfunctrace_comp_assign.php	43	$bar10 -= 1
1		A						%sfunctrace_comp_assign.php	44	$bar10 *= 2
1		A						%sfunctrace_comp_assign.php	45	$bar10 %= 2
1		A						%sfunctrace_comp_assign.php	46	$bar10 /= 2
2	%d	0	%f	%d	xdebug_stop_trace	0		%sfunctrace_comp_assign.php	48	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
