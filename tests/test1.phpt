--TEST--
Test with include file
--INI--
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.var_display_max_depth=1
xdebug.var_display_max_children=3
--FILE--
<?php
	$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));
	function foo ($a)
	{
		$c = new een();
		$b = $a * 3;
		$c->foo2 ($b, array ('blaat', 5, FALSE));
		return $b;
	}

	include ('test_class.inc');

	echo foo(5), "\n";
	xdebug_stop_trace();
	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
15
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> include(%stest_class.inc) %stest1.php:11
%w%f %w%d     -> foo(5) %stest1.php:13
%w%f %w%d       -> een->foo2(15, array (0 => 'blaat', 1 => 5, 2 => FALSE)) %stest1.php:7
%w%f %w%d         -> een->hang() %stest_class.inc:10
%w%f %w%d     -> xdebug_stop_trace() %stest1.php:14
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
