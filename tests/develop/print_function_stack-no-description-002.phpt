--TEST--
Test for xdebug_print_function_stack() without description (CLI colour)
--INI--
xdebug.mode=develop
xdebug.cli_color=2
html_errors=0
--FILE--
<?php
function foo()
{
	xdebug_print_function_stack("test message", XDEBUG_STACK_NO_DESC);
}

function bar()
{
	foo(1, 3, 'foo', 'bar' );
}

bar();
?>
--EXPECTF--
[1mCall Stack:[22m
%w%f %w%d   1. {main}() %sprint_function_stack-no-description-002.php:0
%w%f %w%d   2. bar() %sprint_function_stack-no-description-002.php:12
%w%f %w%d   3. foo(1, 3, 'foo', 'bar') %sprint_function_stack-no-description-002.php:9
%w%f %w%d   4. xdebug_print_function_stack($message = 'test message', $options = 1) %sprint_function_stack-no-description-002.php:4
