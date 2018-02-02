--TEST--
Test for bug #879: Closing brace in trait-using class definitions is counted towards code coverage (>= PHP 7.0, <= PHP 7.0.12)
--SKIPIF--
<?php
if (!version_compare(phpversion(), "7.0", '>=')) echo "skip >= PHP 7.0, <= PHP 7.0.12 needed\n";
if (version_compare(phpversion(), "7.0.12", '>')) echo "skip >= PHP 7.0, <= PHP 7.0.12 needed\n";
?>
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
  ["%sbug00879-php70.php"]=>
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
  ["%sbug00879.inc"]=>
  array(4) {
    [3]=>
    int(1)
    [5]=>
    int(1)
    [6]=>
    int(1)
    [7]=>
    int(1)
  }
}
