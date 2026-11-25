--TEST--
Test for lines of multi-line expressions that are executed, but not reported as executed
--INI--
xdebug.mode=coverage
--FILE--
<?php
require __DIR__ . '/../utils.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

require dirname(__FILE__) . '/multiline_expression.inc';

$coverage = xdebug_get_code_coverage();
xdebug_stop_code_coverage();

$fileInfo = $coverage[dirname(__FILE__) . '/multiline_expression.inc'];

mustBeExecuted( $fileInfo, [ 23, 28, 33, 38, 43, 48, 54, 59, 64, 69, 74, 79, 84, 89, 94, 99, 104, 109 ] );
mustNotBeExecuted( $fileInfo, [ 105, 106, 107, 108 ] );
?>
--EXPECT--
line #23 is present and covered
line #28 is present and covered
line #33 is present and covered
line #38 is present and covered
line #43 is present and covered
line #48 is present and covered
line #54 is present and covered
line #59 is present and covered
line #64 is present and covered
line #69 is present and covered
line #74 is present and covered
line #79 is present and covered
line #84 is present and covered
line #89 is present and covered
line #94 is present and covered
line #99 is present and covered
line #104 is present and covered
line #109 is present and covered
line #105 is present and not covered
line #106 is present and not covered
line #107 is present and not covered
line #108 is present and not covered
