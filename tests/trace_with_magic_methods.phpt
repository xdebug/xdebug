--TEST--
Test for line numbers for __get, __set, __isset, __unset and __call.
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
$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));

class Test {
    private $container = array();
    public function __construct() { }
    public function __set($offset, $value) {
        $this->container[$offset] = $value;
    }
    public function __isset($offset) {
        return isset($this->container[$offset]);
    }
    public function __unset($offset) {
        unset($this->container[$offset]);
    }
    public function __get($offset) {
        return isset($this->container[$offset]) ? $this->container[$offset] : null;
    }
	public function __call($func, $args) {
		return $this->$func($args);
	}
	public function test($args) {
		count($args);
	}
}

$test = new Test();
$test->test = 'test';
$foo = $test->test;
isset($test->test);
unset($test->test);
$test->test('test1', 'test2');

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> Test->__construct() %strace_with_magic_methods.php:27
%w%f %w%d     -> Test->__set(string(4), string(4)) %strace_with_magic_methods.php:28
%w%f %w%d     -> Test->__get(string(4)) %strace_with_magic_methods.php:29
%w%f %w%d     -> Test->__isset(string(4)) %strace_with_magic_methods.php:30
%w%f %w%d     -> Test->__unset(string(4)) %strace_with_magic_methods.php:31
%w%f %w%d     -> Test->test(string(5), string(5)) %strace_with_magic_methods.php:32
%w%f %w%d       -> count(string(5)) %strace_with_magic_methods.php:23
%w%f %w%d     -> xdebug_stop_trace() %strace_with_magic_methods.php:34
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
