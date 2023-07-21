--TEST--
Test for bug #1922: Code coverage misses array assignment lines
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

require dirname(__FILE__) . '/bug01922.inc';

$a = new ExampleTest;
$a->arrayBuild();

$coverage = xdebug_get_code_coverage();
xdebug_stop_code_coverage();

ksort( $coverage );
$fileInfo = array_values( array_slice( $coverage, 0, 1 ) )[0];

mustBeExecuted( $fileInfo, [ 12, 18, 27 ] );
?>
--EXPECTF--
line #12 is present and covered
line #18 is present and covered
line #27 is present and covered
