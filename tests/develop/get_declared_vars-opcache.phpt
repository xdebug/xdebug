--TEST--
Test with xdebug_get_declared_vars() (opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('opcache');
?>
--INI--
xdebug.mode=develop
xdebug.collect_params=0
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.collect_vars=1
xdebug.show_local_vars=0
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
%sget_declared_vars-opcache.php:3:
array(2) {
  [0] =>
  string(1) "a"
  [1] =>
  string(1) "b"
}
5252
%sget_declared_vars-opcache.php:10:
array(2) {
  [0] =>
  string(1) "a"
  [1] =>
  string(1) "b"
}
3.14159
%sget_declared_vars-opcache.php:18:
array(2) {
  [0] =>
  string(1) "a"
  [1] =>
  string(1) "b"
}
1
%sget_declared_vars-opcache.php:26:
array(2) {
  [0] =>
  string(1) "a"
  [1] =>
  string(1) "b"
}
4254
%sget_declared_vars-opcache.php:35:
array(0) {
}
