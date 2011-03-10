--TEST--
Test for bug #670: Xdebug crashes with broken "break x" code.
--FILE--
<?php
xdebug_start_code_coverage( XDEBUG_CC_DEAD_CODE | XDEBUG_CC_UNUSED );
include '670-ConsistentHashing.php';
echo "OK\n";
--EXPECT--
OK
