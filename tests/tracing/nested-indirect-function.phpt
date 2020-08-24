--TEST--
Test for nested indirect function call
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));
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

	$a = blaat("insert blah '".D::a(D::b(D::a(D::c('blah')))));
	xdebug_stop_trace();
	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> D::c($x = 'blah') %snested-indirect-function.php:19
%w%f %w%d      >=> 'c'
%w%f %w%d     -> D::a($x = 'c') %snested-indirect-function.php:19
%w%f %w%d      >=> 'a'
%w%f %w%d     -> D::b($x = 'a') %snested-indirect-function.php:19
%w%f %w%d      >=> 'b'
%w%f %w%d     -> D::a($x = 'b') %snested-indirect-function.php:19
%w%f %w%d      >=> 'a'
%w%f %w%d     -> blaat($a = 'insert blah \'a') %snested-indirect-function.php:19
%w%f %w%d      >=> NULL
%w%f %w%d     -> xdebug_stop_trace() %snested-indirect-function.php:20
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
