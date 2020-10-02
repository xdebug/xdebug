--TEST--
Test for variable function calls
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

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

	xdebug_stop_trace();
	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> foo1($a = 'test\'s') %svariable-functions.php:25
%w%f %w%d       -> addslashes($str%S = 'test\'s') %svariable-functions.php:6
%w%f %w%d     -> foo4($a = 'test\'s') %svariable-functions.php:27
%w%f %w%d       -> addslashes($str%S = 'test\'s') %svariable-functions.php:21
%w%f %w%d     -> foo2($a = 'test\'s') %svariable-functions.php:29
%w%f %w%d       -> addslashes($str%S = 'test\'s') %svariable-functions.php:11
%w%f %w%d     -> foo3($a = 'test\'s') %svariable-functions.php:31
%w%f %w%d       -> addslashes($str%S = 'test\'s') %svariable-functions.php:16
%w%f %w%d     -> xdebug_stop_trace() %svariable-functions.php:33
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
