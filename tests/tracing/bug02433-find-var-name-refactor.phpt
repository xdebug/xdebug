--TEST--
Test for bug #XXXX: Crash with tracing
--INI--
xdebug.mode=trace
xdebug.start_with_request=yes
xdebug.collect_assignments=1
xdebug.collect_return=0
xdebug.trace_format=0
xdebug.use_compression=0
xdebug.trace_output_name=find-var-name_01.%p
--FILE--
<?php
eval('$x = "abs"; $x[0] = "d";');

$tfn = xdebug_get_tracefile_name();
@unlink($tfn);
?>
x
--EXPECTF--
x
