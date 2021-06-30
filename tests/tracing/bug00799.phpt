--TEST--
Test for bug #799: Function traces report base class instead of object name
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

abstract class A
{
	abstract function foo();
	
	public function bar()
	{
		echo "A Test","\n";
	}
}

class B extends A
{
	public function foo()
	{
	}
}

$test = new B;
$test->bar();

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
A Test
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> A->bar() %sbug00799.php:22
%w%f %w%d     -> xdebug_stop_trace() %sbug00799.php:24
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
