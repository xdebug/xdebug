--TEST--
Test call_user_func_array() with multiple files
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
xdebug.var_display_max_depth=3
--FILE--
<?php
require_once 'capture-trace.inc';

include 'call_user_func_array-002.inc';
$c = "call_user_func_array";
$foo = array(1, 2);
$c('debug', array('foo', $foo));

$foo = 'bar';
$c('debug', array('bar', $foo));

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> include(%scall_user_func_array-002.inc) %scall_user_func_array-002.php:4
%w%f %w%d     -> call_user_func_array:{%scall_user_func_array-002.php:7}($%s = 'debug', $%s = [0 => 'foo', 1 => [0 => 1, 1 => 2]]) %scall_user_func_array-002.php:7
%w%f %w%d       -> debug($var = 'foo', $val = [0 => 1, 1 => 2]) %scall_user_func_array-002.php:7
%w%f %w%d         -> is_array($%s = [0 => 1, 1 => 2]) %scall_user_func_array-002.inc:4
%w%f %w%d     -> call_user_func_array:{%scall_user_func_array-002.php:10}($%s = 'debug', $%s = [0 => 'bar', 1 => 'bar']) %scall_user_func_array-002.php:10
%w%f %w%d       -> debug($var = 'bar', $val = 'bar') %scall_user_func_array-002.php:10
%w%f %w%d         -> is_array($%s = 'bar') %scall_user_func_array-002.inc:4
%w%f %w%d         -> is_object($%s = 'bar') %scall_user_func_array-002.inc:4
%w%f %w%d         -> is_resource($%s = 'bar') %scall_user_func_array-002.inc:4
%w%f %w%d     -> xdebug_stop_trace() %scall_user_func_array-002.php:12
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
