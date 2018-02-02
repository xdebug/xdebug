--TEST--
Test for complex parameters to a function (= PHP 7.0)
--SKIPIF--
<?php
if (!version_compare(phpversion(), "7.0", '>=')) echo "skip = PHP 7.0 needed\n";
if (!version_compare(phpversion(), "7.1", '<')) echo "skip = PHP 7.0 needed\n";
?>
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
	function foo2 ($a, $b, $c)
	{
		return foo();
	}

	foo2 (4, array(array('blaat', 5, FALSE)));
?>
--EXPECTF--
Warning: Missing argument 3 for foo2()%sin %sbacktrace-args-php70.php on line 2

Call Stack:
%w%f %w%d   1. {main}() %sbacktrace-args-php70.php:0
%w%f %w%d   2. foo2(long, array(1), ???) %sbacktrace-args-php70.php:7


Variables in local scope (#2):
  $a = 4
  $b = array (0 => array (0 => 'blaat', 1 => 5, 2 => FALSE))
  $c = *uninitialized*


Fatal error: Uncaught Error: Call to undefined function%sfoo() in %sbacktrace-args-php70.php on line 4

Error: Call to undefined function foo() in %sbacktrace-args-php70.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sbacktrace-args-php70.php:0
%w%f %w%d   2. foo2(long, array(1), ???) %sbacktrace-args-php70.php:7


Variables in local scope (#2):
  $a = 4
  $b = array (0 => array (0 => 'blaat', 1 => 5, 2 => FALSE))
  $c = *uninitialized*
