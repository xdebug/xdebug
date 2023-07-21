--TEST--
Test for bug #2020: segfault if xdebug.dump.GET=* and integer key without value in URL
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
