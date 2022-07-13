--TEST--
Test for tracing $this property assign ops [1]
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
class test { public $test; function assign() {
	$this->test = 0;
	$this->test += 42;
	$this->test -= 2;
	$this->test *= 2;
	$this->test /= 5;
	$this->test %= 4;
	$this->test <<= 1;
	$this->test >>= 3;
	$this->test |= 0xffff;
	$this->test &= 0xff0f;
	$this->test ^= 0xf00f;
	$this->test **= 2;
} }

$a->assign();

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
                           => $a = class test { public $test = NULL } %sassignment-trace-this-op-001.php:3
%w%f %w%d     -> test->assign() %sassignment-trace-this-op-001.php:19
                             => $this->test = 0 %sassignment-trace-this-op-001.php:5
                             => $this->test += 42 %sassignment-trace-this-op-001.php:6
                             => $this->test -= 2 %sassignment-trace-this-op-001.php:7
                             => $this->test *= 2 %sassignment-trace-this-op-001.php:8
                             => $this->test /= 5 %sassignment-trace-this-op-001.php:9
                             => $this->test %= 4 %sassignment-trace-this-op-001.php:10
                             => $this->test <<= 1 %sassignment-trace-this-op-001.php:11
                             => $this->test >>= 3 %sassignment-trace-this-op-001.php:12
                             => $this->test |= 65535 %sassignment-trace-this-op-001.php:13
                             => $this->test &= 65295 %sassignment-trace-this-op-001.php:14
                             => $this->test ^= 61455 %sassignment-trace-this-op-001.php:15
                             => $this->test **= 2 %sassignment-trace-this-op-001.php:16
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-this-op-001.php:21
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
