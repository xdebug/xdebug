--TEST--
Test for class members
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.auto_profile=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

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

	xdebug_stop_trace();
	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> aaa->a1() %stest7b.php:27
%w%f %w%d     -> bbb->b1() %stest7b.php:28
%w%f %w%d     -> aaa->a2() %stest7b.php:29
%w%f %w%d     -> xdebug_stop_trace() %stest7b.php:31
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
