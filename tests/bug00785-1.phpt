--TEST--
Test for bug #785: Profiler does not handle closures well (>= PHP 5.3)
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.3", '>=')) echo "skip >= PHP 5.3 needed\n"; ?>
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
creator: xdebug 2%s
cmd: %sbug00785-1.php
part: 1
positions: line

events: Time

fl=php:internal
fn=php::usleep
5 %d

fl=%sbug00785-1.inc
fn={closure:%sbug00785-1.inc:5-5}
5 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
5 %d

fl=php:internal
fn=php::usleep
5 %d

fl=%sbug00785-1.inc
fn={closure:%sbug00785-1.inc:5-5}
5 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
5 %d

fl=php:internal
fn=php::usleep
5 %d

fl=%sbug00785-1.inc
fn={closure:%sbug00785-1.inc:5-5}
5 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
5 %d

fl=php:internal
fn=php::usleep
5 %d

fl=%sbug00785-1.inc
fn={closure:%sbug00785-1.inc:5-5}
5 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
5 %d

fl=php:internal
fn=php::usleep
5 %d

fl=%sbug00785-1.inc
fn={closure:%sbug00785-1.inc:5-5}
5 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
5 %d

fl=php:internal
fn=php::usleep
5 %d

fl=%sbug00785-1.inc
fn={closure:%sbug00785-1.inc:5-5}
5 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
5 %d

fl=php:internal
fn=php::array_walk
7 %d
cfl=%sbug00785-1.inc
cfn={closure:%sbug00785-1.inc:5-5}
calls=1 0 0
7 %d
cfl=%sbug00785-1.inc
cfn={closure:%sbug00785-1.inc:5-5}
calls=1 0 0
7 %d
cfl=%sbug00785-1.inc
cfn={closure:%sbug00785-1.inc:5-5}
calls=1 0 0
7 %d
cfl=%sbug00785-1.inc
cfn={closure:%sbug00785-1.inc:5-5}
calls=1 0 0
7 %d
cfl=%sbug00785-1.inc
cfn={closure:%sbug00785-1.inc:5-5}
calls=1 0 0
7 %d
cfl=%sbug00785-1.inc
cfn={closure:%sbug00785-1.inc:5-5}
calls=1 0 0
7 %d

fl=php:internal
fn=php::usleep
5 %d

fl=%sbug00785-1.inc
fn={closure:%sbug00785-1.inc:5-5}
5 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
5 %d

fl=php:internal
fn=php::var_dump
9 %d

fl=%sbug00785-1.inc
fn=foo
2 %d
cfl=php:internal
cfn=php::array_walk
calls=1 0 0
7 %d
cfl=%sbug00785-1.inc
cfn={closure:%sbug00785-1.inc:5-5}
calls=1 0 0
9 %d
cfl=php:internal
cfn=php::var_dump
calls=1 0 0
9 %d

fl=%sbug00785-1.inc
fn=require_once::%sbug00785-1.inc
1 %d
cfl=%sbug00785-1.inc
cfn=foo
calls=1 0 0
12 %d

fl=php:internal
fn=php::xdebug_get_profiler_filename
4 %d
