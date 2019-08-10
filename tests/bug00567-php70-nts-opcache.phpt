--TEST--
Test for bug #567: xdebug_debug_zval() and xdebug_debug_zval_stdout() don't work (< PHP 7.1, NTS, opcache)
--SKIPIF--
<?php
require __DIR__ . '/utils.inc';
check_reqs('PHP < 7.1; opcache; NTS');
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
--EXPECTF--
a: (interned, is_ref=0)='hoge'
a: (interned, is_ref=0)='hoge'(%d)
