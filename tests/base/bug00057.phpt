--TEST--
Test for bug #57: Crash with overloading functions
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_assignments=0
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
error_reporting=2047
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

$o = new OO;

echo $o->foo('80');

?> 
--EXPECTF--
2
