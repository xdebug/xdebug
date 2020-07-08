--TEST--
Test for bug #515: Dead Code Analysis for code coverage messed up with ticks (!opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
?>
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
	xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

	include 'bug00515.inc';
	$cc = xdebug_get_code_coverage();
	ksort($cc);
	var_dump(array_slice($cc, 0, 1));

	xdebug_stop_code_coverage(false);
?>
--EXPECTF--
array(1) {
  ["%sbug00515.inc"]=>
  array(9) {
    [2]=>
    int(1)
    [%r(3|5)%r]=>
    int(1)
    [9]=>
    int(-1)
    [10]=>
    int(-1)
    [12]=>
    int(-1)
    [14]=>
    int(-1)
    [18]=>
    int(-1)
    [19]=>
    int(-2)
    [22]=>
    int(1)
  }
}
