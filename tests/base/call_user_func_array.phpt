--TEST--
Test for file/line correctness with call_user_func_array()
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.var_display_max_depth=3
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));
function debug($var, $val) {
    $ia = 'is_array'; $io = 'is_object'; $ir = 'is_resource';
    if ($ia($val) || $io($val) || $ir($val)) {
        /* Do nothing */
    } else {
        /* Do nothing */
	}
}
$c = "call_user_func_array";
$foo = array(1, 2);
$c('debug', array('foo', $foo));

$foo = 'bar';
$c('debug', array('bar', $foo));

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> call_user_func_array:{%scall_user_func_array.php:13}('debug', array (0 => 'foo', 1 => array (0 => 1, 1 => 2))) %scall_user_func_array.php:13
%w%f %w%d       -> debug('foo', array (0 => 1, 1 => 2)) %scall_user_func_array.php:13
%w%f %w%d         -> is_array(array (0 => 1, 1 => 2)) %scall_user_func_array.php:5
%w%f %w%d     -> call_user_func_array:{%scall_user_func_array.php:16}('debug', array (0 => 'bar', 1 => 'bar')) %scall_user_func_array.php:16
%w%f %w%d       -> debug('bar', 'bar') %scall_user_func_array.php:16
%w%f %w%d         -> is_array('bar') %scall_user_func_array.php:5
%w%f %w%d         -> is_object('bar') %scall_user_func_array.php:5
%w%f %w%d         -> is_resource('bar') %scall_user_func_array.php:5
%w%f %w%d     -> xdebug_stop_trace() %scall_user_func_array.php:18
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
