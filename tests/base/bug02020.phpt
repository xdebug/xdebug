--TEST--
Test for bug #2020: segfault if xdebug.dump.GET=* and integer key without value in URL
--SKIPIF--
<?php
if (PHP_OS_FAMILY === "Windows") {
    die("skip unsupported on Windows due to the fix for CVE-2024-8926");
}
?>
--INI--
xdebug.dump.GET=*
xdebug.mode=develop
--GET--
1
--FILE--
<?php
throw new Exception("OOPS");
?>
--EXPECTF--
Fatal error: Uncaught Exception: OOPS in %sbug02020.php on line 2

Exception: OOPS in %sbug02020.php on line 2

Call Stack:
%w%f %w%d   1. {main}() %sbug02020.php:0

Dump $_GET
   $_GET[1] = ''
