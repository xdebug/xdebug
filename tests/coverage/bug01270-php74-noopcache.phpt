--TEST--
Test for bug #1270: String parsing marked not covered (>= PHP 7.4, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4; !opcache');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
xdebug_start_code_coverage( XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE );

include dirname( __FILE__ ) . '/bug01270.inc';

try { func1(); } catch (Exception $e) { }
try { func2(); } catch (Exception $e) { }
try { func3(); } catch (Exception $e) { }

$cc = xdebug_get_code_coverage();

ksort( $cc );
var_dump( array_slice( $cc, 1, 1 ) );
?>
--EXPECTF--
array(1) {
  ["%sbug01270.inc"]=>
  array(17) {
    [4]=>
    int(1)
    [5]=>
    int(1)
    [6]=>
    int(1)
    [7]=>
    int(1)
    [9]=>
    int(-2)
    [13]=>
    int(1)
    [14]=>
    int(1)
    [15]=>
    int(1)
    [16]=>
    int(1)
    [18]=>
    int(-2)
    [22]=>
    int(1)
    [23]=>
    int(1)
    [25]=>
    int(1)
    [27]=>
    int(1)
    [31]=>
    int(-2)
    [33]=>
    int(1)
    [35]=>
    int(-2)
  }
}
