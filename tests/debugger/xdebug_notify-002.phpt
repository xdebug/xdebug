--TEST--
xdebug_notify() without a debugging session active
--FILE--
<?php
var_dump( xdebug_notify( "no debug session" ) );
?>
--EXPECTF--
bool(false)
