--TEST--
Test for bug #1446: Code Coverage misses elseif if it uses an isset with a property
--INI--
xdebug.mode=coverage
--FILE--
<?php
xdebug_start_code_coverage( XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE );

include dirname( __FILE__ ) . '/bug01446.inc';

$o = new Test;
$o->doSomething();

$cc = xdebug_get_code_coverage();

ksort( $cc );
var_dump( array_slice( $cc, 0, 1 ) );
?>
--EXPECTF--
array(1) {
  ["%sbug01446.inc"]=>
  array(%r(8|9|10)%r) {%A
    [8]=>
    int(1)
    [9]=>
    int(1)
    [13]=>
    int(1)
    [14]=>
    int(-1)
    [15]=>
    int(1)
    [16]=>
    int(1)
    [19]=>
    %a
    [23]=>
    int(1)
  }
}
