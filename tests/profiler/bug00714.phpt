--TEST--
Test for bug #714: Cachegrind files have huge (wrong) numbers in some lines
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('slow');
?>
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
--FILE--
<?php
require_once 'capture-profile.inc';

function sleep1() { sleep(1); }
function sleep10() { sleep(1); }
function sleep20() { sleep(2); }

echo "Sleeping 1\n";
sleep1();
echo "Sleeping 10\n";
sleep10();
echo "Sleeping 20\n";
sleep20();
echo "DONE\n\n";

exit();
?>
--EXPECTF--
Sleeping 1
Sleeping 10
Sleeping 20
DONE

version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00714.php
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
fn=(4) php::sleep
4 %r(1\d{8}|9\d{7})%r %d

fl=(3) %sbug00714.php
fn=(5) sleep1
4 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
4 %r(1\d{8}|9\d{7})%r %d

fl=(1)
fn=(4)
5 1%r(\d{8})%r %d

fl=(3)
fn=(6) sleep10
5 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
5 1%r(\d{8})%r 0

fl=(1)
fn=(4)
6 2%r(\d{8})%r 0

fl=(3)
fn=(7) sleep20
6 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
6 2%r(\d{8})%r 0

fl=(3)
fn=(8) {main}
1 %d %d
cfl=(2)
cfn=(3)
calls=1 0 0
2 %d %d
cfl=(3)
cfn=(5)
calls=1 0 0
9 1%r(\d{8})%r %d
cfl=(3)
cfn=(6)
calls=1 0 0
11 1%r(\d{8})%r %d
cfl=(3)
cfn=(7)
calls=1 0 0
13 2%r(\d{8})%r %d

summary: %d %d
