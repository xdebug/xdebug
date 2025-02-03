--TEST--
Test for bug #2036: Segfault on fiber switch in finally block in garbage collected fiber (>= PHP 8.2)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.2; ext posix; ext pcntl');
?>
--INI--
xdebug.mode=develop
--FILE--
<?php

pcntl_signal(SIGTERM, function () {
    $f = new Fiber(function () {});
    $f->start();
});

posix_kill(posix_getpid(), SIGTERM);

pcntl_signal_dispatch();

echo "==DONE==\n";
?>
--EXPECTF--
Fatal error: Uncaught FiberError: Cannot switch fibers in current execution context in %sbug02036-php84.php on line 5

FiberError: Cannot switch fibers in current execution context in %sbug02036-php84.php on line 5

Call Stack:
%w%f %w%d   1. {main}() %sbug02036-php84.php:0
%w%f %w%d   2. pcntl_signal_dispatch() %sbug02036-php84.php:10
%w%f %w%d   3. {closure:%sbug02036-php84.php:3-6}(%S) %sbug02036-php84.php:10
%w%f %w%d   4. Fiber->start() %sbug02036-php84.php:5
