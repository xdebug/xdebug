--TEST--
Test for variable member functions
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
--FILE--
<?php
	xdebug_start_trace();

	class a {

		function func_a1() {
		}

		function func_a2() {
		}

	}

	$A = new a;
	$A->func_a1();

	$a = 'func_a2';
	$A->$a();

	xdebug_dump_function_trace();
?>
--EXPECTF--
Function trace:
    %f      %d     -> a->func_a1() /%s/test15.php:15
    %f      %d     -> a->func_a2() /%s/test15.php:18
