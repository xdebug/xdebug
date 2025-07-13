--TEST--
Test for bug #2332: Crash with coverage and nested fibers
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
require __DIR__ . '/../utils.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

require dirname(__FILE__) . '/bug02332.inc';

$coverage = xdebug_get_code_coverage();
xdebug_stop_code_coverage();

ksort( $coverage );
$fileInfo = array_values( array_slice( $coverage, 0, 1 ) )[0];

mustBeExecuted( $fileInfo, [ 5, 6, 7, 8, 9 ] );
?>
--EXPECTF--
bool(true)
bool(true)
line #5 is present and covered
line #6 is present and covered
line #7 is present and covered
line #8 is present and covered
line #9 is present and covered
