--TEST--
Test for indirect function call
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

	function blaat ()
	{
	}

	$func = 'blaat';
	echo $func();

	xdebug_stop_trace();
	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> blaat() %stest11.php:9
%w%f %w%d      >=> NULL
%w%f %w%d     -> xdebug_stop_trace() %stest11.php:11
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
