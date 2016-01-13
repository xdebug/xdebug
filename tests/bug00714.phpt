--TEST--
Test for bug #714: Cachegrind files have huge (wrong) numbers in some lines
--INI--
xdebug.profiler_enable=1
--FILE--
<?php
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

echo file_get_contents(xdebug_get_profiler_filename());
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

events: Time

fl=(1) php:internal
fn=(1) php::sleep
2 10%d

fl=(2) %sbug00714.php
fn=(2) sleep1
2 %d
cfl=(1)
cfn=(1)
calls=1 0 0
2 10%d

fl=(1)
fn=(1)
3 10%d

fl=(2)
fn=(3) sleep10
3 %d
cfl=(1)
cfn=(1)
calls=1 0 0
3 10%d

fl=(1)
fn=(1)
4 20%d

fl=(2)
fn=(4) sleep20
4 %d
cfl=(1)
cfn=(1)
calls=1 0 0
4 20%d

fl=(1)
fn=(5) php::xdebug_get_profiler_filename
14 %d
