--TEST--
Test for complex parameters to a function
--INI--
xdebug.mode=develop
xdebug.dump_globals=0
xdebug.show_local_vars=1
xdebug.var_display_max_depth=3
xdebug.var_display_max_children=3
xdebug.show_error_trace=0
--FILE--
<?php
	function foo2 ($a, $b)
	{
		return foo();
	}

	foo2 (4, array(array('blaat', 5, FALSE)));
?>
--EXPECTF--
Fatal error: Uncaught Error: Call to undefined function%sfoo() in %sbacktrace-args.php on line 4

Error: Call to undefined function foo() in %sbacktrace-args.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sbacktrace-args.php:0
%w%f %w%d   2. foo2($a = 4, $b = [0 => [0 => 'blaat', 1 => 5, 2 => FALSE]]) %sbacktrace-args.php:7


Variables in local scope (#2):
  $a = 4
  $b = [0 => [0 => 'blaat', 1 => 5, 2 => FALSE]]
