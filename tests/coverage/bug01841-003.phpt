--TEST--
Test for bug #1841: 'match' keyword, no default [3]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0');
?>
--INI--
xdebug.mode=coverage
xdebug.start_with_request=trigger
opcache.optimization_level=0
--FILE--
<?php
require __DIR__ . '/../utils.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

require dirname(__FILE__) . '/bug01841-003.inc';

$cc = xdebug_get_code_coverage();

ksort( $cc );
$fileInfo = array_values( array_slice( $cc, 0, 1 ) )[0];
mustBeExecuted( $fileInfo, [ 4, 6 ] );
mustNotBeExecuted( $fileInfo, [ 7, 8, 12 ] );
?>
--EXPECTF--
line #4 is present and covered
line #6 is present and covered
line #7 is present and not covered
line #8 is present and not covered
line #12 is present and not covered
