--TEST--
Test for closure file and location in stack traces
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.collect_vars=1
xdebug.collect_params=4
xdebug.collect_return=1
xdebug.collect_assignments=1
--FILE--
<?php
function test1()
{
	$f = function($a, $b) {
		$Q = strlen($a * $b);
		trigger_error('foo');
	};

	$f(5, 25);
}

test1();
?>
--EXPECTF--
Notice: foo in %sclosure-stack-trace.php on line 6

Call Stack:
%w%f %w%d   1. {main}() %sclosure-stack-trace.php:0
%w%f %w%d   2. test1() %sclosure-stack-trace.php:12
%w%f %w%d   3. {closure:%sclosure-stack-trace.php:4-7}($a = 5, $b = 25) /home/derick/dev/php/xdebug.git/trunk/tests/closure-stack-trace.php:9
%w%f %w%d   4. trigger_error('foo') %sclosure-stack-trace.php:6
