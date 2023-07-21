--TEST--
Test for bug #1148: Can't disable max_nesting_function
--INI--
xdebug.max_nesting_level=-1
--FILE--
<?php
echo strlen("42234");
?>
--EXPECT--
5
