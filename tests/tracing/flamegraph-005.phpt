--TEST--
Tracing: Flamegraph with "start with request"
--INI--
xdebug.mode=trace
xdebug.start_with_request=yes
xdebug.trace_output_name=trace.%p.%r
xdebug.trace_format=3
--FILE--
<?php
$tf = xdebug_get_tracefile_name();

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

if (preg_match('@\.gz$@', $tf)) {
	$fp = gzopen($tf, 'r');
	echo stream_get_contents($fp);
} else {
	echo file_get_contents($tf);
}
?>
--EXPECTF--
{main};xdebug_get_tracefile_name %d
{main};A;AA %d
{main};A;AB;ABA %d
{main};A;AB;ABB %d
{main};A;AB %d
{main};A;AA %d
{main};A;AB;ABA %d
{main};A;AB;ABB %d
{main};A;AB %d
{main};A;AC %d
{main};A %d
