--TEST--
Test with xdebug_get_declared_vars()
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.collect_params=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.dump_globals=0
xdebug.collect_vars=1
xdebug.show_local_vars=0
xdebug.overload_var_dump=0
--FILE--
<?php
	function a($a,$b) {
		var_dump(xdebug_get_declared_vars());
	}
	a(52, 52);

	function b($a,$b) {
		echo $a;
		echo $b, "\n";
		var_dump(xdebug_get_declared_vars());
	}
	b(52, 52);

	function c($a,$b) {
		echo $a;
		echo $b, "\n";
		unset($b);
		var_dump(xdebug_get_declared_vars());
	}
	c(3.14, 159);

	function d($a,$b) {
		$c = 3;
		$d = 4;
		echo $a, "\n";
		var_dump(xdebug_get_declared_vars());
	}
	d(1, 2);

	function s()
	{
		$c = 42;
		$d = 54;
		echo $c, $d, "\n";
		var_dump(xdebug_get_declared_vars());
	}

	register_shutdown_function('s');
?>
--EXPECTF--
array(2) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
}
5252
array(2) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
}
3.14159
array(2) {
  [0]=>
  string(1) "a"
  [1]=>
  string(1) "b"
}
1
array(4) {
  [0]=>
  string(1) "d"
  [1]=>
  string(1) "a"
  [2]=>
  string(1) "c"
  [3]=>
  string(1) "b"
}
4254
array(2) {
  [0]=>
  string(1) "d"
  [1]=>
  string(1) "c"
}
