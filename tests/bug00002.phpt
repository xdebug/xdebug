--TEST--
Test for traces to file
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
--FILE--
<?php
	@unlink('/tmp/bug00002.trace');
	require 'bug00002.inc';

	$action = 'do_stuff';
	xdebug_start_trace('/tmp/bug00002.trace');
	$action();
	xdebug_stop_trace();
	readfile('/tmp/bug00002.trace');
	unlink('/tmp/bug00002.trace');
?>
--EXPECTF--

TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d     -> do_stuff() /%s/bug00002.php:7
    %f      %d     -> xdebug_stop_trace() /%s/bug00002.php:8
TRACE END   [%d-%d-%d %d:%d:%d]
