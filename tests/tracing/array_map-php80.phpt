--TEST--
Test with internal callbacks (PHP >= 8.0)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
xdebug.var_display_max_depth=2
xdebug.var_display_max_children=3
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/' . uniqid('xdt', TRUE));

$ar = array('a', 'bb', 'ccc');
$r = array_map('strlen', $ar);



echo file_get_contents($tf);
xdebug_stop_trace();
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> array_map($callback = 'strlen', $array = [0 => 'a', 1 => 'bb', 2 => 'ccc']) %sarray_map-php80.php:5
%w%f %w%d       -> strlen($string = 'a') %sarray_map-php80.php:5
%w%f %w%d       -> strlen($string = 'bb') %sarray_map-php80.php:5
%w%f %w%d       -> strlen($string = 'ccc') %sarray_map-php80.php:5
%w%f %w%d     -> file_get_contents($filename = '%s') %sarray_map-php80.php:9
