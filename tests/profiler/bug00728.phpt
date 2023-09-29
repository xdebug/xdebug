--TEST--
Test for bug #728: Profiler reports __call() invocations confusingly/wrongly (>= PHP 8.1)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
--FILE--
<?php
require_once 'capture-profile.inc';

class bankaccount
{
	function __call( $foo, $bar )
	{
		var_dump( $foo, $bar );
	}
}

$b = new bankaccount;
$b->bar();

exit();
?>
--EXPECTF--
string(3) "bar"
array(0) {
}
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00728.php
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
fn=(4) php::var_dump
8 %d %d

fl=(3) %sbug00728.php
fn=(5) bankaccount->__call
6 %d %d
cfl=(1)
cfn=(4)
calls=1 0 0
8 %d %d

fl=(3)
fn=(6) {main}
1 %d %d
cfl=(2)
cfn=(3)
calls=1 0 0
2 %d %d
cfl=(3)
cfn=(5)
calls=1 0 0
13 %d %d

summary: %d %d
