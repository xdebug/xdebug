--TEST--
Test for bug #1241: Xdebug doesn't handle FAST_RET and FAST_CALL opcodes for branch/dead code analysis (< PHP 7.4, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.4; !opcache');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

include 'bug01241-002.inc';

$u = new Unzip;
$u->extractFile();

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
ksort($c);
var_dump($c);
?>
--EXPECTF--
before true
Do somthing
array(2) {
  ["%sbug01241-002-php72.php"]=>
  array(4) {
    [4]=>
    int(1)
    [6]=>
    int(1)
    [7]=>
    int(1)
    [9]=>
    int(1)
  }
  ["%sbug01241-002.inc"]=>
  array(9) {
    [2]=>
    int(1)
    [7]=>
    int(1)
    [8]=>
    int(1)
    [9]=>
    int(-2)
    [10]=>
    int(-2)
    [11]=>
    int(1)
    [13]=>
    int(-2)
    [14]=>
    int(-2)
    [17]=>
    int(1)
  }
}
