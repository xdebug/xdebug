--TEST--
Test for file/line correctness with call_user_func_array()
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));

function debug($var, $val) {
    if (is_array($val) || is_object($val) || is_resource($val)) {
        /* Do nothing */
    } else {
        /* Do nothing */
	}
}

$foo = array(1, 2);
call_user_func_array ('debug', array('foo', $foo));

$foo = 'bar';
call_user_func_array ('debug', array('bar', $foo));

echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> call_user_func_array('debug', array (0 => 'foo', 1 => array (0 => 1, 1 => 2))) /%s/call_user_func_array.php:13
%w%f %w%d       -> debug('foo', array (0 => 1, 1 => 2)) /%s/call_user_func_array.php:0
%w%f %w%d         -> is_array(array (0 => 1, 1 => 2)) /%s/call_user_func_array.php:5
%w%f %w%d     -> call_user_func_array('debug', array (0 => 'bar', 1 => 'bar')) /%s/call_user_func_array.php:16
%w%f %w%d       -> debug('bar', 'bar') /%s/call_user_func_array.php:0
%w%f %w%d         -> is_array('bar') /%s/call_user_func_array.php:5
%w%f %w%d         -> is_object('bar') /%s/call_user_func_array.php:5
%w%f %w%d         -> is_resource('bar') /%s/call_user_func_array.php:5
%w%f %w%d     -> file_get_contents('/tmp/%s') /%s/call_user_func_array.php:18
