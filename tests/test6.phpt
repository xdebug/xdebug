--TEST--
Test for complex parameters to a function
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.default_enable=1
xdebug.collect_params=1
xdebug.dump_globals=0
xdebug.show_local_vars=1
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
xdebug.var_display_max_depth=3
xdebug.var_display_max_children=3
--FILE--
<?php
	function foo2 ($a, $b, $c)
	{
		return foo();
	}

	foo2 (4, array(array('blaat', 5, FALSE)));
?>
--EXPECTF--
Warning: Missing argument 3 for foo2()%sin /%s/test6.php on line 2

Call Stack:
%w%f %w%d   1. {main}() /%s/test6.php:0
%w%f %w%d   2. foo2(long, array(1), ???) /%s/test6.php:7


Variables in local scope (#2):
  $a = 4
  $c = *uninitialized*
  $b = array (0 => array (0 => 'blaat', 1 => 5, 2 => FALSE))


Fatal error: Call to undefined function%sfoo() in /%s/test6.php on line 4

Call Stack:
%w%f %w%d   1. {main}() /%s/test6.php:0
%w%f %w%d   2. foo2(long, array(1), ???) /%s/test6.php:7


Variables in local scope (#2):
  $a = 4
  $c = *uninitialized*
  $b = array (0 => array (0 => 'blaat', 1 => 5, 2 => FALSE))
