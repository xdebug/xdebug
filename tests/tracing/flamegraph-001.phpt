--TEST--
Tracing: Flamegraph for time
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=3
--FILE--
<?php
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

$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE), XDEBUG_TRACE_FLAMEGRAPH_COST);

A();

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
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
