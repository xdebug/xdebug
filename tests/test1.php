<?php
	xdebug_start_trace();
	function foo ($a)
	{
		$c = new een();
		$b = $a * 3;
		$c->foo2 ($b, array ('blaat', 5, FALSE));
		return $b;
	}

	include ('test_class.php');

	echo foo (5);
	xdebug_dump_function_trace();
?>
