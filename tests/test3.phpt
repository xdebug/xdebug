--TEST--
Test for xdebug_is_enabled, xdebug_disable and xdebug_enable
--INI--
xdebug.enable=1
--FILE--
<?php
	echo xdebug_is_enabled() ? "Enabled\n" : "Disabled\n";

	xdebug_disable();

	echo xdebug_is_enabled() ? "Enabled\n" : "Disabled\n";

	xdebug_enable();

	echo xdebug_is_enabled() ? "Enabled\n" : "Disabled\n";
?>
--EXPECT--
Enabled
Disabled
Enabled
