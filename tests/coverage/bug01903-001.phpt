--TEST--
Test for bug #1903: XDEBUG_CC_DEAD_CODE and XDEBUG_CC_UNUSED are only defined with mode=coverage
--INI--
xdebug.mode=develop
--FILE--
<?php
echo XDEBUG_CC_DEAD_CODE, ' ', XDEBUG_CC_UNUSED, ' ', XDEBUG_CC_BRANCH_CHECK, "\n";
?>
--EXPECT--
2 1 4
