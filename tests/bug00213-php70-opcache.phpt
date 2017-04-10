--TEST--
Test for bug #213: Dead code analysis doesn't take catches for throws into account (> PHP 7.0.12, < PHP 7.1, opcache)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0.12", '>')) echo "skip > PHP 7.0.12, < PHP 7.1 needed\n"; ?>
<?php if (!version_compare(phpversion(), "7.1", '<')) echo "skip > PHP 7.0.12, < PHP 7.1 needed\n"; ?>
<?php if (!extension_loaded('zend opcache')) echo "skip opcache required\n"; ?>
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
xdebug.overload_var_dump=0
--FILE--
<?php
	xdebug_start_code_coverage( XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE );
	$file = dirname(__FILE__) . DIRECTORY_SEPARATOR . 'bug00213.inc';
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
