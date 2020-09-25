--TEST--
Test for bug #1837: Support for associative variadic variable names (user-defined)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

function takeThemAll(string $one, ...$args)
{
}

takeThemAll(one: "test", arg1: 42, arg2: M_PI);
takeThemAll(arg1: 42, one: "test", arg2: M_PI);
takeThemAll("test", arg1: 42, arg2: M_PI);
takeThemAll("test", 42, M_PI);
takeThemAll("test", 42, arg2: M_PI);

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> takeThemAll($one = 'test', ...$args = variadic($arg1 => 42, $arg2 => 3.1415926535898)) %sbug01837-001.php:8
%w%f %w%d     -> takeThemAll($one = 'test', ...$args = variadic($arg1 => 42, $arg2 => 3.1415926535898)) %sbug01837-001.php:9
%w%f %w%d     -> takeThemAll($one = 'test', ...$args = variadic($arg1 => 42, $arg2 => 3.1415926535898)) %sbug01837-001.php:10
%w%f %w%d     -> takeThemAll($one = 'test', ...$args = variadic(0 => 42, 1 => 3.1415926535898)) %sbug01837-001.php:11
%w%f %w%d     -> takeThemAll($one = 'test', ...$args = variadic(0 => 42, $arg2 => 3.1415926535898)) %sbug01837-001.php:12
%w%f %w%d     -> xdebug_stop_trace() %sbug01837-001.php:14
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
