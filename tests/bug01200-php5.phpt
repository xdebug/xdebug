--TEST--
Test for bug #1200: Coverage of sending arguments to a method (< PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
--FILE--
<?php
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

include 'bug01200.inc';

$cc = xdebug_get_code_coverage();
ksort($cc);
var_dump(array_slice($cc, 1, 1));

xdebug_stop_code_coverage(false);
?>
--EXPECTF--
array(1) {
  ["%sbug01200.inc"]=>
  array(9) {
    [3]=>
    int(1)
    [6]=>
    int(1)
    [9]=>
    int(1)
    [11]=>
    int(1)
    [12]=>
    int(1)
    [13]=>
    int(1)
    [14]=>
    int(1)
    [15]=>
    int(1)
    [17]=>
    int(1)
  }
}
