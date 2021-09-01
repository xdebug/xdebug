--TEST--
Test for bug #699: Xdebug gets the filename wrong for the countable interface
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
require_once 'capture-trace.inc';

require dirname( __FILE__ ) . '/bug00699.inc';
$n = new SomeClass;
$n->addData('foobar');



count($n);



xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> dirname($path = '%s') %sbug00699.php:4
%w%f %w%d     -> require(%sbug00699.inc) %sbug00699.php:4
%w%f %w%d     -> SomeClass->addData($var = 'foobar') %sbug00699.php:6
%w%f %w%d     -> SomeClass->count(%S) %sbug00699.php:10
%w%f %w%d     -> xdebug_stop_trace() %sbug00699.php:14
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
