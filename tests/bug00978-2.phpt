--TEST--
Test for bug #978: Inspection of array with negative keys fails
--FILE--
<?php
include dirname(__FILE__) . '/bug00978.inc';

xdebug_debug_zval('$a');
xdebug_debug_zval('$a[-1]');
xdebug_debug_zval('$a[-]');
xdebug_debug_zval('$a[1]');
?>
--EXPECTF--
4
$a: (refcount=1, is_ref=0)=array (0 => (refcount=1, is_ref=0)='nul', -1 => (refcount=1, is_ref=0)='minus one', -2 => (refcount=1, is_ref=0)='not two', 1 => (refcount=1, is_ref=0)='een')
$a[-1]: (refcount=1, is_ref=0)='minus one'
$a[-]: no such symbol
$a[1]: (refcount=1, is_ref=0)='een'
