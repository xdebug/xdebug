--TEST--
Test for bug #1713: INIT_FCALL is not overloaded in code coverage
--INI--
xdebug.mode=coverage
--FILE--
<?php
require __DIR__ . '/../utils.inc';

xdebug_start_code_coverage( XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE );

include dirname( __FILE__ ) . '/bug01713.inc';

$cc = xdebug_get_code_coverage();

ksort( $cc );
$fileInfo = array_values( array_slice( $cc, 0, 1 ) )[0];

mustBeExecuted( $fileInfo, range( 12, 14 ) );
?>
--EXPECTF--
line #12 is present and covered
line #13 is present and covered
line #14 is present and covered
