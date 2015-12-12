--TEST--
Text for crash bug in tracing to file
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
date.timezone=Europe/Oslo
--FILE--
<?php
	$tf = xdebug_start_trace(sys_get_temp_dir() . '/bug00003.trace');
	strftime('%b %l %Y %H:%M:%S', 1061728888);
	xdebug_stop_trace();
	readfile($tf);
	unlink($tf);
?>
--EXPECTF--

TRACE START [%d-%d-%d %d:%d:%d]
    %f%w%d     -> strftime('%b %l %Y %H:%M:%S', 1061728888) %sbug00003.php:3
    %f%w%d     -> xdebug_stop_trace() %sbug00003.php:4
    %f%w%d
TRACE END   [%d-%d-%d %d:%d:%d]
