--TEST--
Test for bug #642: No line number for offsetGet and offsetSet
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

class Test implements ArrayAccess {
    private $container = array();
    
    public function offsetSet($offset, $value) {
        $this->container[$offset] = $value;
    }
    public function offsetExists($offset) {
        return isset($this->container[$offset]);
    }
    public function offsetUnset($offset) {
        unset($this->container[$offset]);
    }
    public function offsetGet($offset) {
        return isset($this->container[$offset]) ? $this->container[$offset] : null;
    }
}

$test = new Test();
$test['test'] = 'test';
$foo = $test['test'];
isset($test['test']);
unset($test['test']);

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> Test->offsetSet($offset = 'test', $value = 'test') %sbug00642.php:22
%w%f %w%d     -> Test->offsetGet($offset = 'test') %sbug00642.php:23
%w%f %w%d     -> Test->offsetExists($offset = 'test') %sbug00642.php:24
%w%f %w%d     -> Test->offsetUnset($offset = 'test') %sbug00642.php:25
%w%f %w%d     -> xdebug_stop_trace() %sbug00642.php:27
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
