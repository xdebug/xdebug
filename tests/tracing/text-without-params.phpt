--TEST--
Test for trace with xdebug.collect_params turned off (text)
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_assignments=0
xdebug.collect_params=0
xdebug.collect_return=0
xdebug.trace_format=0
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
%w%f %w%d     -> a() %stext-without-params.php:11
%w%f %w%d     -> xdebug_stop_trace() %stext-without-params.php:13
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
