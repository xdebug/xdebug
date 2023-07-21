--TEST--
Test for bug #785: Profiler does not handle closures well
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
--FILE--
<?php
require_once 'capture-profile.inc';

require_once 'bug00785-001.inc';

exit();
?>
--EXPECTF--
int(21)
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00785-001.php
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

fl=(1)
fn=(4) php::usleep
5 %d %d

fl=(3) %sbug00785-001.inc
fn=(5) {closure:%sbug00785-001.inc:5-5}
5 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
5 %d %d

fl=(1)
fn=(4)
5 %d %d

fl=(3)
fn=(5)
5 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
5 %d %d

fl=(1)
fn=(4)
5 %d %d

fl=(3)
fn=(5)
5 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
5 %d %d

fl=(1)
fn=(4)
5 %d %d

fl=(3)
fn=(5)
5 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
5 %d %d

fl=(1)
fn=(4)
5 %d %d

fl=(3)
fn=(5)
5 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
5 %d %d

fl=(1)
fn=(4)
5 %d %d

fl=(3)
fn=(5)
5 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
5 %d %d

fl=(1)
fn=(6) php::array_walk
7 %d %d
cfl=(3)
cfn=(5)
calls=1 0 0
7 %d %d
cfl=(3)
cfn=(5)
calls=1 0 0
7 %d %d
cfl=(3)
cfn=(5)
calls=1 0 0
7 %d %d
cfl=(3)
cfn=(5)
calls=1 0 0
7 %d %d
cfl=(3)
cfn=(5)
calls=1 0 0
7 %d %d
cfl=(3)
cfn=(5)
calls=1 0 0
7 %d %d

fl=(1)
fn=(4)
5 %d %d

fl=(3)
fn=(5)
5 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
5 %d %d

fl=(1)
fn=(7) php::var_dump
9 %d %d

fl=(3)
fn=(8) foo
2 %d %d
cfl=(1)
cfn=(6)
calls=1 0 0
7 %d %d
cfl=(3)
cfn=(5)
calls=1 0 0
9 %d %d
cfl=(1)
cfn=(7)
calls=1 0 0
9 %d %d

fl=(3)
fn=(9) require_once::%sbug00785-001.inc
1 %d %d
cfl=(3)
cfn=(8)
calls=1 0 0
12 %d %d

fl=(4) %sbug00785-001.php
fn=(10) {main}
1 %d %d
cfl=(2)
cfn=(3)
calls=1 0 0
2 %d %d
cfl=(3)
cfn=(9)
calls=1 0 0
4 %d %d

summary: %d %d
