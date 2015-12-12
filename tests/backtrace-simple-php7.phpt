--TEST--
Test for simple backtrace (>= PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '>=')) echo "skip >= PHP 7.0 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
xdebug.trace_format=0
xdebug.show_error_trace=0
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
Fatal error: Uncaught Error: Call to undefined function%sc() in %sbacktrace-simple-php7.php on line 7

Error: Call to undefined function c() in %sbacktrace-simple-php7.php on line 7

Call Stack:
%w%f %w%d   1. {main}() %sbacktrace-simple-php7.php:0
%w%f %w%d   2. a() %sbacktrace-simple-php7.php:10
%w%f %w%d   3. b() %sbacktrace-simple-php7.php:3
