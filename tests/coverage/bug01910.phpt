--TEST--
Test for bug #1910: Code coverage misses constructor property promotion code
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
require __DIR__ . '/../utils.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

require dirname(__FILE__) . '/bug01910.inc';

$coverage = xdebug_get_code_coverage();
xdebug_stop_code_coverage();

ksort( $coverage );
$fileInfo = array_values( array_slice( $coverage, 0, 1 ) )[0];

mustBeExecuted( $fileInfo, [ 8, 11, 14, 16 ] );
?>
--EXPECTF--
line #8 is present and covered
line #11 is present and covered
line #14 is present and covered
line #16 is present and covered
