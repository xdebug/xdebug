--TEST--
Test for bug #360: Function line number in profile not correct
--INI--
xdebug.profiler_enable=1
--FILE--
<?php
function func(){
1+1;
}

func();

echo file_get_contents(xdebug_get_profiler_filename());
?>
--EXPECTF--
version: 1
creator: xdebug 2.%s
cmd: %sbug00360.php
part: 1
positions: line

events: Time

fl=%sbug00360.php
fn=func
2 %d

fl=php:internal
fn=php::xdebug_get_profiler_filename
8 %d
