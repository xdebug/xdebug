--TEST--
Test for bug #1048: Can not get $GLOBAL variable by property_value on function context
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8.1');
?>
--FILE--
<?php
include dirname(__FILE__) . '/bug01048.inc';

xdebug_debug_zval('$GLOBALS[\'cache\']');
xdebug_debug_zval('$GLOBALS[\'cache\']');
?>
--EXPECTF--
11$GLOBALS['cache']: (%s, is_ref=0)='cache'
$GLOBALS['cache']: (%s, is_ref=0)='cache'
