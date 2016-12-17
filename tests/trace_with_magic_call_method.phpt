--TEST--
Test for line numbers and arguments with __call
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.collect_vars=0
xdebug.collect_params=1
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
		count($args);
	}
}

$test = new Test();
$test->testFunc('test1', 'test2');

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> Test->__construct() %strace_with_magic_call_method.php:14
%w%f %w%d     -> Test->testFunc(string(5), string(5)) %strace_with_magic_call_method.php:15
%w%f %w%d       -> Test->__call(string(8), array(2)) %strace_with_magic_call_method.php:15
%w%f %w%d         -> Test->testFunc(array(2)) %strace_with_magic_call_method.php:7
%w%f %w%d           -> count(array(2)) %strace_with_magic_call_method.php:10
%w%f %w%d     -> xdebug_stop_trace() %strace_with_magic_call_method.php:17
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
