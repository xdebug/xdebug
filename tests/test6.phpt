--TEST--
Test for complex parameters to a function
--INI--
xdebug.default_enable=1
xdebug.collect_params=1
xdebug.dump_globals=0
xdebug.show_local_vars=1
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
--FILE--
<?php
	function foo2 ($a, $b, $c)
	{
		return foo();
	}

	foo2 (4, array(array('blaat', 5, FALSE)));
?>
--EXPECTF--
Warning: Missing argument 3 for foo2()%s in /%s/test6.php on line 2

Call Stack:
    %f      %d   1. {main}() /%s/test6.php:0
    %f      %d   2. foo2(4, array (0 => array (0 => 'blaat', 1 => 5, 2 => FALSE))) /%s/test6.php:7


Variables in local scope:
  $a = 4
  $c = NULL
  $b = array (0 => array (0 => 'blaat', 1 => 5, 2 => FALSE))

Fatal error: Call to undefined function%sfoo() in /%s/test6.php on line 4

Call Stack:
    %f      %d   1. {main}() /%s/test6.php:0
    %f      %d   2. foo2(4, array (0 => array (0 => 'blaat', 1 => 5, 2 => FALSE))) /%s/test6.php:7


Variables in local scope:
  $a = 4
  $c = NULL
  $b = array (0 => array (0 => 'blaat', 1 => 5, 2 => FALSE))
