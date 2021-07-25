--TEST--
Test for bug #1732 - Trace flamegraph format output for memory
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

$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE), XDEBUG_TRACE_FLAMEGRAPH_MEM);

A();

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
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
