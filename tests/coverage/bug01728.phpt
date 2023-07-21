--TEST--
Test for bug #1728: INIT_STATIC_METHOD_FCALL is not overloaded in code coverage
--INI--
xdebug.mode=coverage
--FILE--
<?php
require __DIR__ . '/../utils.inc';

xdebug_start_code_coverage( XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE );

include dirname( __FILE__ ) . '/bug01728.inc';

$cc = xdebug_get_code_coverage();

ksort( $cc );
$fileInfo = array_values( array_slice( $cc, 0, 1 ) )[0];
mustBeExecuted( $fileInfo, range( 6, 7 ) );
?>
--EXPECTF--
test
line #6 is present and covered
line #7 is present and covered
