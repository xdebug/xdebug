--TEST--
Test for bug #879: Closing brace in trait-using class definitions is counted towards code coverage
--INI--
xdebug.mode=coverage
xdebug.trace_options=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.trace_format=0
--FILE--
<?php

xdebug_start_code_coverage(XDEBUG_CC_UNUSED);

$file = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'bug00879.inc';
include $file;

new WithTrait;

$cc = xdebug_get_code_coverage();
ksort($cc);
var_dump($cc);
?>
--EXPECTF--
array(2) {
  ["%sbug00879.inc"]=>
  array(2) {
    [5]=>
    int(1)
    [8]=>
    int(1)
  }
  ["%sbug00879.php"]=>
  array(4) {
    [5]=>
    int(1)
    [6]=>
    int(1)
    [8]=>
    int(1)
    [10]=>
    int(1)
  }
}
