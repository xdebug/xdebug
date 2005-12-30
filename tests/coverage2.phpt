--TEST--
Test with Code Coverage with unused lines
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
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
	xdebug_start_code_coverage(XDEBUG_CC_UNUSED);
	$file = realpath('./tests/coverage.inc');
	include $file;
	$cc = xdebug_get_code_coverage();
	xdebug_stop_code_coverage();
	var_dump($cc[$file]);
?>
--EXPECTF--
This is a YYYY-MM-DD format.
This is a YYYYMMDD HHii format.
array(%d) {
  [2]=>
  int(1)
  [4]=>
  int(1)
  [5]=>
  int(-1)
  [6]=>
  int(-1)
  [7]=>
  int(1)
  [8]=>
  int(1)
  [9]=>
  int(1)
  [10]=>
  int(1)
  [11]=>
  int(1)
  [12]=>
  int(1)
  [14]=>
  int(-1)
  [17]=>
  int(1)
  [18]=>
  int(1)
  [20]=>
  int(1)
  [21]=>
  int(1)
  [22]=>
  int(1)
  [23]=>
  int(1)
  [25]=>
  int(1)
}
