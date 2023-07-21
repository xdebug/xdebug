--TEST--
Test for bug #1837: Support for associative variadic variable names in stack traces
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0');
?>
--INI--
html_errors=0
xdebug.mode=develop
--FILE--
<?php
function takeThemAll(string $one, ...$args)
{
	xdebug_print_function_stack();
}

takeThemAll(one: "test", arg1: 42, arg2: M_PI);
takeThemAll(arg1: 42, one: "test", arg2: M_PI);
takeThemAll("test", arg1: 42, arg2: M_PI);
takeThemAll("test", 42, M_PI);
takeThemAll("test", 42, arg2: M_PI);
?>
--EXPECTF--
Xdebug: user triggered in %sbug01837-002.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sbug01837-002.php:0
%w%f %w%d   2. takeThemAll($one = 'test', ...$args = variadic($arg1 = 42, $arg2 = 3.1415926535898)) %sbug01837-002.php:7
%w%f %w%d   3. xdebug_print_function_stack() %sbug01837-002.php:4


Xdebug: user triggered in %sbug01837-002.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sbug01837-002.php:0
%w%f %w%d   2. takeThemAll($one = 'test', ...$args = variadic($arg1 = 42, $arg2 = 3.1415926535898)) %sbug01837-002.php:8
%w%f %w%d   3. xdebug_print_function_stack() %sbug01837-002.php:4


Xdebug: user triggered in %sbug01837-002.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sbug01837-002.php:0
%w%f %w%d   2. takeThemAll($one = 'test', ...$args = variadic($arg1 = 42, $arg2 = 3.1415926535898)) %sbug01837-002.php:9
%w%f %w%d   3. xdebug_print_function_stack() %sbug01837-002.php:4


Xdebug: user triggered in %sbug01837-002.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sbug01837-002.php:0
%w%f %w%d   2. takeThemAll($one = 'test', ...$args = variadic(42, 3.1415926535898)) %sbug01837-002.php:10
%w%f %w%d   3. xdebug_print_function_stack() %sbug01837-002.php:4


Xdebug: user triggered in %sbug01837-002.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sbug01837-002.php:0
%w%f %w%d   2. takeThemAll($one = 'test', ...$args = variadic(42, $arg2 = 3.1415926535898)) %sbug01837-002.php:11
%w%f %w%d   3. xdebug_print_function_stack() %sbug01837-002.php:4
