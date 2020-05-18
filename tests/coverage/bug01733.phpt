--TEST--
Test for bug #1733: SEND_VAR_NO_REF_EX opcode, used for require() in namespace, is not overloaded
--INI--
xdebug.mode=coverage
--FILE--
<?php
require __DIR__ . '/../utils.inc';

xdebug_start_code_coverage( XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE );

include dirname( __FILE__ ) . '/bug01733.inc';

$cc = xdebug_get_code_coverage();

ksort( $cc );
$fileInfo = array_values( array_slice( $cc, 0, 1 ) )[0];
mustBeExecuted( $fileInfo, range( 8, 10 ) );
?>
--EXPECTF--
array(2) {
  [0]=>
  string(%d) "%s"
  [1]=>
  string(%d) "%s"
}
line #8 is present and covered
line #9 is present and covered
line #10 is present and covered
