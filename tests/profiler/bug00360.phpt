--TEST--
Test for bug #360: Function line number in profile not correct
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
--FILE--
<?php
require_once 'capture-profile.inc';

function func(){
	1+1;
}

func();

exit();
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00360.php
part: 1
positions: line

events: Time_(10ns) Memory_(bytes)

fl=(1) php:internal
fn=(1) php::xdebug_get_profiler_filename
2 %d %d

fl=(1)
fn=(2) php::register_shutdown_function
16 %d %d

fl=(2) %scapture-profile.inc
fn=(3) require_once::%scapture-profile.inc
1 %d %d
cfl=(1)
cfn=(1)
calls=1 0 0
2 %d %d
cfl=(1)
cfn=(2)
calls=1 0 0
16 %d %d

fl=(3) %sbug00360.php
fn=(4) func
4 %d %d

fl=(3)
fn=(5) {main}
1 %d %d
cfl=(2)
cfn=(3)
calls=1 0 0
2 %d %d
cfl=(3)
cfn=(4)
calls=1 0 0
8 %d %d

summary: %d %d
