--TEST--
Test for variable member functions
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
--FILE--
<?php
	$tf = xdebug_start_trace(tempnam('/tmp', 'xdt'));

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

	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d     -> a->func_a1() /%s/test15.php:15
    %f      %d     -> a->func_a2() /%s/test15.php:18
    %f      %d     -> file_get_contents('/tmp/%s') /%s/test15.php:20
