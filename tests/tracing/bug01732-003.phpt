--TEST--
Test for bug #1732 - Trace flamegraph trace when xdebug_stop_trace() is called lower in the stack. 
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.trace_format=4
xdebug.dump_globals=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
--FILE--
<?php
global $tf;

function ABB() {
}

function ABA() {
}

function AC() {
}

function AB() {
	global $tf;
	if (!$tf) {
 		$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE), XDEBUG_TRACE_FLAMEGRAPH_MEM);
	}

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
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
ABA %d
ABB %d
AA %d
AB;ABA %d
AB;ABB %d
AB %d
AC %d
