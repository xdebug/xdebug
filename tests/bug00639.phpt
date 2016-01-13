--TEST--
Test for bug #639: Xdebug profiling: output not correct - missing 'cfl='
--INI--
xdebug.profiler_enable=1
--FILE--
<?php
require 'bug00639-2.inc';

function func1()
{
	func2();
	func2();
}

func1();
func2();
func2();
func2();

echo file_get_contents(xdebug_get_profiler_filename());
@unlink(xdebug_get_profiler_filename());
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00639.php
part: 1
positions: line

events: Time

fl=(1) %sbug00639-2.inc
fn=(1) require::%sbug00639-2.inc
1 %d

fl=(2) php:internal
fn=(2) php::strrev
4 %d

fl=(1)
fn=(3) func2
2 %d
cfl=(2)
cfn=(2)
calls=1 0 0
4 %d

fl=(2)
fn=(2)
4 %d

fl=(1)
fn=(3)
2 %d
cfl=(2)
cfn=(2)
calls=1 0 0
4 %d

fl=(3) %sbug00639.php
fn=(4) func1
4 %d
cfl=(1)
cfn=(3)
calls=1 0 0
6 %d
cfl=(1)
cfn=(3)
calls=1 0 0
7 %d

fl=(2)
fn=(2)
4 %d

fl=(1)
fn=(3)
2 %d
cfl=(2)
cfn=(2)
calls=1 0 0
4 %d

fl=(2)
fn=(2)
4 %d

fl=(1)
fn=(3)
2 %d
cfl=(2)
cfn=(2)
calls=1 0 0
4 %d

fl=(2)
fn=(2)
4 %d

fl=(1)
fn=(3)
2 %d
cfl=(2)
cfn=(2)
calls=1 0 0
4 %d

fl=(2)
fn=(5) php::xdebug_get_profiler_filename
15 %d
