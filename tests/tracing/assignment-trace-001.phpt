--TEST--
Test for tracing property assignments in user-readable function traces (!opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
?>
--INI--
xdebug.mode=trace
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

function test($a, $b, $c)
{
	$d = 89;
	$a += $b;
	$c /= 7;
	$b *= 9;
}

class testClass
{
	public $a;
	private $b;
	protected $c;

	function __construct()
	{
		$this->a = 98;
		$this->b = 4;
		$this->b -= 8;
		$this->b *= -0.5;
		$this->b <<= 1;
		$this->c = $this->b / 32;
	}
}

test(1, 2, 3);
$a = new testClass;

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                           => $tf = '%sxt' %sassignment-trace-001.php:2
%w%f %w%d     -> test($a = 1, $b = 2, $c = 3) %sassignment-trace-001.php:29
                             => $d = 89 %sassignment-trace-001.php:6
                             => $a += 2 %sassignment-trace-001.php:7
                             => $c /= 7 %sassignment-trace-001.php:8
                             => $b *= 9 %sassignment-trace-001.php:9
%w%f %w%d     -> testClass->__construct() %sassignment-trace-001.php:30
                             => $this->a = 98 %sassignment-trace-001.php:20
                             => $this->b = 4 %sassignment-trace-001.php:21
                             => $this->b -= 8 %sassignment-trace-001.php:22
                             => $this->b *= -0.5 %sassignment-trace-001.php:23
                             => $this->b <<= 1 %sassignment-trace-001.php:24
                             => $this->c = 0.125 %sassignment-trace-001.php:25
                           => $a = class testClass { public $a = 98; private $b = 4; protected $c = 0.125 } %sassignment-trace-001.php:30
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-001.php:32
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
