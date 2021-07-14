--TEST--
Test for bug #1992: Code Coverage with filter can crash on xdebug_stop_code_coverage() 
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=coverage
auto_prepend_file=tests/coverage/bug01992-filter.inc
--FILE--
<?php
xdebug_start_code_coverage();
xdebug_stop_code_coverage(true);
?>
--EXPECTF--
