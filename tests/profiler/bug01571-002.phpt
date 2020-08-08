--TEST--
Test for bug #1571: Profiler doesn't show file/line for closures in namespaces
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
--FILE--
<?php
require_once 'capture-profile.inc';

require_once 'bug01571-002.inc';

exit();
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug01571-002.php
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

fl=(1)
fn=(4) php::usleep
4 %d %d

fl=(3) %sbug01571-002.inc
fn=(5) Testing\{closure:%sbug01571-002.inc:4-4}
4 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
4 %d %d

fl=(3)
fn=(6) require_once::%sbug01571-002.inc
1 %d %d
cfl=(3)
cfn=(5)
calls=1 0 0
5 %d %d

fl=(4) %sbug01571-002.php
fn=(7) {main}
1 %d %d
cfl=(2)
cfn=(3)
calls=1 0 0
2 %d %d
cfl=(3)
cfn=(6)
calls=1 0 0
4 %d %d

summary: %d %d
