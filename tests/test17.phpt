--TEST--
Test for internal parameters
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
--FILE--
<?php
	$tf = xdebug_start_trace(tempnam('/tmp', 'xdt'));

	echo str_repeat ("5", 5);

	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
55555
TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d     -> str_repeat('5', 5) /%s/test17.php:4
    %f      %d     -> file_get_contents('/tmp/%s') /%s/test17.php:6
