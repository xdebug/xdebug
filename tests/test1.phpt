--TEST--
Test with include file
--INI--
xdebug.auto_trace=0
--FILE--
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
--EXPECTF--
15
Function trace:
    %f      %i     -> {main}() /%s/test1.php:0
    %f      %i     -> foo(5) /%s/test1.php:13
    %f      %i       -> een->foo2(15, array (0 => 'blaat', 1 => 5, 2 => FALSE)) /%s/test1.php:7
    %f      %i         -> een->hang() /%s/test_class.php:10

