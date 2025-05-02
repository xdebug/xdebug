--TEST--
Test for bug #2339: Trying to throw an exception can cause a zend_mm_heap corrupted error under specific circumstances
--INI--
xdebug.mode=develop
--FILE--
<?php
require_once __DIR__ . '/bug02339-index.inc';
?>
--EXPECTF--
Fatal error: Uncaught Exception in %s

Exception:  in %sbug02339-second-file.inc on line %d

Call Stack:
%w%f %w%d   1. {main}() %sbug02339.php:0
%w%f %w%d   2. require_once('%sbug02339-index.inc') %sbug02339.php:%d
%w%f %w%d   3. require_once('%sbug02339-second-file.inc') %sbug02339-index.inc:%d
