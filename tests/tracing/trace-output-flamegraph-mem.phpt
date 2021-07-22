--TEST--
Test flamegraph function traces
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

$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE), XDEBUG_TRACE_FLAMEGRAPH_MEM);

A();

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
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
