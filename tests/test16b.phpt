--TEST--
Test for overloaded member functions / classes (ZE2)
--SKIPIF--
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
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

	class b extends a {

		function func_b1() {
		}

		function func_b2() {
		}

	}

	$B = new b;
	$B->func_a1();
	$B->func_b1();

	xdebug_dump_function_trace();
?>
--EXPECTF--
Function trace:
    %f      %d     -> a->func_a1() /%s/test16b.php:25
    %f      %d     -> b->func_b1() /%s/test16b.php:26
