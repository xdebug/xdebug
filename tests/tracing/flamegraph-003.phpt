--TEST--
Tracing: Flamegraph output is consistent when xdebug_stop_trace() is called lower in the stack
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=4
--FILE--
<?php
function ABB() {
}

function ABA() {
}

function AC() {
}

function AB() {
	global $tf;

	require_once 'capture-trace.inc';

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
ABA %d
ABB %d
AA %d
AB;ABA %d
AB;ABB %d
AB %d
AC %d
