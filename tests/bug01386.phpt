--TEST--
Test for bug #1386: Executable code not shown as executed/executable
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
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

include 'bug01386-class2.inc';
include 'bug01386-class1.inc';

$Test1 = new TestClass();

$cc = xdebug_get_code_coverage();
ksort($cc);
var_dump(array_slice($cc, 0, 2));

xdebug_stop_code_coverage(false);
?>
--EXPECTF--
array(2) {
  ["%sbug01386-class1.inc"]=>
  array(6) {
    [3]=>
    int(1)
    [7]=>
    int(-1)
    [8]=>
    int(-1)
    [9]=>
    int(-1)
    [10]=>
    int(-2)
    [13]=>
    int(1)
  }
  ["%sbug01386-class2.inc"]=>
  array(6) {
    [3]=>
    int(1)
    [7]=>
    int(-1)
    [8]=>
    int(-1)
    [9]=>
    int(-1)
    [10]=>
    int(-2)
    [13]=>
    int(1)
  }
}

