--TEST--
Test for complex parameters to functions
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
--FILE--
<?php
	xdebug_start_trace();

	function a ($a, $b, $h, &$i) {
		echo $a;
		return $a + $b;
	}

	$a = array (1, 2,3,4,5);
	$b = array ("h" => 9.12, $a, $a, $a, "p" => 9 - 0.12);
	echo a (5, 9.12, FALSE, $b);

	xdebug_dump_function_trace();
?>
--EXPECTF--
514.12
Function trace:
    %f      %d     -> a(5, 9.12, FALSE, array ('h' => 9.12, 0 => array (0 => 1, 1 => 2, 2 => 3, 3 => 4, 4 => 5), 1 => array (0 => 1, 1 => 2, 2 => 3, 3 => 4, 4 => 5), 2 => array (0 => 1, 1 => 2, 2 => 3, 3 => 4, 4 => 5), 'p' => 8.88)) /%s/test12.php:11
