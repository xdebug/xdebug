--TEST--
Test for nested indirect function call
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
--FILE--
<?php
	xdebug_start_trace($tf = tempnam('/tmp', 'xdt'));
	class D
	{
		function a($x) {
			return 'a';
		}
		function b($x) {
			return 'b';
		}
		function c($x) {
			return 'c';
		}
	}

	function blaat($a) {
	}

	blaat("insert blah '".D::a(D::b(D::a(D::c('blah')))));
	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d     -> d::c('blah') /%s/test10.php:19
    %f      %d     -> d::a('c') /%s/test10.php:19
    %f      %d     -> d::b('a') /%s/test10.php:19
    %f      %d     -> d::a('b') /%s/test10.php:19
    %f      %d     -> blaat('insert blah \'a') /%s/test10.php:19
    %f      %d     -> file_get_contents('/tmp/%s') /%s/test10.php:20
