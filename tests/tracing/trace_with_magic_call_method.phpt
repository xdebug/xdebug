--TEST--
Test for line numbers and arguments with __call (>= PHP 8.1)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
require_once 'capture-trace.inc';

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
$test->testFunc(21, 62);

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> Test->__construct() %strace_with_magic_call_method.php:14
%w%f %w%d     -> Test->__call($func = 'testFunc', $args = [0 => 21, 1 => 62]) %strace_with_magic_call_method.php:15
%w%f %w%d       -> Test->testFunc($args = [0 => 21, 1 => 62]) %strace_with_magic_call_method.php:7
%w%f %w%d         -> array_sum($ar%s = [0 => 21, 1 => 62]) %strace_with_magic_call_method.php:10
%w%f %w%d     -> xdebug_stop_trace() %strace_with_magic_call_method.php:17
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
