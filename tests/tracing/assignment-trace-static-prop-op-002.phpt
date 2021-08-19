--TEST--
Test for tracing $static-prop property assign ops [2]
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
require_once 'capture-trace.inc';
$a = new test;
class test { static $test; function assign() { self::$test = ['foo' => 0];
	self::$test['foo'] = 0;
	self::$test['foo'] += 42;
	self::$test['foo'] -= 2;
	self::$test['foo'] *= 2;
	self::$test['foo'] /= 5;
	self::$test['foo'] %= 4;
	self::$test['foo'] <<= 1;
	self::$test['foo'] >>= 3;
	self::$test['foo'] |= 0xffff;
	self::$test['foo'] &= 0xff0f;
	self::$test['foo'] ^= 0xf00f;
	self::$test['foo'] **= 2;
} }

$a->assign();

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
                           => $a = class test {  } %sassignment-trace-static-prop-op-002.php:3
%w%f %w%d     -> test->assign() %sassignment-trace-static-prop-op-002.php:19
                             => self::test = ['foo' => 0] %sassignment-trace-static-prop-op-002.php:4
                             => self::test['foo'] = 0 %sassignment-trace-static-prop-op-002.php:5
                             => self::test['foo'] += 42 %sassignment-trace-static-prop-op-002.php:6
                             => self::test['foo'] -= 2 %sassignment-trace-static-prop-op-002.php:7
                             => self::test['foo'] *= 2 %sassignment-trace-static-prop-op-002.php:8
                             => self::test['foo'] /= 5 %sassignment-trace-static-prop-op-002.php:9
                             => self::test['foo'] %= 4 %sassignment-trace-static-prop-op-002.php:10
                             => self::test['foo'] <<= 1 %sassignment-trace-static-prop-op-002.php:11
                             => self::test['foo'] >>= 3 %sassignment-trace-static-prop-op-002.php:12
                             => self::test['foo'] |= 65535 %sassignment-trace-static-prop-op-002.php:13
                             => self::test['foo'] &= 65295 %sassignment-trace-static-prop-op-002.php:14
                             => self::test['foo'] ^= 61455 %sassignment-trace-static-prop-op-002.php:15
                             => self::test['foo'] **= 2 %sassignment-trace-static-prop-op-002.php:16
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-static-prop-op-002.php:21
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
