--TEST--
Test for class members
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.auto_profile=0
--FILE--
<?php
	xdebug_start_trace();

	class aaa {
		var $c1;
		var $c2;
		function a1 () {
			return 'a1';
		}
		function a2 () {
			return 'a2';
		}
	}

	class bbb {
		function b1 () {
			return 'a1';
		}
		function b2 () {
			return 'a2';
		}
	}


	$a = new aaa;
	$b = new bbb;
	$a->a1();
	$b->b1();
	$a->a2();

	xdebug_dump_function_trace();
?>
--EXPECTF--
Function trace:
    %f      %d     -> aaa->a1() /%s/test7.php:27
    %f      %d     -> bbb->b1() /%s/test7.php:28
    %f      %d     -> aaa->a2() /%s/test7.php:29
