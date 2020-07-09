--TEST--
Starting Profiler: trigger, no environment
--INI--
xdebug.mode=profile
xdebug.start_with_request=trigger
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
$fileName = xdebug_get_profiler_filename();
var_dump($fileName);
?>
--EXPECT--
bool(false)
