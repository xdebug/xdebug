<?php
	echo xdebug_is_enabled() ? "Enabled\n" : "Disabled\n";

	xdebug_disable();

	echo xdebug_is_enabled() ? "Enabled\n" : "Disabled\n";

	xdebug_enable();

	echo xdebug_is_enabled() ? "Enabled\n" : "Disabled\n";
?>
