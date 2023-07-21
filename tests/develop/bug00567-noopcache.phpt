--TEST--
Test for bug #567: xdebug_debug_zval() and xdebug_debug_zval_stdout() don't work (!opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
?>
--INI--
xdebug.mode=develop
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
a: (%s, is_ref=0)='hoge'
a: (%s, is_ref=0)='hoge'(%d)
