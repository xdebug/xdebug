--TEST--
Test for bug #1779: INIT_NS_FCALL_BY_NAME opcode, used for built-in functions in namespace, is not overloaded
--INI--
xdebug.mode=coverage
--FILE--
<?php
require __DIR__ . '/../utils.inc';

xdebug_start_code_coverage( XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE );

include dirname( __FILE__ ) . '/bug01779.inc';

$cc = xdebug_get_code_coverage();

ksort( $cc );
$fileInfo = array_values( array_slice( $cc, 0, 1 ) )[0];
mustBeExecuted( $fileInfo, range( 8, 11 ) );
?>
--EXPECTF--
line #8 is present and covered
line #9 is present and covered
line #10 is present and covered
line #11 is present and covered
