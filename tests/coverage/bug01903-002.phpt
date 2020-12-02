--TEST--
Test for bug #1903: XDEBUG_CC_DEAD_CODE and XDEBUG_CC_UNUSED should not be defined with mode=off
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
bool(false)
bool(false)
bool(false)
