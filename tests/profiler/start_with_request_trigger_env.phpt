--TEST--
Starting Profiler: trigger, environment
--INI--
xdebug.mode=profile
xdebug.start_with_request=trigger
xdebug.collect_return=0
xdebug.collect_assignments=0
--ENV--
XDEBUG_PROFILE=anything
--FILE--
<?php
$fileName = xdebug_get_profiler_filename();

echo file_get_contents($fileName);
@unlink($fileName);
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sstart_with_request_trigger_env.php
part: 1
positions: line

events: Time_(µs) Memory_(bytes)
