--TEST--
Text for crash bug in tracing to file
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
date.timezone=Europe/Oslo
--FILE--
<?php
	$tf = xdebug_start_trace('/tmp/bug00003.trace');
	strftime('%b %l %Y %H:%M:%S', 1061728888);
	xdebug_stop_trace();
	readfile($tf);
	unlink($tf);
?>
--EXPECTF--

TRACE START [%d-%d-%d %d:%d:%d]
    %f%w%d     -> strftime('%b %l %Y %H:%M:%S', 1061728888) /%s/bug00003.php:3
    %f%w%d     -> xdebug_stop_trace() /%s/bug00003.php:4
    %f%w%d
TRACE END   [%d-%d-%d %d:%d:%d]
