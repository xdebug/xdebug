--TEST--
Test with Code Coverage (>= PHP 7.0, <= PHP 7.0.12)
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
	xdebug_start_code_coverage();
	$file = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'coverage.inc';
	include $file;
	$cc = xdebug_get_code_coverage();
	xdebug_stop_code_coverage();
	var_dump($cc[$file]);
?>
--EXPECTF--
This is a YYYY-MM-DD format.
This is a YYYYMMDD HHii format.
array(11) {
  [2]=>
  int(1)
  [4]=>
  int(1)
  [7]=>
  int(1)
  [8]=>
  int(1)
  [10]=>
  int(1)
  [11]=>
  int(1)
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
}
