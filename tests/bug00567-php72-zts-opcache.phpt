--TEST--
Test for bug #567: xdebug_debug_zval() and xdebug_debug_zval_stdout() don't work (>= PHP 7.2, ZTS, opcache)
--SKIPIF--
<?php
require 'tests/utils.inc';
if (PHP_ZTS == 0) echo "skip ZTS needed\n";

if ( ! ( runtime_version('7.2', '>=') && opcache_active() ) ) {
	echo "skip >= PHP 7.2 && opcache loaded needed\n";
}
?>
--INI--
xdebug.default_enable=1
xdebug.overload_var_dump=2
--FILE--
<?php
function func(){
	$a="hoge";
	xdebug_debug_zval( 'a' );
	xdebug_debug_zval_stdout( 'a' );
}

func();
?>
--EXPECTF--
a: no such symbol
a: no such symbol
