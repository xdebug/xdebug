--TEST--
Test for bug #501: Xdebug's variable tracing misses POST_INC and variants
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.collect_return=0
xdebug.collect_assignments=1
--FILE--
<?php
require_once 'capture-trace.inc';

$i = 10;
$i +=
    ++$i
    + $i
    + $i++;

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
                           => $i = 10 %sbug00501.php:4
                           => ++$i %sbug00501.php:%r(7|6)%r
                           => $i++ %sbug00501.php:8
                           => $i += 33 %sbug00501.php:8
%w%f %w%d     -> xdebug_stop_trace() %s:%d
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]

