--TEST--
Tracing: Flamegraph for time
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=3
--FILE--
<?php
require_once 'capture-trace.inc';

function ABB() {
}

function ABA() {
}

function AC() {
}

function AB() {
    ABA();
    ABB();
}

function AA() {
}

function A() {
    AA();
    AB();
    AA();
    AB();
    AC();
}

A();

xdebug_stop_trace();
?>
--EXPECTF--
A;AA %d
A;AB;ABA %d
A;AB;ABB %d
A;AB %d
A;AA %d
A;AB;ABA %d
A;AB;ABB %d
A;AB %d
A;AC %d
A %d
