--TEST--
Test with internal callbacks
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

$ar = array('a', 'bb', 'ccc');
$r = array_map('strlen', $ar);

echo gettype($r), "\n";

echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
array
TRACE START [%d-%d-%d %d:%d:%d]
    %f          %d     -> array_map('strlen', array (0 => 'a', 1 => 'bb', 2 => 'ccc')) /%s/array_map.php:5
    %f          %d       -> strlen('a') /%s/array_map.php:5
    %f          %d       -> strlen('bb') /%s/array_map.php:5
    %f          %d       -> strlen('ccc') /%s/array_map.php:5
    %f          %d     -> gettype(array (0 => 1, 1 => 2, 2 => 3)) /%s/array_map.php:7
    %f          %d     -> file_get_contents('/tmp/%s') /%s/array_map.php:9
