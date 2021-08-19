--TEST--
Test for tracing property assign ops [2]
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
require_once 'capture-trace.inc';

$a = new stdClass;
$a->test = ['foo' => 0];
$a->test['foo'] += 42;
$a->test['foo'] -= 2;
$a->test['foo'] *= 2;
$a->test['foo'] /= 5;
$a->test['foo'] %= 4;
$a->test['foo'] <<= 1;
$a->test['foo'] >>= 3;
$a->test['foo'] |= 0xffff;
$a->test['foo'] &= 0xff0f;
$a->test['foo'] ^= 0xf00f;
$a->test['foo'] **= 2;

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
                           => $a = class stdClass {  } %sassignment-trace-obj-op-002.php:4
                           => $a->test = ['foo' => 0] %sassignment-trace-obj-op-002.php:5
                           => $a->test['foo'] += 42 %sassignment-trace-obj-op-002.php:6
                           => $a->test['foo'] -= 2 %sassignment-trace-obj-op-002.php:7
                           => $a->test['foo'] *= 2 %sassignment-trace-obj-op-002.php:8
                           => $a->test['foo'] /= 5 %sassignment-trace-obj-op-002.php:9
                           => $a->test['foo'] %= 4 %sassignment-trace-obj-op-002.php:10
                           => $a->test['foo'] <<= 1 %sassignment-trace-obj-op-002.php:11
                           => $a->test['foo'] >>= 3 %sassignment-trace-obj-op-002.php:12
                           => $a->test['foo'] |= 65535 %sassignment-trace-obj-op-002.php:13
                           => $a->test['foo'] &= 65295 %sassignment-trace-obj-op-002.php:14
                           => $a->test['foo'] ^= 61455 %sassignment-trace-obj-op-002.php:15
                           => $a->test['foo'] **= 2 %sassignment-trace-obj-op-002.php:16
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-obj-op-002.php:18
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
