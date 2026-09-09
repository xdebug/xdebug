--TEST--
Test for bug #2392: Count how often a line of code was executed [2] (stop/start resets counts)
--INI--
xdebug.mode=coverage
--FILE--
<?php
$file = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'bug02392-002.inc';
include $file;

xdebug_start_code_coverage(XDEBUG_CC_HIT_COUNT);
loops(4);
xdebug_stop_code_coverage();

xdebug_start_code_coverage(XDEBUG_CC_HIT_COUNT);
loops(4);
$cc = xdebug_get_code_coverage();
xdebug_stop_code_coverage();
var_dump($cc[$file]);
?>
--EXPECT--
array(4) {
  [4]=>
  int(1)
  [5]=>
  int(5)
  [6]=>
  int(4)
  [8]=>
  int(1)
}
