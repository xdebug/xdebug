--TEST--
Test for class members
--SKIPIF--
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.auto_profile=0
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));

	class aaa {
		public $c1;
		public $c2;
		function a1 () {
			return 'a1';
		}
		function a2 () {
			return 'a2';
		}
	}

	class bbb {
		function b1 () {
			return 'a1';
		}
		function b2 () {
			return 'a2';
		}
	}


	$a = new aaa;
	$b = new bbb;
	$a->a1();
	$b->b1();
	$a->a2();

	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d     -> aaa->a1() /%s/test7b.php:27
    %f      %d     -> bbb->b1() /%s/test7b.php:28
    %f      %d     -> aaa->a2() /%s/test7b.php:29
    %f      %d     -> file_get_contents('/tmp/%s') /%s/test7b.php:31
