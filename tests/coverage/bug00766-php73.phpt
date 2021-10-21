--TEST--
Test for bug #766: Xdebug doesn't understand ZEND_RETURN_BY_REF (< PHP 7.4, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.4; !opcache');
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

	include 'bug00766.inc';
	$cc = xdebug_get_code_coverage();
	ksort($cc);
	var_dump(array_slice($cc, 1, 1));

	xdebug_stop_code_coverage(false);
?>
--EXPECTF--
array(1) {
  ["%sbug00766.inc"]=>
  array(5) {
    [2]=>
    int(1)
    [6]=>
    int(1)
    [7]=>
    int(-2)
    [10]=>
    int(1)
    [11]=>
    int(1)
  }
}
