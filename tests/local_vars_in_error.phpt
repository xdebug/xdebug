--TEST--
Test with showing local variables on errors (ZE2)
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.dump_globals=0
xdebug.show_local_vars=1
--FILE--
<?php
	function a($a,$b) {
		$c = array($a, $b * $b);
		$d = new stdClass;
		do_f();
	}

	a(5, 6);
?>
--EXPECTF--
Fatal error: Call to undefined function do_f() in /%s/local_vars_in_error.php on line 5

Call Stack:
%w%f %w%d   1. {main}() /%s/local_vars_in_error.php:0
%w%f %w%d   2. a(5, 6) /%s/local_vars_in_error.php:8


Variables in local scope:
  $d = class stdClass {  }
  $a = 5
  $c = array (0 => 5, 1 => 36)
  $b = 6
