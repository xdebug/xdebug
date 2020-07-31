--TEST--
Test for bug #639: Xdebug profiling: output not correct - missing 'cfl='
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
--FILE--
<?php
require_once 'capture-profile.inc';
require_once 'bug00639.inc';

function func1()
{
	func2();
	func2();
}

func1();
func2();
func2();
func2();

exit();
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00639.php
part: 1
positions: line

events: Time_(10ns) Memory_(bytes)

fl=(1) php:internal
fn=(1) php::xdebug_get_profiler_filename
2 %d %d

fl=(1)
fn=(2) php::register_shutdown_function
10 %d %d

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
10 %d %d

fl=(3) %sbug00639.inc
fn=(4) require_once::%sbug00639.inc
1 %d %d

fl=(1)
fn=(5) php::strrev
4 %d %d

fl=(3)
fn=(6) func2
2 %d %d
cfl=(1)
cfn=(5)
calls=1 0 0
4 %d %d

fl=(1)
fn=(5)
4 %d %d

fl=(3)
fn=(6)
2 %d %d
cfl=(1)
cfn=(5)
calls=1 0 0
4 %d %d

fl=(4) %sbug00639.php
fn=(7) func1
5 %d %d
cfl=(3)
cfn=(6)
calls=1 0 0
7 %d %d
cfl=(3)
cfn=(6)
calls=1 0 0
8 %d %d

fl=(1)
fn=(5)
4 %d %d

fl=(3)
fn=(6)
2 %d %d
cfl=(1)
cfn=(5)
calls=1 0 0
4 %d %d

fl=(1)
fn=(5)
4 %d %d

fl=(3)
fn=(6)
2 %d %d
cfl=(1)
cfn=(5)
calls=1 0 0
4 %d %d

fl=(1)
fn=(5)
4 %d %d

fl=(3)
fn=(6)
2 %d %d
cfl=(1)
cfn=(5)
calls=1 0 0
4 %d %d

fl=(4)
fn=(8) {main}
1 %d %d
cfl=(2)
cfn=(3)
calls=1 0 0
2 %d %d
cfl=(3)
cfn=(4)
calls=1 0 0
3 %d %d
cfl=(4)
cfn=(7)
calls=1 0 0
11 %d %d
cfl=(3)
cfn=(6)
calls=1 0 0
12 %d %d
cfl=(3)
cfn=(6)
calls=1 0 0
13 %d %d
cfl=(3)
cfn=(6)
calls=1 0 0
14 %d %d

summary: %d %d
