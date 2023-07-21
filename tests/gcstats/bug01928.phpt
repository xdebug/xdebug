--TEST--
Test for bug #1928: xdebug_stop_gcstats() may return false as return type too
--INI--
xdebug.mode=gcstats
xdebug.start_with_request=no
--FILE--
<?php
var_dump(xdebug_stop_gcstats());
?>
--EXPECTF--
%sGarbage Collection statistics was not started in %s
bool(false)
