--TEST--
Test for bug #567: xdebug_debug_zval() and xdebug_debug_zval_stdout() don't work (NTS, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/utils.inc';
check_reqs('NTS; !opcache');
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
a: (%s, is_ref=0)='hoge'
a: (%s, is_ref=0)='hoge'(%d)
