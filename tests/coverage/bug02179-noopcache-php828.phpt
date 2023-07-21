--TEST--
Test for bug #2179: Coverage mistakes with if/else without curly braces (!opcache, <= PHP 8.2.8) 
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP <= 8.2.8; !opcache');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
require __DIR__ . '/../utils.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

require dirname(__FILE__) . '/bug02179.inc';

getByFilename();
getByFilename2();

$coverage = xdebug_get_code_coverage();
xdebug_stop_code_coverage();

ksort( $coverage );
$fileInfo = array_values( array_slice( $coverage, 1, 1 ) )[0];

mustBeExecuted( $fileInfo, [ 5, 7, 12, 16, 20, 22, 24, 28 ] );
?>
--EXPECTF--
line #5 is present and covered
line #7 is present and covered
line #12 is present and covered
line #16 is present and covered
line #20 is present and covered
line #22 is present and covered
line #24 is present and covered
line #28 is present and covered
