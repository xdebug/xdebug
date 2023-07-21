--TEST--
xdebug_start_trace() without filename
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.trace_format=0
xdebug.trace_output_name=trace.%c
xdebug.use_compression=0
--FILE--
<?php
	$tf = xdebug_start_trace();
	echo $tf, "\n";
	xdebug_stop_trace();
	unlink($tf);
?>
--EXPECTF--
%strace.%d.xt
