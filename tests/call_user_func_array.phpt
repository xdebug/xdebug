--TEST--
Test for file/line correctness with call_user_func_array()
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
--FILE--
<?php
xdebug_start_trace();

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

xdebug_dump_function_trace();
?>
--EXPECTF--
Function trace:
    %f      %d     -> call_user_func_array('debug', array (0 => 'foo', 1 => array (0 => 1, 1 => 2))) /%s/call_user_func_array.php:13
    %f      %d       -> debug('foo', array (0 => 1, 1 => 2)) /%s/call_user_func_array.php:4
    %f      %d         -> is_array(array (0 => 1, 1 => 2)) /%s/call_user_func_array.php:5
    %f      %d     -> call_user_func_array('debug', array (0 => 'bar', 1 => 'bar')) /%s/call_user_func_array.php:16
    %f      %d       -> debug('bar', 'bar') /%s/call_user_func_array.php:4
    %f      %d         -> is_array('bar') /%s/call_user_func_array.php:5
    %f      %d         -> is_object('bar') /%s/call_user_func_array.php:5
    %f      %d         -> is_resource('bar') /%s/call_user_func_array.php:5
