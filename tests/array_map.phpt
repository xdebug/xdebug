--TEST--
Test with internal callbacks
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.5", '<')) echo "skip < PHP 5.5 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.var_display_max_depth=2
xdebug.var_display_max_children=3
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

$ar = array('a', 'bb', 'ccc');
$r = array_map('strlen', $ar);

echo gettype($r), "\n";

echo file_get_contents($tf);
xdebug_stop_trace();
unlink($tf);
?>
--EXPECTF--
array
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> array_map('strlen', array (0 => 'a', 1 => 'bb', 2 => 'ccc')) %sarray_map.php:5
%w%f %w%d     -> gettype(array (0 => 1, 1 => 2, 2 => 3)) %sarray_map.php:7
%w%f %w%d     -> file_get_contents('/tmp/%s') %sarray_map.php:9
