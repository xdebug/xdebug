--TEST--
Test for bug #728: Profiler reports __call() invocations confusingly/wrongly (>= PHP 7.1, < PHP 7.4)
--SKIPIF--
<?php
require __DIR__ . '/utils.inc';
check_reqs('PHP >= 7.1, < 7.4');
?>
--INI--
xdebug.profiler_enable=1
--FILE--
<?php
class bankaccount
{
	function __call( $foo, $bar )
	{
		var_dump( $foo, $bar );
	}
}

$b = new bankaccount;
$b->bar();

echo file_get_contents(xdebug_get_profiler_filename());
?>
--EXPECTF--
string(3) "bar"
array(0) {
}
version: 1
creator: xdebug %d.%s (PHP %s)
cmd: %sbug00728-php71.php
part: 1
positions: line

events: Time Memory

fl=(1) php:internal
fn=(1) php::{zend_pass}
10 %d %i

fl=(1)
fn=(2) php::var_dump
6 %d %i

fl=(2) %sbug00728-php71.php
fn=(3) bankaccount->__call
4 %d %i
cfl=(1)
cfn=(2)
calls=1 0 0
6 %d %i

fl=(2)
fn=(4) bankaccount->bar
4 %d %i
cfl=(2)
cfn=(3)
calls=1 0 0
11 %d %i

fl=(1)
fn=(5) php::xdebug_get_profiler_filename
13 %d %i
