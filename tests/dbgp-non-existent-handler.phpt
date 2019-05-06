--TEST--
DBGp: Using an unknown remote handler
--INI--
xdebug.remote_handler=foobar
xdebug.remote_enable=1
xdebug.remote_autostart=1
--FILE--
<?php
echo "Alive!\n";
?>
--EXPECTF--
%s: The remote debug handler 'foobar' is not supported in %sdbgp-non-existent-handler.php on line %d
Alive!
