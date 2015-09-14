--TEST--
Test for bug #670: Xdebug crashes with broken "break x" code (>= PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '>=')) echo "skip >= PHP 7.0 needed\n"; ?>
--FILE--
<?php
xdebug_start_code_coverage( XDEBUG_CC_DEAD_CODE | XDEBUG_CC_UNUSED );
include '670-ConsistentHashing.inc';
echo "OK\n";
--EXPECTF--
Fatal error: Cannot 'break' 2 levels in %s670-ConsistentHashing.inc on line 146
