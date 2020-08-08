--TEST--
Test for bug #785: Profiler does not handle call_user_func_array well
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
--FILE--
<?php
require_once 'capture-profile.inc';

require_once 'bug00785-002.inc';

exit();
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00785-002.php
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

fl=(1)
fn=(4) php::usleep
10 %d %d

fl=(3) %sbug00785-002.inc
fn=(5) nested2
8 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
10 %d %d

fl=(1)
fn=(6) php::call_user_func_array:{%sbug00785-002.inc:24}
24 %d %d
cfl=(3)
cfn=(5)
calls=1 0 0
24 %d %d

fl=(1)
fn=(4)
4 %d %d

fl=(3)
fn=(7) nested
2 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
4 %d %d

fl=(1)
fn=(8) php::call_user_func_array:{%sbug00785-002.inc:20}
20 %d %d
cfl=(3)
cfn=(7)
calls=1 0 0
20 %d %d

fl=(1)
fn=(4)
4 %d %d

fl=(3)
fn=(7)
2 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
4 %d %d

fl=(1)
fn=(8)
20 %d %d
cfl=(3)
cfn=(7)
calls=1 0 0
20 %d %d

fl=(1)
fn=(4)
4 %d %d

fl=(3)
fn=(7)
2 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
4 %d %d

fl=(1)
fn=(8)
20 %d %d
cfl=(3)
cfn=(7)
calls=1 0 0
20 %d %d

fl=(1)
fn=(4)
4 %d %d

fl=(3)
fn=(7)
2 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
4 %d %d

fl=(1)
fn=(8)
20 %d %d
cfl=(3)
cfn=(7)
calls=1 0 0
20 %d %d

fl=(1)
fn=(4)
4 %d %d

fl=(3)
fn=(7)
2 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
4 %d %d

fl=(1)
fn=(8)
20 %d %d
cfl=(3)
cfn=(7)
calls=1 0 0
20 %d %d

fl=(1)
fn=(4)
4 %d %d

fl=(3)
fn=(7)
2 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
4 %d %d

fl=(1)
fn=(8)
20 %d %d
cfl=(3)
cfn=(7)
calls=1 0 0
20 %d %d

fl=(3)
fn=(9) foo
15 %d %d
cfl=(1)
cfn=(8)
calls=1 0 0
20 %d %d
cfl=(1)
cfn=(8)
calls=1 0 0
20 %d %d
cfl=(1)
cfn=(8)
calls=1 0 0
20 %d %d
cfl=(1)
cfn=(8)
calls=1 0 0
20 %d %d
cfl=(1)
cfn=(8)
calls=1 0 0
20 %d %d
cfl=(1)
cfn=(8)
calls=1 0 0
20 %d %d

fl=(1)
fn=(4)
4 %d %d

fl=(3)
fn=(7)
2 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
4 %d %d

fl=(1)
fn=(10) php::call_user_func:{%sbug00785-002.inc:26}
26 %d %d
cfl=(3)
cfn=(7)
calls=1 0 0
26 %d %d

fl=(3)
fn=(11) require_once::%sbug00785-002.inc
1 %d %d
cfl=(1)
cfn=(6)
calls=1 0 0
24 %d %d
cfl=(3)
cfn=(9)
calls=1 0 0
25 %d %d
cfl=(1)
cfn=(10)
calls=1 0 0
26 %d %d

fl=(4) %sbug00785-002.php
fn=(12) {main}
1 %d %d
cfl=(2)
cfn=(3)
calls=1 0 0
2 %d %d
cfl=(3)
cfn=(11)
calls=1 0 0
4 %d %d

summary: %d %d
