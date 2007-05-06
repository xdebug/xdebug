--TEST--
Test for nested indirect function call
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));
	class D
	{
		static function a($x) {
			return 'a';
		}
		static function b($x) {
			return 'b';
		}
		static function c($x) {
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
%w%f %w%d     -> D::c('blah') /%s/test10b.php:19
                           >=> 'c'
%w%f %w%d     -> D::a('c') /%s/test10b.php:19
                           >=> 'a'
%w%f %w%d     -> D::b('a') /%s/test10b.php:19
                           >=> 'b'
%w%f %w%d     -> D::a('b') /%s/test10b.php:19
                           >=> 'a'
%w%f %w%d     -> blaat('insert blah \'a') /%s/test10b.php:19
                           >=> NULL
%w%f %w%d     -> file_get_contents('/tmp/%s') /%s/test10b.php:20
