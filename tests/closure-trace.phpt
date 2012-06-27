--TEST--
Test for closure file and location in stack traces
--SKIPIF--
<?php if(version_compare(phpversion(), "5.3.0", '<')) echo "skip PHP 5.3 needed\n"; ?>
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
$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));

function test1()
{
	$f = function($a, $b) {
		return strlen($a * $b);
	};

	$f(5, 25);
}

test1();
xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
                           => $tf = '/tmp/%s.xt' %sclosure-trace.php:2
%w%f %w%d     -> test1() %sclosure-trace.php:13
                             => $f = class Closure {  } %sclosure-trace.php:8
%w%f %w%d       -> {closure:%sclosure-trace.php:6-8}($a = 5, $b = 25) %sclosure-trace.php:10
%w%f %w%d         -> strlen(125) %sclosure-trace.php:7
                               >=> 3
                             >=> 3
                           >=> NULL
%w%f %w%d     -> xdebug_stop_trace() %sclosure-trace.php:14
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
