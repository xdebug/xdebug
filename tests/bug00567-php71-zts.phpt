--TEST--
Test for bug #567: xdebug_debug_zval() and xdebug_debug_zval_stdout() don't work (>= PHP 7.1, ZTS)
--SKIPIF--
<?php if (PHP_ZTS == 0) echo "skip ZTS needed\n"; ?>
<?php if (!version_compare(phpversion(), "7.1", '>=')) echo "skip >= PHP 7.1 needed\n"; ?>
<?php if (extension_loaded('zend opcache')) echo "skip opcache should not be loaded\n"; ?>
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
--EXPECT--
a: (refcount=2, is_ref=0)='hoge'
a: (refcount=2, is_ref=0)='hoge'(29)
