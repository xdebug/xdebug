--TEST--
Test for indirect function call
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
--FILE--
<?php
	xdebug_start_trace($tf = tempnam('/tmp', 'xdt'));

	function blaat ()
	{
	}

	$func = 'blaat';
	echo $func();

	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d     -> blaat() /%s/test11.php:9
    %f      %d     -> file_get_contents('/tmp/%s') /%s/test11.php:11
