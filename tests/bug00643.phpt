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
creator: xdebug 2.%s
cmd: %sbug00643.php
part: 1
positions: line

events: Time

fl=%sbug00643-t2.inc
fn=require_once::%sbug00643-t2.inc
1 %d

fl=%sbug00643-t1.inc
fn=require_once::%sbug00643-t1.inc
1 %d
cfl=%sbug00643-t2.inc
cfn=require_once::%sbug00643-t2.inc
calls=1 0 0
2 %d

fl=php:internal
fn=php::count
23 %d

fl=php:internal
fn=php::count
12 %d

fl=php:internal
fn=php::is_array
12 %d

fl=%sbug00643-t2.inc
fn=errors_fatal
10 %d
cfl=php:internal
cfn=php::count
calls=1 0 0
12 %d
cfl=php:internal
cfn=php::is_array
calls=1 0 0
12 %d

fl=%sbug00643-t1.inc
fn=t1
20 %d
cfl=php:internal
cfn=php::count
calls=1 0 0
23 %d
cfl=%sbug00643-t2.inc
cfn=errors_fatal
calls=1 0 0
23 %d

fl=php:internal
fn=php::xdebug_get_profiler_filename
7 %d
