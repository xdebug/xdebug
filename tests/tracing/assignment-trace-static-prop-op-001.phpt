--TEST--
Test for tracing $static-prop property assign ops [1]
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));
$a = new test;
class test { static $test; function assign() {
	self::$test = 0;
	self::$test += 42;
	self::$test -= 2;
	self::$test *= 2;
	self::$test /= 5;
	self::$test %= 4;
	self::$test <<= 1;
	self::$test >>= 3;
	self::$test |= 0xffff;
	self::$test &= 0xff0f;
	self::$test ^= 0xf00f;
	self::$test **= 2;
} }

$a->assign();

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                           => $tf = '%s.xt' %sassignment-trace-static-prop-op-001.php:2
                           => $a = class test {  } %sassignment-trace-static-prop-op-001.php:3
%w%f %w%d     -> test->assign() %sassignment-trace-static-prop-op-001.php:19
                             => self::test = 0 %sassignment-trace-static-prop-op-001.php:5
                             => self::test += 42 %sassignment-trace-static-prop-op-001.php:6
                             => self::test -= 2 %sassignment-trace-static-prop-op-001.php:7
                             => self::test *= 2 %sassignment-trace-static-prop-op-001.php:8
                             => self::test /= 5 %sassignment-trace-static-prop-op-001.php:9
                             => self::test %= 4 %sassignment-trace-static-prop-op-001.php:10
                             => self::test <<= 1 %sassignment-trace-static-prop-op-001.php:11
                             => self::test >>= 3 %sassignment-trace-static-prop-op-001.php:12
                             => self::test |= 65535 %sassignment-trace-static-prop-op-001.php:13
                             => self::test &= 65295 %sassignment-trace-static-prop-op-001.php:14
                             => self::test ^= 61455 %sassignment-trace-static-prop-op-001.php:15
                             => self::test **= 2 %sassignment-trace-static-prop-op-001.php:16
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-static-prop-op-001.php:21
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
