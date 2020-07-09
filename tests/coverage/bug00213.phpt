--TEST--
Test for bug #213: Dead code analysis doesn't take catches for throws into account (!opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
?>
--INI--
xdebug.mode=coverage
xdebug.trace_options=0
xdebug.collect_return=0
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.trace_format=0
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
