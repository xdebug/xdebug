--TEST--
Test for bug #360: Function line number in profile not correct
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
--FILE--
<?php
function func(){
1+1;
}

func();

echo file_get_contents(xdebug_get_profiler_filename());
@unlink(xdebug_get_profiler_filename());
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00360.php
part: 1
positions: line

events: Time_(µs) Memory_(bytes)

fl=(2) %sbug00360.php
fn=(1) func
2 %d %d

fl=(1) php:internal
fn=(2) php::xdebug_get_profiler_filename
8 %d %d
