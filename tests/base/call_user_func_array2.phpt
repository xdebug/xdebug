--TEST--
Test call_user_func_array() with multiple files
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

include 'call_user_func_array2.inc';
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
%w%f %w%d     -> include(%scall_user_func_array2.inc) %scall_user_func_array2.php:4
%w%f %w%d     -> call_user_func_array:{%scall_user_func_array2.php:7}('debug', array (0 => 'foo', 1 => array (0 => 1, 1 => 2))) %scall_user_func_array2.php:7
%w%f %w%d       -> debug('foo', array (0 => 1, 1 => 2)) %scall_user_func_array2.php:7
%w%f %w%d         -> is_array(array (0 => 1, 1 => 2)) %scall_user_func_array2.inc:4
%w%f %w%d     -> call_user_func_array:{%scall_user_func_array2.php:10}('debug', array (0 => 'bar', 1 => 'bar')) %scall_user_func_array2.php:10
%w%f %w%d       -> debug('bar', 'bar') %scall_user_func_array2.php:10
%w%f %w%d         -> is_array('bar') %scall_user_func_array2.inc:4
%w%f %w%d         -> is_object('bar') %scall_user_func_array2.inc:4
%w%f %w%d         -> is_resource('bar') %scall_user_func_array2.inc:4
%w%f %w%d     -> xdebug_stop_trace() %scall_user_func_array2.php:12
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
