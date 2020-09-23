--TEST--
Test for xdebug.max_stack_frames
--INI--
xdebug.max_nesting_level=100
xdebug.mode=develop
xdebug.show_local_vars=0
--FILE--
<?php
ini_set('xdebug.max_stack_frames', 5);

function foo($a)
{
	foo($a+1);
}
foo(0);
?>
--EXPECTF--
Fatal error: Uncaught Error: Xdebug has detected a possible infinite loop, and aborted your script with a stack depth of '100' frames in %smax_stack_frames.php on line 4

Error: Xdebug has detected a possible infinite loop, and aborted your script with a stack depth of '100' frames in %smax_stack_frames.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %smax_stack_frames.php:0
%w%f %w%d   2. foo($a = 0) %smax_stack_frames.php:8
%w%f %w%d   3. foo($a = 1) %smax_stack_frames.php:6
%w%f %w%d   4. foo($a = 2) %smax_stack_frames.php:6
%w%f %w%d   5. foo($a = 3) %smax_stack_frames.php:6
