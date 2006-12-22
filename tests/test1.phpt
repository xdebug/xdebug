--TEST--
Test with include file
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.var_display_max_depth=1
xdebug.var_display_max_children=3
--FILE--
<?php
	$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));
	function foo ($a)
	{
		$c = new een();
		$b = $a * 3;
		$c->foo2 ($b, array ('blaat', 5, FALSE));
		return $b;
	}

	include ('test_class.inc');

	echo foo(5), "\n";
	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
15
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> include(/%s/test_class.inc) /%s/test1.php:11
%w%f %w%d     -> foo(5) /%s/test1.php:13
%w%f %w%d       -> een->foo2(15, array (0 => 'blaat', 1 => 5, 2 => FALSE)) /%s/test1.php:7
%w%f %w%d         -> een->hang() /%s/test_class.inc:10
%w%f %w%d     -> file_get_contents('/tmp/%s') /%s/test1.php:14
