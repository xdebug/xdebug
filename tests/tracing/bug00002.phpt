--TEST--
Test for traces to file
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
--FILE--
<?php
	require 'bug00002.inc';

	$action = 'do_stuff';
	$tf = xdebug_start_trace(sys_get_temp_dir() . '/' . uniqid('', true) . 'bug00002.trace');
	$action();
	xdebug_stop_trace();
	readfile($tf);
	unlink($tf);
?>
--EXPECTF--

TRACE START [%d-%d-%d %d:%d:%d.%d]
    %f%w%d     -> do_stuff() %sbug00002.php:6
    %f%w%d     -> xdebug_stop_trace() %sbug00002.php:7
    %f%w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
