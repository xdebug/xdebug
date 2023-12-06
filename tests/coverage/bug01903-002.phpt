--TEST--
Test for bug #1903: XDEBUG_CC_DEAD_CODE and XDEBUG_CC_UNUSED should be defined with mode=off
--INI--
xdebug.mode=off
--FILE--
<?php
var_dump(
	defined( 'XDEBUG_CC_DEAD_CODE' ),
	defined( 'XDEBUG_CC_UNUSED' ),
	defined( 'XDEBUG_CC_BRANCH_CHECK' )
);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
