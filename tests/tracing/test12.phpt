--TEST--
Test for complex parameters to functions
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.var_display_max_depth=3
xdebug.var_display_max_children=6
--FILE--
<?php
	$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

	function a ($a, $b, $h, &$i) {
		echo $a;
		return $a + $b;
	}

	$a = array (1, 2,3,4,5);
	$b = array ("h" => 9.12, $a, $a, $a, "p" => 9 - 0.12);
	echo a (5, 9.12, FALSE, $b), "\n";

	xdebug_stop_trace();
	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
514.12
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> a(5, 9.12, FALSE, array ('h' => 9.12, 0 => array (0 => 1, 1 => 2, 2 => 3, 3 => 4, 4 => 5), 1 => array (0 => 1, 1 => 2, 2 => 3, 3 => 4, 4 => 5), 2 => array (0 => 1, 1 => 2, 2 => 3, 3 => 4, 4 => 5), 'p' => 8.88)) %stest12.php:11
%w%f %w%d     -> xdebug_stop_trace() %stest12.php:13
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
