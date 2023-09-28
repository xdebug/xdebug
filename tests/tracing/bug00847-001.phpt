--TEST--
Test for bug #847: %s doesn't work in xdebug.trace_output_name (auto_trace)
--INI--
xdebug.mode=trace
xdebug.trace_format=0
xdebug.start_with_request=yes
xdebug.trace_output_name=trace.%s
xdebug.trace_options=1
--FILE--
<?php
$trace_file = xdebug_get_tracefile_name();
echo $trace_file, "\n";
xdebug_stop_trace();
unlink($trace_file);
?>
--EXPECTF--
%strace.%s_tests_tracing_bug00847-001_php.xt%S
