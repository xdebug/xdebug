--TEST--
Test for bug #728: Profiler reports __call() invocations confusingly/wrongly.
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
creator: xdebug %s
cmd: %sbug00728.php
part: 1
positions: line

events: Time

fl=php:internal
fn=php::var_dump
6 %d

fl=%sbug00728.php
fn=bankaccount->__call
4 %d
cfl=php:internal
cfn=php::var_dump
calls=1 0 0
6 %d

fl=%sbug00728.php
fn=bankaccount->bar
11 %d
cfl=%sbug00728.php
cfn=bankaccount->__call
calls=1 0 0
11 %d

fl=php:internal
fn=php::xdebug_get_profiler_filename
13 %d
