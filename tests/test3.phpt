--TEST--
Test for xdebug_is_enabled, xdebug_disable and xdebug_enable
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
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
