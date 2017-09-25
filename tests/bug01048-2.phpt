--TEST--
Test for bug #1048: Can not get $GLOBAL variable by property_value on function context
--FILE--
<?php
include dirname(__FILE__) . '/bug01048.inc';

xdebug_debug_zval('$GLOBALS[\'cache\']');
xdebug_debug_zval('$GLOBALS[\'cache\']');
?>
--EXPECTF--
11$GLOBALS['cache']: (refcount=%d, is_ref=0)='cache'
$GLOBALS['cache']: (refcount=%d, is_ref=0)='cache'
