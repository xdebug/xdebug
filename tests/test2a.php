<?php
	function foo1 ($a, $b, $c)
	{
		echo "In foo1: ".xdebug_call_function();
		echo "-".xdebug_call_line();
		echo "-".xdebug_call_file(). "\n";
		var_dump (xdebug_get_function_stack());
	}

	function foo2 ($a, $b, $c)
	{
		echo "In foo2: ".xdebug_call_function();
		echo "-".xdebug_call_line();
		echo "-".xdebug_call_file(). "\n";
		foo1 ($b, $c, $a);
	}

	function foo3 ($a, $b, $c)
	{
		echo "In foo3: ".xdebug_call_function();
		echo "-".xdebug_call_line();
		echo "-".xdebug_call_file(). "\n";
		foo2 ($b, $c, $a);
	}

?>
