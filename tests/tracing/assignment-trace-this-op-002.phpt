--TEST--
Test for tracing $this property assign ops [2]
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
	$this->test = ['foo' => 0];
	$this->test['foo'] += 42;
	$this->test['foo'] -= 2;
	$this->test['foo'] *= 2;
	$this->test['foo'] /= 5;
	$this->test['foo'] %= 4;
	$this->test['foo'] <<= 1;
	$this->test['foo'] >>= 3;
	$this->test['foo'] |= 0xffff;
	$this->test['foo'] &= 0xff0f;
	$this->test['foo'] ^= 0xf00f;
	$this->test['foo'] **= 2;
} }

$a->assign();

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
                           => $a = class test { public $test = NULL } %sassignment-trace-this-op-002.php:3
%w%f %w%d     -> test->assign() %sassignment-trace-this-op-002.php:19
                             => $this->test = ['foo' => 0] %sassignment-trace-this-op-002.php:5
                             => $this->test['foo'] += 42 %sassignment-trace-this-op-002.php:6
                             => $this->test['foo'] -= 2 %sassignment-trace-this-op-002.php:7
                             => $this->test['foo'] *= 2 %sassignment-trace-this-op-002.php:8
                             => $this->test['foo'] /= 5 %sassignment-trace-this-op-002.php:9
                             => $this->test['foo'] %= 4 %sassignment-trace-this-op-002.php:10
                             => $this->test['foo'] <<= 1 %sassignment-trace-this-op-002.php:11
                             => $this->test['foo'] >>= 3 %sassignment-trace-this-op-002.php:12
                             => $this->test['foo'] |= 65535 %sassignment-trace-this-op-002.php:13
                             => $this->test['foo'] &= 65295 %sassignment-trace-this-op-002.php:14
                             => $this->test['foo'] ^= 61455 %sassignment-trace-this-op-002.php:15
                             => $this->test['foo'] **= 2 %sassignment-trace-this-op-002.php:16
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-this-op-002.php:21
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
