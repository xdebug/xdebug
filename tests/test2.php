<?php
	include 'test2a.php';

	function foo4 ($a, $b, $c)
	{
		echo "In foo4: ".xdebug_call_function();
		echo "-".xdebug_call_line();
		echo "-".xdebug_call_file(). "\n";
		foo3 ($b, $c, $a);
	}

	function foo5 ($a, $b, $c)
	{
		echo "In foo5: ".xdebug_call_function();
		echo "-".xdebug_call_line();
		echo "-".xdebug_call_file(). "\n";
		foo4 ($b, $c, $a);
	}

	function foo6 ($a, $b, $c)
	{
		echo "In foo6: ".xdebug_call_function();
		echo "-".xdebug_call_line();
		echo "-".xdebug_call_file(). "\n";
		foo5 ($b, $c, $a);
	}


	echo foo6 (1,2,3);

?>
