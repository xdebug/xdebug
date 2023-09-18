--TEST--
Test for trace with xdebug.collect_params turned off (comp)
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_assignments=0
xdebug.collect_params=0
xdebug.collect_return=0
xdebug.trace_format=1
--FILE--
<?php
require_once 'capture-trace.inc';

function a ($a, $b, $h, &$i) {
	echo $a;
	return $a + $b;
}

$a = array (1, 2,3,4,5);
$b = array ("h" => 9.12, $a, $a, $a, "p" => 9 - 0.12);
echo a (5, 9.12, FALSE, $b), "\n";

xdebug_stop_trace();
?>
--EXPECTF--
514.12
Version: %s
File format: %d
TRACE START [%d-%d-%d %d:%d:%d.%d]
2	1	1	%f	%d
2	7	0	%f	%d	a	1		%scomp-without-params.php	11
2	7	1	%f	%d
2	8	0	%f	%d	xdebug_stop_trace	0		%scomp-without-params.php	13
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
