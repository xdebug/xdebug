--TEST--
Test for simple backtrace
--INI--
xdebug.enable=1
--FILE--
<?php
	function a () {
		b();
	}

	function b () {
		c();
	}

	a();
?>
--EXPECTF--
Fatal error: Call to undefined function:  c() in /dat/dev/php/xdebug/tests/phpt.%s on line 7

Call Stack:
    %f      %d   1. {main}() /%s/phpt.%s:0
    %f      %d   2. a() /%s/phpt.%s:10
    %f      %d   3. b() /%s/phpt.%s:3
