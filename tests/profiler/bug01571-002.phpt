--TEST--
Test for bug #1571: Profiler doesn't show file/line for closures in namespaces
--INI--
xdebug.profiler_enable=1
--FILE--
<?php
require_once('bug01571-002.inc');
 
echo file_get_contents(xdebug_get_profiler_filename());
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug01571-002.php
part: 1
positions: line

events: Time Memory

fl=(1) php:internal
fn=(1) php::usleep
4 %d %i

fl=(2) %sbug01571-002.inc
fn=(2) Testing\{closure:%sbug01571-002.inc:4-4}
4 %d %i
cfl=(1)
cfn=(1)
calls=1 0 0
4 %d %i

fl=(2)
fn=(3) require_once::%sbug01571-002.inc
1 %d %i
cfl=(2)
cfn=(2)
calls=1 0 0
5 %d %i

fl=(1)
fn=(4) php::xdebug_get_profiler_filename
4 %d %i
