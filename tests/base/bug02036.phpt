--TEST--
Test for bug #2036: Segfault on fiber switch in finally block in garbage collected fiber (>= PHP 8.1)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
xdebug.mode=develop
--FILE--
<?php

$fiberA = new Fiber(function () {
    Fiber::suspend();
});

$fiberB = new Fiber(function () use ($fiberA) {
    try {
        Fiber::suspend();
    } finally {
        $fiberA->start();
    }
});

$fiberB->start();

echo "==DONE==\n";
?>
--EXPECTF--
==DONE==

Fatal error: Uncaught FiberError: Cannot switch fibers in current execution context in %sbug02036.php on line 11

FiberError: Cannot switch fibers in current execution context in %sbug02036.php on line 11

Call Stack:
%w%f %w%d   1. {fiber:%s}() %sbug02036.php:15
%w%f %w%d   2. {closure:%sbug02036.php:7-13}() %sbug02036.php:15
%w%f %w%d   3. Fiber->start() %sbug02036.php:11
