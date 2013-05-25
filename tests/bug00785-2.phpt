--TEST--
Test for bug #785: Profiler does not handle call_user_func_array well
--INI--
xdebug.profiler_enable=1
--FILE--
<?php
require_once('bug00785-2.inc');
 
echo file_get_contents(xdebug_get_profiler_filename());
?>
--EXPECTF--
version: 1
creator: xdebug 2%s
cmd: %sbug00785-2.php
part: 1
positions: line

events: Time

fl=php:internal
fn=php::usleep
10 %d

fl=%sbug00785-2.inc
fn=nested2
8 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
10 %d

fl=php:internal
fn=php::call_user_func_array:{%sbug00785-2.inc:24}
24 %d
cfl=%sbug00785-2.inc
cfn=nested2
calls=1 0 0
24 %d

fl=php:internal
fn=php::usleep
4 %d

fl=%sbug00785-2.inc
fn=nested
2 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
4 %d

fl=php:internal
fn=php::call_user_func_array:{%sbug00785-2.inc:20}
20 %d
cfl=%sbug00785-2.inc
cfn=nested
calls=1 0 0
20 %d

fl=php:internal
fn=php::usleep
4 %d

fl=%sbug00785-2.inc
fn=nested
2 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
4 %d

fl=php:internal
fn=php::call_user_func_array:{%sbug00785-2.inc:20}
20 %d
cfl=%sbug00785-2.inc
cfn=nested
calls=1 0 0
20 %d

fl=php:internal
fn=php::usleep
4 %d

fl=%sbug00785-2.inc
fn=nested
2 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
4 %d

fl=php:internal
fn=php::call_user_func_array:{%sbug00785-2.inc:20}
20 %d
cfl=%sbug00785-2.inc
cfn=nested
calls=1 0 0
20 %d

fl=php:internal
fn=php::usleep
4 %d

fl=%sbug00785-2.inc
fn=nested
2 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
4 %d

fl=php:internal
fn=php::call_user_func_array:{%sbug00785-2.inc:20}
20 %d
cfl=%sbug00785-2.inc
cfn=nested
calls=1 0 0
20 %d

fl=php:internal
fn=php::usleep
4 %d

fl=%sbug00785-2.inc
fn=nested
2 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
4 %d

fl=php:internal
fn=php::call_user_func_array:{%sbug00785-2.inc:20}
20 %d
cfl=%sbug00785-2.inc
cfn=nested
calls=1 0 0
20 %d

fl=php:internal
fn=php::usleep
4 %d

fl=%sbug00785-2.inc
fn=nested
2 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
4 %d

fl=php:internal
fn=php::call_user_func_array:{%sbug00785-2.inc:20}
20 %d
cfl=%sbug00785-2.inc
cfn=nested
calls=1 0 0
20 %d

fl=%sbug00785-2.inc
fn=foo
15 %d
cfl=php:internal
cfn=php::call_user_func_array:{%sbug00785-2.inc:20}
calls=1 0 0
20 %d
cfl=php:internal
cfn=php::call_user_func_array:{%sbug00785-2.inc:20}
calls=1 0 0
20 %d
cfl=php:internal
cfn=php::call_user_func_array:{%sbug00785-2.inc:20}
calls=1 0 0
20 %d
cfl=php:internal
cfn=php::call_user_func_array:{%sbug00785-2.inc:20}
calls=1 0 0
20 %d
cfl=php:internal
cfn=php::call_user_func_array:{%sbug00785-2.inc:20}
calls=1 0 0
20 %d
cfl=php:internal
cfn=php::call_user_func_array:{%sbug00785-2.inc:20}
calls=1 0 0
20 %d

fl=php:internal
fn=php::usleep
4 %d

fl=%sbug00785-2.inc
fn=nested
2 %d
cfl=php:internal
cfn=php::usleep
calls=1 0 0
4 %d

fl=php:internal
fn=php::call_user_func:{%sbug00785-2.inc:26}
26 %d
cfl=%sbug00785-2.inc
cfn=nested
calls=1 0 0
26 %d

fl=%sbug00785-2.inc
fn=require_once::%sbug00785-2.inc
1 %d
cfl=php:internal
cfn=php::call_user_func_array:{%sbug00785-2.inc:24}
calls=1 0 0
24 %d
cfl=%sbug00785-2.inc
cfn=foo
calls=1 0 0
25 %d
cfl=php:internal
cfn=php::call_user_func:{%sbug00785-2.inc:26}
calls=1 0 0
26 %d

fl=php:internal
fn=php::xdebug_get_profiler_filename
4 %d
