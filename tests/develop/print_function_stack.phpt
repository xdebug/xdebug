--TEST--
Test for xdebug_print_function_stack()
--INI--
xdebug.mode=develop
xdebug.cli_color=0
--FILE--
<?php
function foo()
{
	xdebug_print_function_stack();
}

function bar()
{
	foo(1, 3, 'foo', 'bar' );
}

bar();
?>
--EXPECTF--
Xdebug: user triggered in %sprint_function_stack.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sprint_function_stack.php:0
%w%f %w%d   2. bar() %sprint_function_stack.php:12
%w%f %w%d   3. foo(1, 3, 'foo', 'bar') %sprint_function_stack.php:9
%w%f %w%d   4. xdebug_print_function_stack() %sprint_function_stack.php:4
