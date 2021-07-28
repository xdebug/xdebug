--TEST--
Tracing: Flamegraph with fiber
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=3
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1; tracing');
?>
--FILE--
<?php

$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE), XDEBUG_TRACE_FLAMEGRAPH_COST);

require dirname(__FILE__) . '/fiber-001.inc';

xdebug_stop_trace();

echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
dirname %d
require;Fiber->__construct %d
require;Fiber->__construct %d
{closure:%s/fiber-001.inc:27-29};A;AA %d
{closure:%s/fiber-001.inc:27-29};A;Fiber::suspend %d
{closure:%s/fiber-001.inc:31-33};B;BA %d
{closure:%s/fiber-001.inc:31-33};B;Fiber::suspend %d
require;B;Fiber->resume %d
{closure:%s/fiber-001.inc:31-33};B;AB %d
{closure:%s/fiber-001.inc:31-33};B %d
{closure:%s/fiber-001.inc:31-33} %d
require;Fiber->start %d
require;Fiber->resume %d
{closure:%s/fiber-001.inc:27-29};A;BB %d
{closure:%s/fiber-001.inc:27-29};A %d
{closure:%s/fiber-001.inc:27-29} %d
require;Fiber->start %d
require 72288
