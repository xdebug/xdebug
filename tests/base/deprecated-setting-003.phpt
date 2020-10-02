--TEST--
Test for removed setting with E_DEPRECATED
--INI--
xdebug.remote_handler=dbgp
error_reporting=E_DEPRECATED
--FILE--
<?php
echo "Hello!\n";
?>
--EXPECTF--
Xdebug: [Config] The setting 'xdebug.remote_handler' has been removed, see the upgrading guide at %s/upgrade_guide#changed-xdebug.remote_handler (See: %s/errors#CFG-C-REMOVED)
Hello!
