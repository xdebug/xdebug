--TEST--
Test for bug #2430: Variable fetching may crash with :: if there is no active stack
--INI--
xdebug.mode=develop
--FILE--
<?php
xdebug_debug_zval(":::");
?>
--EXPECT--
:::: no such symbol
