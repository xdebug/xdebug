--TEST--
Test for changed/removed setting without E_DEPRECATED
--INI--
xdebug.remote_mode=req
xdebug.remote_handler=dbgp
error_reporting=E_ALL & ~E_DEPRECATED
--FILE--
<?php
echo "Hello!\n";
?>
--EXPECT--
Hello!
