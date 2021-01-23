--TEST--
Test for bug #1932: Code coverage misses array assignment lines
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
require __DIR__ . '/../utils.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

require dirname(__FILE__) . '/bug01932.inc';

$a = new \Service\Example;
$a->executeAction();

$coverage = xdebug_get_code_coverage();
xdebug_stop_code_coverage();

ksort( $coverage );
$fileInfo = array_values( array_slice( $coverage, 0, 1 ) )[0];

mustBeExecuted( $fileInfo, [ 9, 10, 11 ] );
?>
--EXPECTF--
line #9 is present and covered
line #10 is present and covered
line #11 is present and covered
