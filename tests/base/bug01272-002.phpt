--TEST--
Test for bug #1272: property_get doesn't return attributes for SimpleXMLElement
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext simplexml');
?>
--FILE--
<?php
include dirname(__FILE__) . '/bug01272.inc';

xdebug_debug_zval('e');
xdebug_debug_zval('e->@attributes');
xdebug_debug_zval('e->@attributes["att1"]');
xdebug_debug_zval('e->b[0]->@attributes');
xdebug_debug_zval('e->b[1]->@attributes["attb"]');
?>
--EXPECTF--
attb-1
attb-2
e: (refcount=1, is_ref=0)=class SimpleXMLElement { public $@attributes = (refcount=1, is_ref=0)=array ('att1' => (refcount=1, is_ref=0)='att-a'); public $b = (refcount=1, is_ref=0)=array (0 => (refcount=1, is_ref=0)=class SimpleXMLElement { ... }, 1 => (refcount=1, is_ref=0)=class SimpleXMLElement { ... }) }
e->@attributes: (refcount=%d, is_ref=0)=array ('att1' => (refcount=1, is_ref=0)='att-a')
e->@attributes["att1"]: (refcount=%d, is_ref=0)='att-a'
e->b[0]->@attributes: (refcount=%d, is_ref=0)=array ('attb' => (refcount=1, is_ref=0)='attb-1')
e->b[1]->@attributes["attb"]: (refcount=%d, is_ref=0)='attb-2'
