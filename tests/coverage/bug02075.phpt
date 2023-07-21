--TEST--
Test for bug #2075: Code coverage misses static array assignment lines
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

require dirname(__FILE__) . '/bug02075.inc';

Issue2075::showMe();

$coverage = xdebug_get_code_coverage();
xdebug_stop_code_coverage();

ksort( $coverage );
$fileInfo = array_values( array_slice( $coverage, 0, 1 ) )[0];

mustBeExecuted( $fileInfo, [ 10, 11, 12, 14 ] );
?>
--EXPECTF--
line #10 is present and covered
line #11 is present and covered
line #12 is present and covered
line #14 is present and covered
