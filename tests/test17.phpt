--TEST--
Test for internal parameters
--INI--
xdebug.enable=1
xdebug.auto_trace=0
--FILE--
<?php
	xdebug_start_trace();

	echo str_repeat ("5", 5);

	xdebug_dump_function_trace();
?>
--EXPECTF--
55555
Function trace:
    %f      %d     -> str_repeat('5', 5) /%s/test17.php:4
