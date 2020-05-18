--TEST--
Test for bug #1241: Xdebug doesn't handle FAST_RET and FAST_CALL opcodes for branch/dead code analysis (>= PHP 7.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

include 'bug01241.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
ksort($c);
var_dump($c);
?>
--EXPECTF--
try
finally
end
array(2) {
  ["%sbug01241-001-php74.php"]=>
  array(2) {
    [4]=>
    int(1)
    [6]=>
    int(1)
  }
  ["%sbug01241.inc"]=>
  array(11) {
    [5]=>
    int(1)
    [6]=>
    int(-1)
    [7]=>
    int(-1)
    [8]=>
    int(-1)
    [9]=>
    int(-1)
    [10]=>
    int(1)
    [11]=>
    int(1)
    [13]=>
    int(1)
    [14]=>
    int(1)
    [16]=>
    int(1)
    [18]=>
    int(1)
  }
}
