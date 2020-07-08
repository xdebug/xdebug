--TEST--
Starting Profiler: never, trigger match
--INI--
xdebug.mode=profile
xdebug.start_with_request=no
xdebug.trigger_value=something
xdebug.collect_return=0
xdebug.collect_assignments=0
--ENV--
XDEBUG_PROFILE=something
--FILE--
<?php
$fileName = xdebug_get_profiler_filename();
var_dump($fileName);
?>
--EXPECT--
bool(false)
