--TEST--
Test for changed setting with E_DEPRECATED
--INI--
xdebug.remote_mode=req
error_reporting=E_DEPRECATED
--FILE--
<?php
echo "Hello!\n";
?>
--EXPECTF--
Xdebug: [Config] The setting 'xdebug.remote_mode' has been renamed, see the upgrading guide at %s/upgrade_guide#changed-xdebug.remote_mode (See: %s/errors#CFG-C-CHANGED)
Hello!
