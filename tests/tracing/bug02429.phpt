--TEST--
Test for bug #2429: Crash when trace_output_name setting is wrong
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=trace,develop
xdebug.start_with_request=yes
xdebug.trace_output_name=trace.%
xdebug.use_compression=0
log_errors=0
--FILE--
<?php
$tfn = xdebug_get_tracefile_name();

echo $tfn, "\n";

@unlink($tfn);
?>
--EXPECTF--
%strace.%.xt
