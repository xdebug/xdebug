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
unlink(xdebug_get_profiler_filename());
?>
--EXPECTF--
version: 1
creator: xdebug 2.%s
cmd: %sbug00639.php
part: 1
positions: line

events: Time

fl=%sbug00639-2.inc
fn=require::%sbug00639-2.inc
1 %d

fl=php:internal
fn=php::strlen
4 %d

fl=%sbug00639-2.inc
fn=func2
6 %d
cfl=php:internal
cfn=php::strlen
calls=1 0 0
4 %d

fl=php:internal
fn=php::strlen
4 %d

fl=%sbug00639-2.inc
fn=func2
7 %d
cfl=php:internal
cfn=php::strlen
calls=1 0 0
4 %d

fl=%sbug00639.php
fn=func1
10 %d
cfl=%sbug00639-2.inc
cfn=func2
calls=1 0 0
6 %d
cfl=%sbug00639-2.inc
cfn=func2
calls=1 0 0
7 %d

fl=php:internal
fn=php::strlen
4 %d

fl=%sbug00639-2.inc
fn=func2
11 %d
cfl=php:internal
cfn=php::strlen
calls=1 0 0
4 %d

fl=php:internal
fn=php::strlen
4 %d

fl=%sbug00639-2.inc
fn=func2
12 %d
cfl=php:internal
cfn=php::strlen
calls=1 0 0
4 %d

fl=php:internal
fn=php::strlen
4 %d

fl=%sbug00639-2.inc
fn=func2
13 %d
cfl=php:internal
cfn=php::strlen
calls=1 0 0
4 %d

fl=php:internal
fn=php::xdebug_get_profiler_filename
15 %d
