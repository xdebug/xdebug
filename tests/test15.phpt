--TEST--
Test for variable member functions
--INI--
xdebug.enable=1
xdebug.auto_trace=0
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
    %f      %d     -> a->func_a1() /%s/phpt.%x:15
    %f      %d     -> a->func_a2() /%s/phpt.%x:18
