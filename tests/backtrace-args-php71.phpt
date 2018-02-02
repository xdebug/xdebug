--TEST--
Test for complex parameters to a function (>= PHP 7.1)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.1", '>=')) echo "skip >= PHP 7.1 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.collect_params=1
xdebug.dump_globals=0
xdebug.show_local_vars=1
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
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
Fatal error: Uncaught Error: Call to undefined function%sfoo() in %sbacktrace-args-php71.php on line 4

Error: Call to undefined function foo() in %sbacktrace-args-php71.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sbacktrace-args-php71.php:0
%w%f %w%d   2. foo2(long, array(1)) %sbacktrace-args-php71.php:7


Variables in local scope (#2):
  $a = 4
  $b = array (0 => array (0 => 'blaat', 1 => 5, 2 => FALSE))
