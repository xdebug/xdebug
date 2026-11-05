--TEST--
Test for bug #2392: Count how often a line of code was executed [1]
--INI--
xdebug.mode=coverage
--FILE--
<?php
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_HIT_COUNT);
$file = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'bug02392.inc';
include $file;
$cc = xdebug_get_code_coverage();
xdebug_stop_code_coverage();
var_dump($cc[$file]);
?>
--EXPECT--
array(10) {
  [4]=>
  int(5)
  [5]=>
  int(-2)
  [9]=>
  int(2)
  [10]=>
  int(7)
  [11]=>
  int(5)
  [13]=>
  int(2)
  [14]=>
  int(-2)
  [16]=>
  int(1)
  [17]=>
  int(1)
  [18]=>
  int(1)
}
