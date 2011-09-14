--TEST--
Test for bug #213: Dead code analysis doesn't take catches for throws into account
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
xdebug.coverage_enable=1
--FILE--
<?php
	xdebug_start_code_coverage( XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE );
	$file = realpath('./tests/bug00213.inc');
	include $file;
	$cc = xdebug_get_code_coverage();
	xdebug_stop_code_coverage();
	var_dump($cc[$file]);
?>
--EXPECT--
48
array(5) {
  [5]=>
  int(1)
  [6]=>
  int(-2)
  [8]=>
  int(1)
  [12]=>
  int(1)
  [14]=>
  int(1)
}
