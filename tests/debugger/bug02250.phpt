--TEST--
Test for bug #2250: Multiple 'DEBUG SESSION ENDED' messages
--INI--
xdebug.mode=debug
--GET--
XDEBUG_SESSION_STOP_NO_EXEC=netbeans-xdebug
--FILE--
<?php
xdebug_info();
?>
--EXPECT--
DEBUG SESSION ENDED
