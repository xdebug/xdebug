--TEST--
Tracing: Flamegraph for memory
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=4
--FILE--
<?php
require_once 'capture-trace.inc';

// This forcefully creates some memory leaks to yield meaningful numbers
// in the generated output, so that we will have some results to see.
global $memoryLeak;
$memoryLeak = '';

function ABB() {
	global $memoryLeak;
    for ($i = 0; $i < 10000; $i++) {
        $memoryLeak .= 'a';
    }
}

function ABA() {
}

function AC() {
	global $memoryLeak;
    for ($i = 0; $i < 10000; $i++) {
        $memoryLeak .= 'a';
    }
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
A;AA 0
A;AB;ABA 0
A;AB;ABB %d
A;AB 0
A;AA 0
A;AB;ABA 0
A;AB;ABB %d
A;AB 0
A;AC %d
A %d
