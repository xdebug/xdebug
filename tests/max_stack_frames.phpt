--TEST--
Test for xdebug.max_stack_frames
--INI--
xdebug.max_nesting_level=100
xdebug.collect_params=3
xdebug.default_enable=1
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
Fatal error: Maximum function nesting level of '100' reached, aborting! in %smax_stack_frames.php on line %d

Call Stack:
%w%f %w%d   1. {main}() %smax_stack_frames.php:0
%w%f %w%d   2. foo(0) %smax_stack_frames.php:8
%w%f %w%d   3. foo(1) %smax_stack_frames.php:6
%w%f %w%d   4. foo(2) %smax_stack_frames.php:6
%w%f %w%d   5. foo(3) %smax_stack_frames.php:6
