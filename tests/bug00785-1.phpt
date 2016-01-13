--TEST--
Test for bug #785: Profiler does not handle closures well
--INI--
xdebug.profiler_enable=1
--FILE--
<?php
require_once('bug00785-1.inc');
 
echo file_get_contents(xdebug_get_profiler_filename());
?>
--EXPECTF--
int(21)
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00785-1.php
part: 1
positions: line

events: Time

fl=(1) php:internal
fn=(1) php::usleep
5 %d

fl=(2) %sbug00785-1.inc
fn=(2) {closure:%sbug00785-1.inc:5-5}
5 %d
cfl=(1)
cfn=(1)
calls=1 0 0
5 %d

fl=(1)
fn=(1)
5 %d

fl=(2)
fn=(2)
5 %d
cfl=(1)
cfn=(1)
calls=1 0 0
5 %d

fl=(1)
fn=(1)
5 %d

fl=(2)
fn=(2)
5 %d
cfl=(1)
cfn=(1)
calls=1 0 0
5 %d

fl=(1)
fn=(1)
5 %d

fl=(2)
fn=(2)
5 %d
cfl=(1)
cfn=(1)
calls=1 0 0
5 %d

fl=(1)
fn=(1)
5 %d

fl=(2)
fn=(2)
5 %d
cfl=(1)
cfn=(1)
calls=1 0 0
5 %d

fl=(1)
fn=(1)
5 %d

fl=(2)
fn=(2)
5 %d
cfl=(1)
cfn=(1)
calls=1 0 0
5 %d

fl=(1)
fn=(3) php::array_walk
7 %d
cfl=(2)
cfn=(2)
calls=1 0 0
7 %d
cfl=(2)
cfn=(2)
calls=1 0 0
7 %d
cfl=(2)
cfn=(2)
calls=1 0 0
7 %d
cfl=(2)
cfn=(2)
calls=1 0 0
7 %d
cfl=(2)
cfn=(2)
calls=1 0 0
7 %d
cfl=(2)
cfn=(2)
calls=1 0 0
7 %d

fl=(1)
fn=(1)
5 %d

fl=(2)
fn=(2)
5 %d
cfl=(1)
cfn=(1)
calls=1 0 0
5 %d

fl=(1)
fn=(4) php::var_dump
9 %d

fl=(2)
fn=(5) foo
2 %d
cfl=(1)
cfn=(3)
calls=1 0 0
7 %d
cfl=(2)
cfn=(2)
calls=1 0 0
9 %d
cfl=(1)
cfn=(4)
calls=1 0 0
9 %d

fl=(2)
fn=(6) require_once::%sbug00785-1.inc
1 %d
cfl=(2)
cfn=(5)
calls=1 0 0
12 %d

fl=(1)
fn=(7) php::xdebug_get_profiler_filename
4 %d
