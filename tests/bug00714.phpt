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
creator: xdebug 2.%s
cmd: %sbug00714.php
part: 1
positions: line

events: Time

fl=php:internal
fn=php::sleep
2 1000%d

fl=%sbug00714.php
fn=sleep1
2 %d
cfl=php:internal
cfn=php::sleep
calls=1 0 0
2 1000%d

fl=php:internal
fn=php::sleep
3 1000%d

fl=%sbug00714.php
fn=sleep10
3 %d
cfl=php:internal
cfn=php::sleep
calls=1 0 0
3 1000%d

fl=php:internal
fn=php::sleep
4 2000%d

fl=%sbug00714.php
fn=sleep20
4 %d
cfl=php:internal
cfn=php::sleep
calls=1 0 0
4 2000%d

fl=php:internal
fn=php::xdebug_get_profiler_filename
14 %d
