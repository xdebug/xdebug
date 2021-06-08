--TEST--
Test for bug #1192: Dead code analysis does not work for generators with 'return;'
--INI--
xdebug.mode=coverage
--FILE--
<?php
xdebug_start_code_coverage (XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

include 'bug01192.inc';

testGen();

$cc = xdebug_get_code_coverage();
ksort($cc);
var_dump(array_slice($cc, 0, 1));

xdebug_stop_code_coverage();
?>
--EXPECTF--
array(1) {
  ["%sbug01192.inc"]=>
  array(13) {
    [2]=>
    int(1)
    [4]=>
    int(1)
    [6]=>
    int(1)
    [7]=>
    int(1)
    [8]=>
    int(1)
    [10]=>
    int(-1)
    [12]=>
    int(-1)
    [16]=>
    int(1)
    [17]=>
    int(1)
    [19]=>
    int(1)
    [20]=>
    int(1)
    [22]=>
    int(1)
    [23]=>
    int(1)
  }
}
