--TEST--
Test for xdebug_get_function_count
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.enable=1
xdebug.collect_params=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
--FILE--
<?php
	echo xdebug_get_function_count(). "\n";

	for ($i = 0; $i < 9; $i++) {
		strlen($i);
	}

	echo xdebug_get_function_count(). "\n";
?>
--EXPECT--
1
11
