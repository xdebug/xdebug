--TEST--
Test with Code Coverage with abstract methods
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

	include 'coverage4.inc';

    xdebug_stop_code_coverage(false);
    $c = xdebug_get_code_coverage();
	ksort($c);
	var_dump($c);
?>
--EXPECTF--
array(2) {
  ["%scoverage4.inc"]=>
  array(2) {
    [25]=>
    int(1)
    [26]=>
    int(1)
  }
  ["%scoverage4.php"]=>
  array(2) {
    [4]=>
    int(1)
    [6]=>
    int(1)
  }
}
