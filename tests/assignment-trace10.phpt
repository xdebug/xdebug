--TEST--
Test for tracing assign pow (>= PHP 5.6)
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.6", '>=')) echo "skip >= PHP 5.6 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.collect_vars=1
xdebug.collect_params=4
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

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
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
                           => $tf = '%sxt' %sassignment-trace10.php:2
%w%f %w%d     -> test($a = 7, $b = 3) %sassignment-trace10.php:22
                             => $a **= 3 %sassignment-trace10.php:6
%w%f %w%d     -> testClass->__construct() %sassignment-trace10.php:23
                             => $this->a = 98 %sassignment-trace10.php:16
                             => $this->b = 4 %sassignment-trace10.php:17
                             => $this->a **= 4 %sassignment-trace10.php:18
                           => $a = class testClass { public $a = 92236816; private $b = 4 } %sassignment-trace10.php:23
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace10.php:25
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
