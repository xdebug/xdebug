--TEST--
Test for bug #699: Xdebug gets the filename wrong for the countable interface (>= PHP 7.2)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.2", '>=')) echo "skip >= PHP 7.2 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.collect_vars=0
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

require dirname( __FILE__ ) . '/bug00699.inc';
$n = new SomeClass;
$n->addData('foobar');



count($n);



xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> dirname(string(%d)) %sbug00699-php72.php:4
%w%f %w%d     -> require(%sbug00699.inc) %sbug00699-php72.php:4
%w%f %w%d     -> SomeClass->addData(string(6)) %sbug00699-php72.php:6
%w%f %w%d     -> SomeClass->count(%S) %sbug00699-php72.php:10
%w%f %w%d     -> xdebug_stop_trace() %sbug00699-php72.php:14
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
