--TEST--
Test for bug #1691: Code Coverage misses fluent interface function call
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
require __DIR__ . '/../utils.inc';

xdebug_start_code_coverage( XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE );

include dirname( __FILE__ ) . '/bug01691.inc';

new Sample(-0.123, 52.5);

$cc = xdebug_get_code_coverage();

ksort( $cc );
$fileInfo = array_values( array_slice( $cc, 0, 1 ) )[0];

mustBeExecuted( $fileInfo, [ 20, 21, 22, 23, 24 ] );
?>
--EXPECTF--
that: -0.123, latitude
range: -90, 90
that: 52.5, longitude
range: -140, 140
verifyNow: 
line #20 is present and covered
line #21 is present and covered
line #22 is present and covered
line #23 is present and covered
line #24 is present and covered
