--TEST--
Test for bug #1073: Segmentation Fault 11 when nesting call_user_func_array
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.var_display_max_children=128
xdebug.var_display_max_data=512
xdebug.var_display_max_depth=3
--FILE--
<?php
require_once 'capture-trace.inc';
$c = 'call_user_func_array';
$c('call_user_func_array', array('printf', array("%u %u %u\n", 1,2,3)));

$c('call_user_func_array', array('call_user_func_array', array('printf', array("%u %u %u\n", 1,2,3))));

xdebug_stop_trace();
?>
--EXPECTF--
1 2 3
1 2 3
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> call_user_func_array:{%sbug01073.php:4}($%s = 'call_user_func_array', $%s = [0 => 'printf', 1 => [0 => '%u %u %u\n', 1 => 1, 2 => 2, 3 => 3]]) %sbug01073.php:4
%w%f %w%d       -> call_user_func_array:{%sbug01073.php:4}($%s = 'printf', $%s = [0 => '%u %u %u\n', 1 => 1, 2 => 2, 3 => 3]) %sbug01073.php:4
%w%f %w%d         -> printf($format = '%u %u %u\n', ...$%s = variadic(0 => 1, 1 => 2, 2 => 3)) %sbug01073.php:4
%w%f %w%d     -> call_user_func_array:{%sbug01073.php:6}($%s = 'call_user_func_array', $%s = [0 => 'call_user_func_array', 1 => [0 => 'printf', 1 => [...]]]) %sbug01073.php:6
%w%f %w%d       -> call_user_func_array:{%sbug01073.php:6}($%s = 'call_user_func_array', $%s = [0 => 'printf', 1 => [0 => '%u %u %u\n', 1 => 1, 2 => 2, 3 => 3]]) %sbug01073.php:6
%w%f %w%d         -> call_user_func_array:{%sbug01073.php:6}($%s = 'printf', $%s = [0 => '%u %u %u\n', 1 => 1, 2 => 2, 3 => 3]) %sbug01073.php:6
%w%f %w%d           -> printf($format = '%u %u %u\n', ...$%s = variadic(0 => 1, 1 => 2, 2 => 3)) %sbug01073.php:6
%w%f %w%d     -> xdebug_stop_trace() %sbug01073.php:8
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
