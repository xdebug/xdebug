--TEST--
Test for bug #1679: Code Coverage misses static property as function argument
--INI--
xdebug.mode=coverage
--FILE--
<?php
require __DIR__ . '/../utils.inc';

xdebug_start_code_coverage( XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE );

include dirname( __FILE__ ) . '/bug01679.inc';

$o = new Test;
$o->doSomething();

$cc = xdebug_get_code_coverage();

ksort( $cc );
$fileInfo = array_values( array_slice( $cc, 0, 1 ) )[0];

mustBeExecuted( $fileInfo, [ 10, 11, 12, 14, 17 ] );
?>
--EXPECTF--
line #10 is present and covered
line #11 is present and covered
line #12 is present and covered
line #14 is present and covered
line #17 is present and covered
