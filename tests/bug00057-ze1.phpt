--TEST--
Test for bug #57: Crash with overloading functions (ZE1)
--SKIPIF--
<?php if(version_compare(zend_version(), "2.0.0-dev", '>')) echo "skip Zend Engine 1 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
xdebug.show_local_vars=0
--FILE--
<?php
class OO {
	var $a = 111;
	var $elem = array('b' => 9, 'c' => 42);

	// Callback method for executing a method
	function __call($function, $params)
	{
		echo strlen($params[0]);
	}
}

// Here we overload the OO object
overload('OO');

$o = new OO;

echo $o->foo('80');

?> 
--EXPECTF--
Notice: Undefined offset:  0 in /%s/bug00057-ze1.php on line 9

Call Stack:
    %f      %d   1. {main}() /%s/bug00057-ze1.php:0
    %f      %d   2. oo->__call('oo', array (), NULL) /%s/bug00057-ze1.php:16
0
Warning: Call to undefined method oo::oo() in /%s/bug00057-ze1.php on line 16

Call Stack:
    %f      %d   1. {main}() /%s/bug00057-ze1.php:0
2
Warning: Call to undefined method oo::foo() in /%s/bug00057-ze1.php on line 18

Call Stack:
    %f      %d   1. {main}() /%s/bug00057-ze1.php:0
