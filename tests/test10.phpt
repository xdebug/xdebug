--TEST--
Test for nested indirect function call
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(zend_version(), "2.0.0-dev", '>=')) echo "skip Zend Engine 1 needed\n"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));
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
    %f          %d     -> d::c('blah') /%s/test10.php:19
    %f          %d     -> d::a('c') /%s/test10.php:19
    %f          %d     -> d::b('a') /%s/test10.php:19
    %f          %d     -> d::a('b') /%s/test10.php:19
    %f          %d     -> blaat('insert blah \'a') /%s/test10.php:19
    %f          %d     -> file_get_contents('/tmp/%s') /%s/test10.php:20
