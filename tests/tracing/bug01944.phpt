--TEST--
Test for bug #1944: tracing is started without trigger, when profiler is also enabled
--INI--
xdebug.mode=trace,profile
xdebug.start_with_request=default
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
$fileName = xdebug_get_tracefile_name();
var_dump($fileName);
@unlink($fileName);

$fileName = xdebug_get_profiler_filename();
var_dump($fileName);
@unlink($fileName);
?>
--EXPECTF--
bool(false)
string(%d) "%s"
