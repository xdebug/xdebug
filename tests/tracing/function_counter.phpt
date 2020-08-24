--TEST--
Test for xdebug_get_function_count
--INI--
xdebug.mode=develop
xdebug.auto_profile=0
--FILE--
<?php
	echo xdebug_get_function_count(). "\n";

	for ($i = 0; $i < 9; $i++) {
		strrev($i);
	}

	echo xdebug_get_function_count(). "\n";
?>
--EXPECT--
1
11
