--TEST--
Test for line numbers for __get, __set, __isset, and __unset
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
}

$test = new Test();
$test->test = 'test';
$foo = $test->test;
isset($test->test);
unset($test->test);

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> Test->__construct() %strace_with_magic_methods.php:21
%w%f %w%d     -> Test->__set($offset = 'test', $value = 'test') %strace_with_magic_methods.php:22
%w%f %w%d     -> Test->__get($offset = 'test') %strace_with_magic_methods.php:23
%w%f %w%d     -> Test->__isset($offset = 'test') %strace_with_magic_methods.php:24
%w%f %w%d     -> Test->__unset($offset = 'test') %strace_with_magic_methods.php:25
%w%f %w%d     -> xdebug_stop_trace() %strace_with_magic_methods.php:27
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
