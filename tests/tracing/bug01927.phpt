--TEST--
Test for bug #1927: Crash when calling xdebug_stop_trace without a trace in progress
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
--FILE--
<?php
var_dump(xdebug_stop_trace());
?>
--EXPECTF--
%sFunction trace was not started in %s
bool(false)
