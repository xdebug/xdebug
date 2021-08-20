--TEST--
Test for tracing array assign ops
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
require_once 'capture-trace.inc';

$a['foo'][3]['test'] = 0;

$a['foo'][3]['test'] += 42;
$a['foo'][3]['test'] -= 2;
$a['foo'][3]['test'] *= 2;
$a['foo'][3]['test'] /= 5;
$a['foo'][3]['test'] %= 4;
$a['foo'][3]['test'] <<= 1;
$a['foo'][3]['test'] >>= 3;
$a['foo'][3]['test'] |= 0xffff;
$a['foo'][3]['test'] &= 0xff0f;
$a['foo'][3]['test'] ^= 0xf00f;
$a['foo'][3]['test'] **= 2;

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
                           => $a['foo'][3]['test'] = 0 %sassignment-trace-dim-op.php:4
                           => $a['foo'][3]['test'] += 42 %sassignment-trace-dim-op.php:6
                           => $a['foo'][3]['test'] -= 2 %sassignment-trace-dim-op.php:7
                           => $a['foo'][3]['test'] *= 2 %sassignment-trace-dim-op.php:8
                           => $a['foo'][3]['test'] /= 5 %sassignment-trace-dim-op.php:9
                           => $a['foo'][3]['test'] %= 4 %sassignment-trace-dim-op.php:10
                           => $a['foo'][3]['test'] <<= 1 %sassignment-trace-dim-op.php:11
                           => $a['foo'][3]['test'] >>= 3 %sassignment-trace-dim-op.php:12
                           => $a['foo'][3]['test'] |= 65535 %sassignment-trace-dim-op.php:13
                           => $a['foo'][3]['test'] &= 65295 %sassignment-trace-dim-op.php:14
                           => $a['foo'][3]['test'] ^= 61455 %sassignment-trace-dim-op.php:15
                           => $a['foo'][3]['test'] **= 2 %sassignment-trace-dim-op.php:16
%w%f %w%d     -> xdebug_stop_trace() %sassignment-trace-dim-op.php:18
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
