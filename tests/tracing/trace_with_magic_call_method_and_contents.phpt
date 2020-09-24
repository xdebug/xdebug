--TEST--
Test for line numbers and arguments with __call including contents
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

class Test {
	public function __construct() { }
	public function __call($func, $args) {
		return $this->$func($args);
	}
	private function testFunc($args) {
		array_sum($args);
	}
}

$test = new Test();
$test->testFunc('test1', 'test2');

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> Test->__construct() %strace_with_magic_call_method_and_contents.php:14
%w%f %w%d     -> Test->testFunc('test1', 'test2') %strace_with_magic_call_method_and_contents.php:15
%w%f %w%d       -> Test->__call($func = 'testFunc', $args = [0 => 'test1', 1 => 'test2']) %strace_with_magic_call_method_and_contents.php:15
%w%f %w%d         -> Test->testFunc($args = [0 => 'test1', 1 => 'test2']) %strace_with_magic_call_method_and_contents.php:7
%w%f %w%d           -> array_sum($ar%s = [0 => 'test1', 1 => 'test2']) %strace_with_magic_call_method_and_contents.php:10
%w%f %w%d     -> xdebug_stop_trace() %strace_with_magic_call_method_and_contents.php:17
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
