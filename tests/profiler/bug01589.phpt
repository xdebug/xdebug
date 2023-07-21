--TEST--
Test for bug #1589: function names used in auto_prepend_file missing
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!win');
?>
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
auto_prepend_file=tests/profiler/bug01589-prepend.inc
auto_append_file=tests/profiler/bug01589-append.inc
--FILE--
<?php
if (strlen($foobar)>0) {
}
else {
}

$abc = substr('abc', 1);


?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug01589-prepend.inc
part: 1
positions: line

events: Time_(10ns) Memory_(bytes)

fl=(1) php:internal
fn=(1) php::substr
17 %d %d

fl=(1)
fn=(2) php::xdebug_get_profiler_filename
23 %d %d

fl=(1)
fn=(3) php::register_shutdown_function
24 %d %d

fl=(2) %sbug01589-prepend.inc
fn=(4) {main}
1 %d %d
cfl=(1)
cfn=(1)
calls=1 0 0
17 %d %d
cfl=(1)
cfn=(2)
calls=1 0 0
23 %d %d
cfl=(1)
cfn=(3)
calls=1 0 0
24 %d %d

fl=(1)
fn=(1)
7 %d %d

fl=(3) %sbug01589.php
fn=(4)
1 %d %d
cfl=(1)
cfn=(1)
calls=1 0 0
7 %d %d

fl=(1)
fn=(1)
3 %d %d

fl=(4) %sbug01589-append.inc
fn=(4)
1 %d %d
cfl=(1)
cfn=(1)
calls=1 0 0
3 %d %d

summary: %d %d
