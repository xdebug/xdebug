--TEST--
xdebug_notify() without a debugging session active
--INI--
xdebug.mode=debug
--FILE--
<?php
var_dump( xdebug_notify( "no debug session" ) );
?>
--EXPECTF--
bool(false)
