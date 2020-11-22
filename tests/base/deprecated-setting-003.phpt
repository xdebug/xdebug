--TEST--
Test for removed setting with E_DEPRECATED
--INI--
xdebug.show_mem_delta=1
error_reporting=E_DEPRECATED
--FILE--
<?php
echo "Hello!\n";
?>
--EXPECTF--
Xdebug: [Config] The setting 'xdebug.show_mem_delta' has been removed, see the upgrading guide at %s/upgrade_guide#changed-xdebug.show_mem_delta (See: %s/errors#CFG-C-REMOVED)
Hello!
