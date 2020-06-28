--TEST--
Test for xdebug_debug_zval() (!opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
?>
--INI--
xdebug.mode=develop
xdebug.cli_color=0
--FILE--
<?php
function func(){
	$a="hoge";
	$b = array('a' => 4, 'b' => 5, 'c' => 6, 8, 9);
	$c =& $b['b'];
	$d = $b['c'];
	$e = new stdClass;
	$e->foo = false;
	$e->bar =& $e->foo;
	$e->baz = array(4, 'b' => 42);
	xdebug_debug_zval( 'a' );
	xdebug_debug_zval( '$a' );
	xdebug_debug_zval( '$b' );
	xdebug_debug_zval( "\$b['a']" );
	xdebug_debug_zval( "\$b['b']" );
	xdebug_debug_zval( "b[1]" );
	xdebug_debug_zval( "c" );
	xdebug_debug_zval( "d" );
	xdebug_debug_zval( "e" );
	xdebug_debug_zval( "e->bar" );
	xdebug_debug_zval( "e->bar['b']" );
	xdebug_debug_zval( "e->baz[0]" );
	xdebug_debug_zval( "e->baz['b']" );
}

func();
?>
--EXPECTF--
a: (%s, is_ref=0)='hoge'
$a: (%s, is_ref=0)='hoge'
$b: (refcount=1, is_ref=0)=array ('a' => (refcount=0, is_ref=0)=4, 'b' => (refcount=2, is_ref=1)=5, 'c' => (refcount=0, is_ref=0)=6, 0 => (refcount=0, is_ref=0)=8, 1 => (refcount=0, is_ref=0)=9)
$b['a']: (refcount=0, is_ref=0)=4
$b['b']: (refcount=2, is_ref=1)=5
b[1]: (refcount=0, is_ref=0)=9
c: (refcount=2, is_ref=1)=5
d: (refcount=0, is_ref=0)=6
e: (refcount=1, is_ref=0)=class stdClass { public $foo = (refcount=2, is_ref=1)=FALSE; public $bar = (refcount=2, is_ref=1)=FALSE; public $baz = (%s, is_ref=0)=array (0 => (refcount=0, is_ref=0)=4, 'b' => (refcount=0, is_ref=0)=42) }
e->bar: (refcount=2, is_ref=1)=FALSE
e->bar['b']: no such symbol
e->baz[0]: (refcount=0, is_ref=0)=4
e->baz['b']: (refcount=0, is_ref=0)=42
