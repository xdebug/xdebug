--TEST--
Test call_user_func_array() with multiple files
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));

include 'call_user_func_array2.inc';

$foo = array(1, 2);
call_user_func_array ('debug', array('foo', $foo));

$foo = 'bar';
call_user_func_array ('debug', array('bar', $foo));

echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d     -> include(/%s/call_user_func_array2.inc) /%s/call_user_func_array2.php:4
    %f      %d     -> call_user_func_array('debug', array (0 => 'foo', 1 => array (0 => 1, 1 => 2))) /%s/call_user_func_array2.php:7
    %f      %d       -> debug('foo', array (0 => 1, 1 => 2)) /%s/call_user_func_array2.php:7
    %f      %d         -> is_array(array (0 => 1, 1 => 2)) /%s/call_user_func_array2.inc:4
    %f      %d     -> call_user_func_array('debug', array (0 => 'bar', 1 => 'bar')) /%s/call_user_func_array2.php:10
    %f      %d       -> debug('bar', 'bar') /%s/call_user_func_array2.php:10
    %f      %d         -> is_array('bar') /%s/call_user_func_array2.inc:4
    %f      %d         -> is_object('bar') /%s/call_user_func_array2.inc:4
    %f      %d         -> is_resource('bar') /%s/call_user_func_array2.inc:4
    %f      %d     -> file_get_contents('/tmp/%s') /%s/call_user_func_array2.php:12
