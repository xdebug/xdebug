--TEST--
Test for bug #567: xdebug_debug_zval() and xdebug_debug_zval_stdout() don't work (>= PHP 7.0, < PHP 7.1, NTS, opcache)
--SKIPIF--
<?php
if (PHP_ZTS == 1) echo "skip NTS needed\n";
if (!version_compare(phpversion(), "7.0", '>=')) echo "skip >= PHP 7.0, < PHP 7.1 needed\n";
if (!version_compare(phpversion(), "7.1", '<')) echo "skip >= PHP 7.0, < PHP 7.1 needed\n";
if (!extension_loaded('zend opcache')) echo "skip opcache required\n";
?>
--INI--
xdebug.default_enable=1
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
a: (refcount=0, is_ref=0)='hoge'
a: (refcount=0, is_ref=0)='hoge'(29)
