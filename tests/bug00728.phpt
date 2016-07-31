--TEST--
Test for bug #728: Profiler reports __call() invocations confusingly/wrongly. (< PHP 7.1)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.1", '<')) echo "skip < PHP 7.1 needed\n"; ?>
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
cmd: %sbug00728.php
part: 1
positions: line

events: Time

fl=(1) php:internal
fn=(1) php::var_dump
6 %d

fl=(2) %sbug00728.php
fn=(2) bankaccount->__call
4 %d
cfl=(1)
cfn=(1)
calls=1 0 0
6 %d

fl=(2)
fn=(3) bankaccount->bar
%r(11|4)%r %d
cfl=(2)
cfn=(2)
calls=1 0 0
11 %d

fl=(1)
fn=(4) php::xdebug_get_profiler_filename
13 %d
