--TEST--
Test for tracing assign pow
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
require_once 'capture-trace.inc';

function test($a, $b)
{
	$a **= $b;
}

class testClass
{
	public $a;
	private $b;

	function __construct()
	{
		$this->a = 98;
		$this->b = 4;
		$this->a **= $this->b;
	}
}

test(7, 3);
$a = new testClass;

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
%w%f %w%d     -> test($a = 7, $b = 3) %sassignment-trace-010.php:22
                             => $a **= 3 %sassignment-trace-010.php:6
%w%f %w%d     -> testClass->__construct() %sassignment-trace-010.php:23
                             => $this->a = 98 %sassignment-trace-010.php:16
                             => $this->b = 4 %sassignment-trace-010.php:17
                             => $this->a **= 4 %sassignment-trace-010.php:18
                           => $a = class testClass { public $a = 92236816; private $b = 4 } %sassignment-trace-010.php:23
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-010.php:25
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
