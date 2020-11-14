--TEST--
Test for bug #1875: Overflow with large amounts of elements for variadics
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

$a = [];

$input1 = array_fill(0, 65534, 65534);
$input2 = array_fill(0, 65535, 65535);

array_push($a, ...$input1);
array_push($a, ...$input2);

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> array_fill($%s = 0, $%s = 65534, $%s = 65534) %sbug01875-002.php:6
%w%f %w%d     -> array_fill($%s = 0, $%s = 65535, $%s = 65535) %sbug01875-002.php:7
%w%f %w%d     -> array_push($%s = [], ...$%s = variadic(%s)) %sbug01875-002.php:9
%w%f %w%d     -> array_push() %sbug01875-002.php:10
%w%f %w%d     -> xdebug_stop_trace() %sbug01875-002.php:12
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
