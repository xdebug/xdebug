--TEST--
Test for bug #1073: Segmentation Fault 11 when nesting call_user_func_array (< PHP 5.5)
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.5", '<')) echo "skip < PHP 5.5 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.collect_vars=0
xdebug.collect_params=4
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

call_user_func_array('call_user_func_array', array('printf', array("%u %u %u\n", 1,2,3)));

call_user_func_array('call_user_func_array', array('call_user_func_array', array('printf', array("%u %u %u\n", 1,2,3))));

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
1 2 3
1 2 3
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> call_user_func_array:{%sbug01073.php:4}('call_user_func_array', array (0 => 'printf', 1 => array (0 => '%u %u %u\n', 1 => 1, 2 => 2, 3 => 3))) %sbug01073.php:4
%w%f %w%d     -> call_user_func_array:{%sbug01073.php:6}('call_user_func_array', array (0 => 'call_user_func_array', 1 => array (0 => 'printf', 1 => array (...)))) %sbug01073.php:6
%w%f %w%d     -> xdebug_stop_trace() %sbug01073.php:8
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
