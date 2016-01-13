--TEST--
Test for bug #643: Profiler gets line numbers wrong
--INI--
xdebug.profiler_enable=1
--FILE--
<?php
require_once('bug00643-t1.inc');
 
$a = array("testing");
 
t1();
echo file_get_contents(xdebug_get_profiler_filename());
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00643.php
part: 1
positions: line

events: Time

fl=(1) %sbug00643-t2.inc
fn=(1) require_once::%sbug00643-t2.inc
1 %d

fl=(2) %sbug00643-t1.inc
fn=(2) require_once::%sbug00643-t1.inc
1 %d
cfl=(1)
cfn=(1)
calls=1 0 0
2 %d

fl=(3) php:internal
fn=(3) php::count
23 %d

fl=(3)
fn=(3)
12 %d

fl=(3)
fn=(4) php::is_array
12 %d

fl=(1)
fn=(5) errors_fatal
10 %d
cfl=(3)
cfn=(3)
calls=1 0 0
12 %d
cfl=(3)
cfn=(4)
calls=1 0 0
12 %d

fl=(2)
fn=(6) t1
20 %d
cfl=(3)
cfn=(3)
calls=1 0 0
23 %d
cfl=(1)
cfn=(5)
calls=1 0 0
23 %d

fl=(3)
fn=(7) php::xdebug_get_profiler_filename
7 %d
