--TEST--
Test for bug #1241: Xdebug doesn't handle FAST_RET and FAST_CALL opcodes for branch/dead code analysis (>= PHP 7.0.3, PHP <= 7.0.12)
--SKIPIF--
<?php
if (!version_compare(phpversion(), "7.0.3", '>=')) echo "skip >= PHP 7.0.3, <= PHP 7.0.12 needed\n";
if (version_compare(phpversion(), "7.0.12", '>')) echo "skip >= PHP 7.0.3, <= PHP 7.0.12 needed\n";
?>
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
  ["%sbug01241-php70.php"]=>
  array(2) {
    [4]=>
    int(1)
    [6]=>
    int(1)
  }
  ["%sbug01241.inc"]=>
  array(11) {
    [2]=>
    int(1)
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
  }
}
