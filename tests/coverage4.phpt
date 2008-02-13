--TEST--
Test with Code Coverage with abstract methods (ZE2)
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.trace_options=0
xdebug.trace_output_dir=/tmp
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.extended_info=1
--FILE--
<?php
    xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

	include 'coverage4.inc';

    xdebug_stop_code_coverage(false);
    var_dump(xdebug_get_code_coverage());
?>
--EXPECTF--
array(2) {
  ["%scoverage4.inc"]=>
  array(2) {
    [2]=>
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
