--TEST--
Test for bug #1200: Coverage of sending arguments to a method (< PHP 7.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.4');
?>
--INI--
xdebug.mode=coverage
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
  array(8) {
    [2]=>
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
    [17]=>
    int(1)
  }
}
