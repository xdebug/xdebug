--TEST--
Test for variable function calls
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
--FILE--
<?php
	xdebug_start_trace();

	function foo1 ($a)
	{
		return addslashes ($a);
	}

	function foo2 ($a)
	{
		return addslashes ($a);
	}

	function foo3 ($a)
	{
		return addslashes ($a);
	}

	function foo4 ($a)
	{
		return addslashes ($a);
	}

	$f = 'foo1';
	$f('test\'s');
	$g = 'foo4';
	$g('test\'s');
	$h = 'foo2';
	$h('test\'s');
	$i = 'foo3';
	$i('test\'s');

	xdebug_dump_function_trace();
?>
--EXPECTF--
Function trace:
    %f      %d     -> foo1('test\'s') /%s/test13.php:25
    %f      %d       -> addslashes('test\'s') /%s/test13.php:6
    %f      %d     -> foo4('test\'s') /%s/test13.php:27
    %f      %d       -> addslashes('test\'s') /%s/test13.php:21
    %f      %d     -> foo2('test\'s') /%s/test13.php:29
    %f      %d       -> addslashes('test\'s') /%s/test13.php:11
    %f      %d     -> foo3('test\'s') /%s/test13.php:31
    %f      %d       -> addslashes('test\'s') /%s/test13.php:16
