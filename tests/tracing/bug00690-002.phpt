--TEST--
Test for bug #690: Function traces are not appended to file when xdebug_start_trace() is used with xdebug.trace_options=1 (step 2 of 2)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('unparallel');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.trace_output_name=trace.bug690
xdebug.trace_options=1
xdebug.trace_format=0
xdebug.collect_assignments=1
xdebug.collect_return=1
--FILE--
<?php
xdebug_start_trace();
$trace_file = xdebug_get_tracefile_name();
echo $trace_file, "\n";
echo file_get_contents($trace_file);
xdebug_stop_trace();
unlink($trace_file);
?>
--EXPECTF--
%strace.bug690.xt
DONE
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> xdebug_get_tracefile_name() %sbug00690-002.php:3
%w%f %w%d      >=> '%strace.bug690.xt'
                           => $trace_file = '%strace.bug690.xt' %sbug00690-002.php:3
%w%f %w%d     -> file_get_contents($filename = '%strace.bug690.xt') %sbug00690-002.php:5

