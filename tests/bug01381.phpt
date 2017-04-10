--TEST--
Test for bug #1381: FETCH_DIM_W is not overloaded
--SKIPIF--
<?php if (extension_loaded('zend opcache')) echo "skip opcache should not be loaded\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.trace_options=0
xdebug.trace_output_dir=/tmp
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.extended_info=1
xdebug.coverage_enable=1
xdebug.overload_var_dump=0
--FILE--
<?php
include 'bug01381.inc';

$test = new test();
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);
$test->testFunc();

$cc = xdebug_get_code_coverage();
ksort($cc);
var_dump(array_slice($cc, 0, 1));

xdebug_stop_code_coverage(false);
?>
--EXPECTF--
array(1) {
  ["%sbug01381.inc"]=>
  array(9) {
    [6]=>
    int(1)
    [7]=>
    int(1)
    [8]=>
    int(1)
    [11]=>
    int(1)
    [12]=>
    int(1)
    [13]=>
    int(1)
    [14]=>
    int(1)
    [20]=>
    int(1)
    [21]=>
    int(-2)
  }
}
