<?php
	function foo2 ($a, $b, $c)
	{
		echo xdebug_call_function(). "\n";
		echo xdebug_call_line(). "\n";
		echo xdebug_call_file(). "\n";
	}

	function foo ($a)
	{
		foo2 ($b, $a, array ('blaat', 5, FALSE));
	}

	function foo3 ($a)
	{
		foo ($a + 4);
	}

	echo foo3 (5);

?>
