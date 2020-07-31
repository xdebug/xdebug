--TEST--
Test for bug #643: Profiler gets line numbers wrong
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
--FILE--
<?php
require_once 'capture-profile.inc';

require_once 'bug00643-t1.inc';
 
$a = array("testing");
 
t1();

exit();
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00643.php
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

fl=(3) %sbug00643-t2.inc
fn=(4) require_once::%sbug00643-t2.inc
1 %d %d

fl=(4) %sbug00643-t1.inc
fn=(5) require_once::%sbug00643-t1.inc
1 %d %d
cfl=(3)
cfn=(4)
calls=1 0 0
2 %d %d

fl=(1)
fn=(6) php::array_count_values
23 %d %d

fl=(1)
fn=(6)
12 %d %d

fl=(1)
fn=(7) php::is_array
12 %d %d

fl=(3)
fn=(8) errors_fatal
10 %d %d
cfl=(1)
cfn=(6)
calls=1 0 0
12 %d %d
cfl=(1)
cfn=(7)
calls=1 0 0
12 %d %d

fl=(4)
fn=(9) t1
20 %d %d
cfl=(1)
cfn=(6)
calls=1 0 0
23 %d %d
cfl=(3)
cfn=(8)
calls=1 0 0
23 %d %d

fl=(5) %sbug00643.php
fn=(10) {main}
1 %d %d
cfl=(2)
cfn=(3)
calls=1 0 0
2 %d %d
cfl=(4)
cfn=(5)
calls=1 0 0
4 %d %d
cfl=(4)
cfn=(9)
calls=1 0 0
8 %d %d

summary: %d %d
