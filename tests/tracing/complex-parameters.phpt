--TEST--
Test for complex parameters to functions
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
xdebug.var_display_max_depth=3
xdebug.var_display_max_children=6
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
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> a($a = 5, $b = 9.12, $h = FALSE, $i = ['h' => 9.12, 0 => [0 => 1, 1 => 2, 2 => 3, 3 => 4, 4 => 5], 1 => [0 => 1, 1 => 2, 2 => 3, 3 => 4, 4 => 5], 2 => [0 => 1, 1 => 2, 2 => 3, 3 => 4, 4 => 5], 'p' => 8.88]) %scomplex-parameters.php:11
%w%f %w%d     -> xdebug_stop_trace() %scomplex-parameters.php:13
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
