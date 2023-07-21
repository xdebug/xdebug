--TEST--
Test for xdebug_debug_zval() (CLI colours) (NTS, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('NTS; !opcache');
?>
--INI--
xdebug.mode=develop
xdebug.cli_color=2
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
a: (%s, is_ref=0)=[1mstring[22m([32m4[0m) "[31mhoge[0m"

$a: (%s, is_ref=0)=[1mstring[22m([32m4[0m) "[31mhoge[0m"

$b: (refcount=1, is_ref=0)=[1marray[22m([32m5[0m) {
  'a' =>
  (refcount=0, is_ref=0)=[1mint[22m([32m4[0m)
  'b' =>
  (refcount=2, is_ref=1)=[1mint[22m([32m5[0m)
  'c' =>
  (refcount=0, is_ref=0)=[1mint[22m([32m6[0m)
  [0] [0m=>[0m
  (refcount=0, is_ref=0)=[1mint[22m([32m8[0m)
  [1] [0m=>[0m
  (refcount=0, is_ref=0)=[1mint[22m([32m9[0m)
}

$b['a']: (refcount=0, is_ref=0)=[1mint[22m([32m4[0m)

$b['b']: (refcount=2, is_ref=1)=[1mint[22m([32m5[0m)

b[1]: (refcount=0, is_ref=0)=[1mint[22m([32m9[0m)

c: (refcount=2, is_ref=1)=[1mint[22m([32m5[0m)

d: (refcount=0, is_ref=0)=[1mint[22m([32m6[0m)

e: (refcount=1, is_ref=0)=[1mclass[22m [31mstdClass[0m#%d ([32m3[0m) {
  [32m[1mpublic[22m[0m $foo [0m=>[0m
  (refcount=2, is_ref=1)=[1mbool[22m([35mfalse[0m)
  [32m[1mpublic[22m[0m $bar [0m=>[0m
  (refcount=2, is_ref=1)=[1mbool[22m([35mfalse[0m)
  [32m[1mpublic[22m[0m $baz [0m=>[0m
  (%s, is_ref=0)=[1marray[22m([32m2[0m) {
    [0] [0m=>[0m
    (refcount=0, is_ref=0)=[1mint[22m([32m4[0m)
    'b' =>
    (refcount=0, is_ref=0)=[1mint[22m([32m42[0m)
  }
}

e->bar: (refcount=2, is_ref=1)=[1mbool[22m([35mfalse[0m)

e->bar['b']: no such symbol
e->baz[0]: (refcount=0, is_ref=0)=[1mint[22m([32m4[0m)

e->baz['b']: (refcount=0, is_ref=0)=[1mint[22m([32m42[0m)
