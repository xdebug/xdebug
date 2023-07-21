--TEST--
Test for bug #1837: Support for associative variadic variable names (internal)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
require_once 'capture-trace.inc';

function test(...$all) {
}

call_user_func("test", one: "test", arg1: 42, arg2: M_PI);
call_user_func("test", arg1: 42, one: "test", arg2: M_PI);
call_user_func("test", "test", arg1: 42, arg2: M_PI);
call_user_func("test", "test", 42, M_PI);
call_user_func("test", "test", 42, arg2: M_PI);

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> call_user_func:{%sbug01837-004.php:7}($callback = 'test', ...$args = variadic($one => 'test', $arg1 => 42, $arg2 => 3.1415926535898)) %sbug01837-004.php:7
%w%f %w%d       -> test(...$all = variadic($one => 'test', $arg1 => 42, $arg2 => 3.1415926535898)) %sbug01837-004.php:7
%w%f %w%d     -> call_user_func:{%sbug01837-004.php:8}($callback = 'test', ...$args = variadic($arg1 => 42, $one => 'test', $arg2 => 3.1415926535898)) %sbug01837-004.php:8
%w%f %w%d       -> test(...$all = variadic($arg1 => 42, $one => 'test', $arg2 => 3.1415926535898)) %sbug01837-004.php:8
%w%f %w%d     -> call_user_func:{%sbug01837-004.php:9}($callback = 'test', ...$args = variadic(0 => 'test', $arg1 => 42, $arg2 => 3.1415926535898)) %sbug01837-004.php:9
%w%f %w%d       -> test(...$all = variadic(0 => 'test', $arg1 => 42, $arg2 => 3.1415926535898)) %sbug01837-004.php:9
%w%f %w%d     -> test(...$all = variadic(0 => 'test', 1 => 42, 2 => 3.1415926535898)) %sbug01837-004.php:10
%w%f %w%d     -> call_user_func:{%sbug01837-004.php:11}($callback = 'test', ...$args = variadic(0 => 'test', 1 => 42, $arg2 => 3.1415926535898)) %sbug01837-004.php:11
%w%f %w%d       -> test(...$all = variadic(0 => 'test', 1 => 42, $arg2 => 3.1415926535898)) %sbug01837-004.php:11
%w%f %w%d     -> xdebug_stop_trace() %sbug01837-004.php:13
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
