--TEST--
Test with include file
--INI--
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
xdebug.show_mem_delta=0
--FILE--
<?php
	$tf = xdebug_start_trace(tempnam('/tmp', 'xdt'));
	function foo ($a)
	{
		$c = new een();
		$b = $a * 3;
		$c->foo2 ($b, array ('blaat', 5, FALSE));
		return $b;
	}

	include ('test_class.php');

	echo foo (5);
	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
15
TRACE START [%d-%d-%d %d:%d:%d]
    %f      %i     -> include('test_class.php') /%s/test1.php:11
    %f      %i     -> foo(5) /%s/test1.php:13
    %f      %i       -> een->foo2(15, array (0 => 'blaat', 1 => 5, 2 => FALSE)) /%s/test1.php:7
    %f      %i         -> een->hang() /%s/test_class.php:10
    %f      %d     -> file_get_contents('/tmp/%s') /%s/test1.php:14
