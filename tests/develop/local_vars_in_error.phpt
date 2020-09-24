--TEST--
Test with showing local variables on errors
--INI--
xdebug.mode=develop
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.show_local_vars=1
xdebug.show_error_trace=0
--FILE--
<?php
	function a($a,$b) {
		$c = array($a, $b * $b);
		$d = new stdClass;
		do_f($a, $b, $c, $d);
	}

	a(5, 6);
?>
--EXPECTF--
Fatal error: Uncaught Error: Call to undefined function do_f() in %slocal_vars_in_error.php on line 5

Error: Call to undefined function do_f() in %slocal_vars_in_error.php on line 5

Call Stack:
%w%f %w%d   1. {main}() %slocal_vars_in_error.php:0
%w%f %w%d   2. a($a = 5, $b = 6) %slocal_vars_in_error.php:8


Variables in local scope (#2):
  $a = 5
  $b = 6
  $c = [0 => 5, 1 => 36]
  $d = class stdClass {  }
