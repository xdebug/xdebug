--TEST--
Test for tracing array assignments in user-readable function traces
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
?>
--INI--
xdebug.mode=trace,develop
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
xdebug.force_error_reporting=0
--FILE--
<?php
require_once 'capture-trace.inc';

function test()
{
	$t = array( 'a' => 4, 'b' => 9, 'c' => 13 );
	$t['d'] = 89;
	$t['a'] += $b;
	@$t['a'] += $b;
	$t['c'] /= 7;
	$t['b'] *= 9;
}
$t = array();
$t['a'] = 98;
$t['b'] = 4;
$t['b'] -= 8;
$t['b'] *= -0.5;
$t['b'] <<= 1;
$t['c'] = $t['b'] / 32;

test(1, 2, 3);

xdebug_stop_trace();
?>
--EXPECTF--
%s: Undefined variable%sb in %sassignment-trace-002.php on line 8

Call Stack:
%w%f %w%d   1. {main}() %sassignment-trace-002.php:0
%w%f %w%d   2. test(1, 2, 3) %sassignment-trace-002.php:21

TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
                           => $t = [] %sassignment-trace-002.php:13
                           => $t['a'] = 98 %sassignment-trace-002.php:14
                           => $t['b'] = 4 %sassignment-trace-002.php:15
                           => $t['b'] -= 8 %sassignment-trace-002.php:16
                           => $t['b'] *= -0.5 %sassignment-trace-002.php:17
                           => $t['b'] <<= 1 %sassignment-trace-002.php:18
                           => $t['c'] = 0.125 %sassignment-trace-002.php:19
%w%f %w%d     -> test(1, 2, 3) %sassignment-trace-002.php:21
                             => $t = ['a' => 4, 'b' => 9, 'c' => 13] %sassignment-trace-002.php:6
                             => $t['d'] = 89 %sassignment-trace-002.php:7
                             => $t['a'] += %r(NULL|\*uninitialized\*)%r %sassignment-trace-002.php:8
                             => $t['a'] += %r(NULL|\*uninitialized\*)%r %sassignment-trace-002.php:9
                             => $t['c'] /= 7 %sassignment-trace-002.php:10
                             => $t['b'] *= 9 %sassignment-trace-002.php:11
%w%f %w%d     -> xdebug_stop_trace() %s:%d
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
