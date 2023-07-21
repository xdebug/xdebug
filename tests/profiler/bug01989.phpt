--TEST--
Test for bug #1989: Profiling shows wrong class when parent keyword is used
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
--FILE--
<?php
require_once 'capture-profile.inc';

require_once 'bug01989.inc';

exit();
?>
--EXPECTF--
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug01989.php
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

fl=(3) %sbug01989.inc
fn=(4) A->__construct
5 %d %d

fl=(3)
fn=(5) B->__construct
12 %d %d
cfl=(3)
cfn=(4)
calls=1 0 0
14 %d %d

fl=(3)
fn=(6) require_once::%sbug01989.inc
1 %d %d
cfl=(3)
cfn=(5)
calls=1 0 0
18 %d %d

fl=(4) %sbug01989.php
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
