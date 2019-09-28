--TEST--
xdebug_start_trace() without filename
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.trace_output_name=trace.%c
--FILE--
<?php
	$tf = xdebug_start_trace();
	echo $tf, "\n";
	xdebug_stop_trace();
	unlink($tf);
?>
--EXPECTF--
%strace.%d.xt
