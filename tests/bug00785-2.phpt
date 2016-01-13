--TEST--
Test for bug #785: Profiler does not handle call_user_func_array well
--INI--
xdebug.profiler_enable=1
--FILE--
<?php
require_once('bug00785-2.inc');
 
echo file_get_contents(xdebug_get_profiler_filename());
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00785-2.php
part: 1
positions: line

events: Time

fl=(1) php:internal
fn=(1) php::usleep
10 %d

fl=(2) %sbug00785-2.inc
fn=(2) nested2
8 %d
cfl=(1)
cfn=(1)
calls=1 0 0
10 %d

fl=(1)
fn=(3) php::call_user_func_array:{%sbug00785-2.inc:24}
24 %d
cfl=(2)
cfn=(2)
calls=1 0 0
24 %d

fl=(1)
fn=(1)
4 %d

fl=(2)
fn=(4) nested
2 %d
cfl=(1)
cfn=(1)
calls=1 0 0
4 %d

fl=(1)
fn=(5) php::call_user_func_array:{%sbug00785-2.inc:20}
20 %d
cfl=(2)
cfn=(4)
calls=1 0 0
20 %d

fl=(1)
fn=(1)
4 %d

fl=(2)
fn=(4)
2 %d
cfl=(1)
cfn=(1)
calls=1 0 0
4 %d

fl=(1)
fn=(5)
20 %d
cfl=(2)
cfn=(4)
calls=1 0 0
20 %d

fl=(1)
fn=(1)
4 %d

fl=(2)
fn=(4)
2 %d
cfl=(1)
cfn=(1)
calls=1 0 0
4 %d

fl=(1)
fn=(5)
20 %d
cfl=(2)
cfn=(4)
calls=1 0 0
20 %d

fl=(1)
fn=(1)
4 %d

fl=(2)
fn=(4)
2 %d
cfl=(1)
cfn=(1)
calls=1 0 0
4 %d

fl=(1)
fn=(5)
20 %d
cfl=(2)
cfn=(4)
calls=1 0 0
20 %d

fl=(1)
fn=(1)
4 %d

fl=(2)
fn=(4)
2 %d
cfl=(1)
cfn=(1)
calls=1 0 0
4 %d

fl=(1)
fn=(5)
20 %d
cfl=(2)
cfn=(4)
calls=1 0 0
20 %d

fl=(1)
fn=(1)
4 %d

fl=(2)
fn=(4)
2 %d
cfl=(1)
cfn=(1)
calls=1 0 0
4 %d

fl=(1)
fn=(5)
20 %d
cfl=(2)
cfn=(4)
calls=1 0 0
20 %d

fl=(2)
fn=(6) foo
15 %d
cfl=(1)
cfn=(5)
calls=1 0 0
20 %d
cfl=(1)
cfn=(5)
calls=1 0 0
20 %d
cfl=(1)
cfn=(5)
calls=1 0 0
20 %d
cfl=(1)
cfn=(5)
calls=1 0 0
20 %d
cfl=(1)
cfn=(5)
calls=1 0 0
20 %d
cfl=(1)
cfn=(5)
calls=1 0 0
20 %d

fl=(1)
fn=(1)
4 %d

fl=(2)
fn=(4)
2 %d
cfl=(1)
cfn=(1)
calls=1 0 0
4 %d

fl=(1)
fn=(7) php::call_user_func:{%sbug00785-2.inc:26}
26 %d
cfl=(2)
cfn=(4)
calls=1 0 0
26 %d

fl=(2)
fn=(8) require_once::%sbug00785-2.inc
1 %d
cfl=(1)
cfn=(3)
calls=1 0 0
24 %d
cfl=(2)
cfn=(6)
calls=1 0 0
25 %d
cfl=(1)
cfn=(7)
calls=1 0 0
26 %d

fl=(1)
fn=(9) php::xdebug_get_profiler_filename
4 %d
