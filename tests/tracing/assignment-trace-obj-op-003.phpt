--TEST--
Test for tracing property assign ops [3]
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
require_once 'capture-trace.inc';

$a = ['foo' => new stdClass];
$a['foo']->test = 0;
$a['foo']->test += 42;
$a['foo']->test -= 2;
$a['foo']->test *= 2;
$a['foo']->test /= 5;
$a['foo']->test %= 4;
$a['foo']->test <<= 1;
$a['foo']->test >>= 3;
$a['foo']->test |= 0xffff;
$a['foo']->test &= 0xff0f;
$a['foo']->test ^= 0xf00f;
$a['foo']->test **= 2;

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
                           => $a = ['foo' => class stdClass {  }] %sassignment-trace-obj-op-003.php:4
                           => $a['foo']->test = 0 %sassignment-trace-obj-op-003.php:5
                           => $a['foo']->test += 42 %sassignment-trace-obj-op-003.php:6
                           => $a['foo']->test -= 2 %sassignment-trace-obj-op-003.php:7
                           => $a['foo']->test *= 2 %sassignment-trace-obj-op-003.php:8
                           => $a['foo']->test /= 5 %sassignment-trace-obj-op-003.php:9
                           => $a['foo']->test %= 4 %sassignment-trace-obj-op-003.php:10
                           => $a['foo']->test <<= 1 %sassignment-trace-obj-op-003.php:11
                           => $a['foo']->test >>= 3 %sassignment-trace-obj-op-003.php:12
                           => $a['foo']->test |= 65535 %sassignment-trace-obj-op-003.php:13
                           => $a['foo']->test &= 65295 %sassignment-trace-obj-op-003.php:14
                           => $a['foo']->test ^= 61455 %sassignment-trace-obj-op-003.php:15
                           => $a['foo']->test **= 2 %sassignment-trace-obj-op-003.php:16
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-obj-op-003.php:18
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
